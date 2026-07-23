#include <algorithm>
#include <tuple>
#include <godot_cpp/classes/camera2d.hpp>
#include <godot_cpp/classes/collision_polygon2d.hpp>
#include <godot_cpp/classes/directional_light2d.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/marker2d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/rigid_body2d.hpp>
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "core/constants.hpp"
#include "entity/character/character.hpp"
#include "entity/character/enemy.hpp"
#include "entity/character/enemy_brute.hpp"
#include "entity/controller/enemy_controller.hpp"
#include "entity/controller/player_controller.hpp"
#include "entity/level.hpp"
#include "entity/projectile/projectile.hpp"
#include "singletons/console.hpp"
#include "util/birth_buff.hpp"
#include "util/bind.hpp"
#include "util/conversions.hpp"
#include "util/debug.hpp"
#include "util/engine.hpp"
#include "util/input.hpp"
#include "util/io.hpp"

namespace rl
{
    namespace
    {
        constexpr const char* state_label(const LevelState state)
        {
            switch (state)
            {
                case LevelState::Playing:
                    return "Playing";
                case LevelState::Victory:
                    return "Victory";
                case LevelState::Defeat:
                    return "Defeat";
                default:
                    return "Unknown";
            }
        }
    }

    Level::Level()
    {
        scene::node::set_unique_name(this, name::level::level1);
        this->activate(true);
    }

    void Level::set_room_index(const int room_index)
    {
        m_room_index = room_index;
    }

    int Level::get_room_index() const
    {
        return m_room_index;
    }

    void Level::set_room_kind(const RoomKind kind)
    {
        m_room_kind = kind;
    }

    void Level::set_is_final_room(const bool is_final)
    {
        m_is_final_room = is_final;
    }

    void Level::request_room_clear()
    {
        if (m_state != LevelState::Playing)
            return;

        if (m_room_kind != RoomKind::Whisper && m_room_kind != RoomKind::Mood)
            return;

        this->complete_room();
    }

    void Level::complete_room()
    {
        if (m_state != LevelState::Playing)
            return;

        this->set_player_input_enabled(false);

        if (m_is_final_room)
        {
            this->transition_to_state(LevelState::Victory);
            return;
        }

        this->emit_signal(event::room_cleared, m_room_index);
    }

    int Level::get_player_hearts() const
    {
        if (m_player == nullptr)
            return 0;
        return m_player->get_hearts();
    }

    void Level::apply_player_hearts(const int hearts)
    {
        if (m_player == nullptr)
            return;
        m_player->apply_hearts(hearts);
    }

    void Level::_ready()
    {
        godot::Node* box{ this->find_child(name::level::physics_box, true, false) };
        m_physics_box = try_gdcast<godot::RigidBody2D>(box);

        resource::preload::packed_scene<Player> player_scene{ path::scene::Player };
        m_player = player_scene.instantiate();
        m_player->set_controller(memnew(PlayerController));

        this->add_child(m_player);
        this->spawn_player_at_marker();
        this->add_child(m_projectile_spawner);
        birth_buff::apply_from_disk(m_player, m_projectile_spawner);
        if (m_room_kind == RoomKind::Combat || m_room_kind == RoomKind::Boss)
            this->spawn_enemies_from_markers();
        this->apply_room_camera_limits();
        this->apply_forest_atmosphere();

        signal<event::died>::connect<Player>(m_player) <=> signal_callback(this, on_player_died);

        PlayerController* controller{ gdcast<PlayerController>(m_player->get_controller()) };
        if (controller != nullptr)
        {
            signal<event::position_changed>::connect<CharacterController>(controller) <=>
                signal_callback(this, on_character_position_changed);

            signal<event::spawn_projectile>::connect<Player>(m_player) <=>
                signal_callback(this, on_player_spawn_projectile);
        }
    }

    LevelState Level::get_state() const
    {
        return m_state;
    }

    void Level::spawn_player_at_marker()
    {
        if (m_player == nullptr)
            return;

        godot::Node* spawn_node{ this->find_child(name::level::spawn_point, true, false) };
        godot::Marker2D* spawn_point{ try_gdcast<godot::Marker2D>(spawn_node) };
        if (spawn_point != nullptr)
            m_player->set_global_position(spawn_point->get_global_position());
    }

    void Level::spawn_enemies_from_markers()
    {
        m_pending_spawns.clear();

        const int child_count{ this->get_child_count() };
        for (int i = 0; i < child_count; ++i)
        {
            godot::Node* child{ this->get_child(i) };
            if (child == nullptr)
                continue;

            const godot::String child_name{ child->get_name() };
            const bool is_brute{ child_name.begins_with(name::level::enemy_brute_spawn_prefix) };
            const bool is_scout{ child_name.begins_with(name::level::enemy_spawn_prefix) };
            if (!is_brute && !is_scout)
                continue;

            godot::Marker2D* marker{ try_gdcast<godot::Marker2D>(child) };
            if (marker == nullptr)
                continue;

            this->queue_enemy_spawn(marker->get_position(), is_brute);
        }
    }

    void Level::queue_enemy_spawn(const godot::Vector2 position, const bool is_brute)
    {
        PendingEnemySpawn entry{};
        entry.position = position;
        entry.is_brute = is_brute;
        entry.delay_remaining = spawn::telegraph_duration;
        m_pending_spawns.push_back(entry);
        this->queue_redraw();
    }

    void Level::finalize_enemy_spawn(const PendingEnemySpawn& pending)
    {
        resource::preload::packed_scene<Enemy> enemy_scene{ path::scene::Enemy };
        resource::preload::packed_scene<EnemyBrute> brute_scene{ path::scene::EnemyBrute };

        Enemy* enemy{ pending.is_brute ? static_cast<Enemy*>(brute_scene.instantiate())
                                     : enemy_scene.instantiate() };
        if (enemy == nullptr)
            return;

        auto* controller{ memnew(EnemyController) };
        controller->set_behavior(pending.is_brute ? EnemyController::Behavior::BruteCharge
                                                  : EnemyController::Behavior::ScoutRanged);
        enemy->set_controller(controller);
        enemy->set_position(pending.position);
        enemy->set_modulate(godot::Color(1.0f, 1.0f, 1.0f, 0.0f));
        this->add_child(enemy);

        signal<event::died>::connect<Enemy>(enemy) <=> signal_callback(this, on_enemy_died);
        signal<event::spawn_projectile>::connect<Enemy>(enemy) <=>
            signal_callback(this, on_enemy_spawn_projectile);

        m_fading_enemies.push_back(FadingEnemy{ enemy, spawn::fade_in_duration });
        ++m_enemy_count;
    }

    void Level::update_pending_spawns(const double delta_time)
    {
        if (m_pending_spawns.empty())
            return;

        std::vector<PendingEnemySpawn> ready{};
        ready.reserve(m_pending_spawns.size());

        for (auto& pending : m_pending_spawns)
        {
            pending.delay_remaining -= delta_time;
            if (pending.delay_remaining <= 0.0)
                ready.push_back(pending);
        }

        if (ready.empty())
            return;

        m_pending_spawns.erase(
            std::remove_if(m_pending_spawns.begin(), m_pending_spawns.end(),
                           [](const PendingEnemySpawn& p) { return p.delay_remaining <= 0.0; }),
            m_pending_spawns.end());

        for (const PendingEnemySpawn& pending : ready)
            this->finalize_enemy_spawn(pending);
    }

    void Level::update_fading_enemies(const double delta_time)
    {
        if (m_fading_enemies.empty())
            return;

        for (auto it = m_fading_enemies.begin(); it != m_fading_enemies.end();)
        {
            FadingEnemy& entry{ *it };
            if (entry.enemy == nullptr || !entry.enemy->is_inside_tree())
            {
                it = m_fading_enemies.erase(it);
                continue;
            }

            entry.fade_remaining -= delta_time;
            const float t{ godot::Math::clamp(
                1.0f - static_cast<float>(entry.fade_remaining / spawn::fade_in_duration), 0.0f,
                1.0f) };
            entry.enemy->set_modulate(godot::Color(1.0f, 1.0f, 1.0f, t));

            if (entry.fade_remaining <= 0.0)
            {
                entry.enemy->set_modulate(godot::Color(1.0f, 1.0f, 1.0f, 1.0f));
                it = m_fading_enemies.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void Level::apply_room_camera_limits()
    {
        if (m_player == nullptr)
            return;

        godot::Node* camera_node{
            m_player->find_child(name::level::player_camera, true, false) };
        godot::Camera2D* camera{ try_gdcast<godot::Camera2D>(camera_node) };
        if (camera == nullptr)
            return;

        camera->set("limit_smoothed", true);
        camera->set("limit_left", static_cast<int>(-level::half_playable_width));
        camera->set("limit_right", static_cast<int>(level::half_playable_width));
        camera->set("limit_top", static_cast<int>(-level::half_playable_height));
        camera->set("limit_bottom", static_cast<int>(level::half_playable_height));
    }

    void Level::apply_forest_atmosphere()
    {
        // 1) 背景向暖绿靠拢（保留各房原 modulate 差异）
        if (godot::Node* bg_node{ this->find_child("BackgroundTexture", true, false) })
        {
            if (godot::Sprite2D* bg{ godot::Object::cast_to<godot::Sprite2D>(bg_node) })
            {
                const godot::Color forest{ 0.72f, 0.88f, 0.70f, 1.0f };
                bg->set_modulate(
                    bg->get_modulate().lerp(forest, atmosphere::background_forest_blend));
            }
        }

        // 2) 定向光偏林间暖绿
        if (godot::Node* light_node{ this->find_child("DirectionalLight2D", true, false) })
        {
            if (godot::DirectionalLight2D* light{
                    godot::Object::cast_to<godot::DirectionalLight2D>(light_node) })
            {
                light->set_color(godot::Color(0.90f, 0.96f, 0.82f, 1.0f));
                light->set_energy(atmosphere::light_energy);
            }
        }

        // 已有场景雾节点则不重复挂
        if (this->find_child(name::level::fog_overlay, true, false) != nullptr)
            return;

        godot::ResourceLoader* loader{ godot::ResourceLoader::get_singleton() };
        if (loader == nullptr || !loader->exists(path::shader::forest_fog))
        {
            console::get()->print("{} {}", io::orange("atmosphere"),
                                  io::yellow("forest_fog.gdshader missing"));
            return;
        }

        godot::Ref<godot::Shader> shader{ loader->load(path::shader::forest_fog) };
        if (!shader.is_valid())
            return;

        godot::Ref<godot::Image> image{
            godot::Image::create_empty(64, 64, false, godot::Image::FORMAT_RGBA8) };
        if (!image.is_valid())
            return;
        image->fill(godot::Color(1.0f, 1.0f, 1.0f, 1.0f));

        godot::Ref<godot::ImageTexture> texture{ godot::ImageTexture::create_from_image(image) };
        if (!texture.is_valid())
            return;

        godot::Ref<godot::ShaderMaterial> material;
        material.instantiate();
        material->set_shader(shader);
        material->set_shader_parameter("density", atmosphere::fog_density);
        material->set_shader_parameter("drift_speed", atmosphere::fog_drift_speed);
        material->set_shader_parameter("scale", atmosphere::fog_scale);
        material->set_shader_parameter("fog_color",
                                      godot::Color(0.58f, 0.76f, 0.58f, 1.0f));

        godot::Sprite2D* fog{ memnew(godot::Sprite2D) };
        fog->set_name(name::level::fog_overlay);
        fog->set_texture(texture);
        fog->set_centered(true);
        fog->set_z_index(atmosphere::fog_z_index);
        fog->set_scale(godot::Vector2(level::playable_width / 64.0f,
                                      level::playable_height / 64.0f));
        fog->set_material(material);
        this->add_child(fog);

        console::get()->print("{} {}", io::green("atmosphere"), io::blue("forest fog on"));
    }

    void Level::clear_enemies()
    {
        m_pending_spawns.clear();
        m_fading_enemies.clear();

        for (int i = this->get_child_count() - 1; i >= 0; --i)
        {
            godot::Node* child{ this->get_child(i) };
            if (child != nullptr && godot::Object::cast_to<Enemy>(child) != nullptr)
                child->queue_free();
        }
    }

    void Level::clear_projectiles()
    {
        for (int i = this->get_child_count() - 1; i >= 0; --i)
        {
            godot::Node* child{ this->get_child(i) };
            if (child != nullptr && godot::Object::cast_to<Projectile>(child) != nullptr)
                child->queue_free();
        }
    }

    void Level::set_player_input_enabled(const bool enabled)
    {
        if (m_player == nullptr)
            return;

        PlayerController* controller{ gdcast<PlayerController>(m_player->get_controller()) };
        if (controller != nullptr)
            controller->set_input_enabled(enabled);
    }

    void Level::transition_to_state(const LevelState new_state)
    {
        if (m_state != LevelState::Playing)
            return;

        if (new_state == LevelState::Playing)
            return;

        m_state = new_state;
        this->set_player_input_enabled(false);

        this->emit_signal(event::level_state_changed, static_cast<int>(new_state));
    }

    void Level::reset_level()
    {
        this->clear_projectiles();
        this->clear_enemies();

        m_state = LevelState::Playing;
        m_enemy_count = 0;

        if (m_player != nullptr)
        {
            m_player->reset_hearts();
            this->spawn_player_at_marker();
        }

        this->set_player_input_enabled(true);
        if (m_room_kind == RoomKind::Combat || m_room_kind == RoomKind::Boss)
            this->spawn_enemies_from_markers();

        this->emit_signal(event::level_state_changed, static_cast<int>(m_state));

        auto console{ console::get() };
        console->print("{} {}", io::green("level reset"), io::blue(state_label(m_state)));
    }

    void Level::handle_restart_input()
    {
        if (m_state != LevelState::Defeat && m_state != LevelState::Victory)
            return;

        godot::Input* input_handler{ input::get() };
        if (input_handler == nullptr)
            return;

        if (!input_handler->is_action_just_pressed(input::action::restart))
            return;

        if (m_state == LevelState::Victory)
        {
            // Day12：结语层处理回标题；Victory 下忽略 R，避免与结语抢输入。
            return;
        }

        this->reset_level();
    }

    void Level::_process(double delta_time)
    {
        if (engine::editor_active())
            return;

        if (this->active() && input::cursor_visible()) [[unlikely]]
            input::hide_cursor();
        else if (!this->active() && !input::cursor_visible()) [[unlikely]]
            input::show_cursor();

        if (m_state == LevelState::Defeat || m_state == LevelState::Victory)
            this->handle_restart_input();

        if (m_state == LevelState::Playing)
        {
            this->update_pending_spawns(delta_time);
            this->update_fading_enemies(delta_time);
        }

        this->queue_redraw();
    }

    void Level::_draw()
    {
        if (this->active() && m_state == LevelState::Playing) [[likely]]
        {
            for (const PendingEnemySpawn& pending : m_pending_spawns)
            {
                const float progress{ godot::Math::clamp(
                    1.0f - static_cast<float>(pending.delay_remaining / spawn::telegraph_duration),
                    0.0f, 1.0f) };
                const float alpha{ 0.25f + 0.55f * progress };
                const godot::Color ring{ 0.95f, 0.78f, 0.35f, alpha };
                const godot::Color fill{ 0.95f, 0.78f, 0.35f, alpha * 0.25f };
                this->draw_circle(pending.position, spawn::telegraph_radius, fill);
                this->draw_arc(pending.position, spawn::telegraph_radius, 0.0f, 6.2831855f, 32,
                               ring, 2.5f, true);
            }

            // 攻击准星：暖金光点（环 + 亮心 + 四道光刺），呼应「光」主题。
            const godot::Vector2 aim{ this->get_global_mouse_position() };
            const godot::Color halo{ 0.98f, 0.90f, 0.60f, 0.85f };
            const godot::Color core{ 1.0f, 0.98f, 0.85f, 1.0f };
            this->draw_arc(aim, 10.0f, 0.0f, 6.2831855f, 28, halo, 1.8f, true);
            this->draw_circle(aim, 2.4f, core);
            constexpr float spike_in{ 5.0f };
            constexpr float spike_out{ 9.0f };
            this->draw_line(aim + godot::Vector2{ spike_in, 0.0f },
                            aim + godot::Vector2{ spike_out, 0.0f }, halo, 1.6f, true);
            this->draw_line(aim + godot::Vector2{ -spike_in, 0.0f },
                            aim + godot::Vector2{ -spike_out, 0.0f }, halo, 1.6f, true);
            this->draw_line(aim + godot::Vector2{ 0.0f, spike_in },
                            aim + godot::Vector2{ 0.0f, spike_out }, halo, 1.6f, true);
            this->draw_line(aim + godot::Vector2{ 0.0f, -spike_in },
                            aim + godot::Vector2{ 0.0f, -spike_out }, halo, 1.6f, true);
        }
    }

    void Level::activate(bool active)
    {
        m_active = active;

        // 鼠标离开游戏画面 → 暂停玩家输入（朝向 / 移动 / 射击），
        // 避免鼠标在窗口外时小人仍跟随移动或转向。仅在游玩中联动。
        if (m_state == LevelState::Playing)
            this->set_player_input_enabled(active);
    }

    bool Level::active() const
    {
        return m_active;
    }

    void Level::_bind_methods()
    {
        bind_member_function(Level, on_character_position_changed);
        bind_member_function(Level, on_player_spawn_projectile);
        bind_member_function(Level, on_enemy_spawn_projectile);
        bind_member_function(Level, on_player_died);
        bind_member_function(Level, on_enemy_died);
        bind_member_function(Level, request_room_clear);
        signal_binding<Level, event::level_state_changed>::add<int>();
        signal_binding<Level, event::room_cleared>::add<int>();
        signal_binding<Level, event::run_restart>::add<>();
    }

    [[signal_slot]]
    void Level::on_player_spawn_projectile(godot::Node* obj)
    {
        this->spawn_projectile_from_marker(obj, false);
    }

    [[signal_slot]]
    void Level::on_enemy_spawn_projectile(godot::Node* obj)
    {
        this->spawn_projectile_from_marker(obj, true);
    }

    void Level::spawn_projectile_from_marker(godot::Node* obj, const bool from_enemy)
    {
        if (m_state != LevelState::Playing)
            return;

        Projectile* projectile{ m_projectile_spawner->spawn_projectile() };
        if (projectile == nullptr)
            return;

        if (from_enemy)
            projectile->configure_as_enemy_shot();

        godot::Marker2D* firing_pt{ try_gdcast<godot::Marker2D>(obj) };
        if (firing_pt != nullptr)
        {
            const double spread{ from_enemy ? combat::projectile_spread_radians * 2.0
                                            : combat::projectile_spread_radians };
            const double yaw_jitter{
                spread > 0.0 ? godot::UtilityFunctions::randf_range(-spread, spread) : 0.0
            };
            projectile->set_position(firing_pt->get_global_position());
            projectile->set_rotation(firing_pt->get_global_rotation() + yaw_jitter);
        }
        this->add_child(projectile);
    }

    [[signal_slot]]
    void Level::on_character_position_changed(const godot::Object* const node,
                                              godot::Vector2 location) const
    {
        if (m_state != LevelState::Playing)
            return;

        runtime_assert(node != nullptr);
        auto console{ console::get() };
        console->print("{} ({},{})", io::green(to<std::string>(node->get_class()) + " location: "),
                       io::orange(location.x), io::orange(location.y));
    }

    [[signal_slot]]
    void Level::on_player_died()
    {
        this->transition_to_state(LevelState::Defeat);
    }

    [[signal_slot]]
    void Level::on_enemy_died()
    {
        if (m_state != LevelState::Playing)
            return;

        if (m_enemy_count > 0)
            --m_enemy_count;

        if (m_enemy_count > 0)
            return;

        this->complete_room();
    }
}
