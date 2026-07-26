#include <algorithm>

#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/classes/collision_polygon2d.hpp>
#include <godot_cpp/classes/kinematic_collision2d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/polygon2d.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "core/constants.hpp"
#include "entity/camera.hpp"
#include "entity/character/character.hpp"
#include "entity/character/enemy.hpp"
#include "entity/character/player.hpp"
#include "entity/controller/character_controller.hpp"
#include "singletons/console.hpp"
#include "util/bind.hpp"
#include "util/combat_feedback.hpp"
#include "util/conversions.hpp"
#include "util/io.hpp"
#include "util/scene.hpp"

namespace rl
{
    namespace
    {
        godot::PackedVector2Array make_slash_fan_points()
        {
            const float range{ combat::polar_slash_range };
            const float half_w{ combat::polar_slash_half_width };
            godot::PackedVector2Array points;
            points.push_back({ 0.0f, -8.0f });
            points.push_back({ -half_w, -range * 0.35f });
            points.push_back({ -half_w * 0.55f, -range });
            points.push_back({ half_w * 0.55f, -range });
            points.push_back({ half_w, -range * 0.35f });
            return points;
        }

        godot::PackedVector2Array make_aura_circle_points(const float radius)
        {
            godot::PackedVector2Array points;
            constexpr int segments{ 20 };
            for (int i = 0; i < segments; ++i)
            {
                const float angle{ static_cast<float>(i) * (6.2831853f / segments) };
                points.push_back({ godot::Math::cos(angle) * radius,
                                   godot::Math::sin(angle) * radius });
            }
            return points;
        }
    }

    Player::Player()
        : Character()
    {
        scene::node::set_unique_name(this, name::character::player);
        m_health.set_max(5);
        this->set_collision_mask(collision::player_mask);
    }

    void Player::_ready()
    {
        Character::_ready();
        this->set_collision_mask(collision::player_mask);
        emit_hearts_changed();

        if (m_camera != nullptr)
            m_camera->set_lookahead_enabled(true);

        if (m_character_controller != nullptr)
        {
            signal<event::character_dash>::connect<CharacterController>(m_character_controller) <=>
                signal_callback(this, on_character_dash);
            signal<event::character_polar_slash>::connect<CharacterController>(
                m_character_controller) <=> signal_callback(this, on_character_polar_slash);
            signal<event::character_energy_light>::connect<CharacterController>(
                m_character_controller) <=> signal_callback(this, on_character_energy_light);
        }

        this->ensure_energy_light_aura();

        // Keep ticking for dash / skill cooldown even after hit-flash settles.
        this->set_process(true);
    }

    void Player::_process(const double delta_time)
    {
        this->update_dash(delta_time);
        this->update_skills(delta_time);
        Character::_process(delta_time);
        this->set_process(true);
    }

    godot::Color Player::hit_flash_color() const
    {
        return godot::Color(combat::player_hit_flash_r, combat::player_hit_flash_g,
                            combat::player_hit_flash_b, 1.0f);
    }

    bool Player::is_dashing() const
    {
        return m_dash_remaining > 0.0;
    }

    double Player::get_dash_ready_ratio() const
    {
        if (m_dash_cooldown_remaining <= 0.0)
            return 1.0;
        return std::clamp(1.0 - (m_dash_cooldown_remaining / combat::dash_cooldown), 0.0, 1.0);
    }

    double Player::get_polar_slash_ready_ratio() const
    {
        if (m_polar_slash_cooldown_remaining <= 0.0)
            return 1.0;
        return std::clamp(
            1.0 - (m_polar_slash_cooldown_remaining / combat::polar_slash_cooldown), 0.0, 1.0);
    }

    double Player::get_energy_light_ready_ratio() const
    {
        if (m_energy_light_cooldown_remaining <= 0.0)
            return 1.0;
        return std::clamp(
            1.0 - (m_energy_light_cooldown_remaining / combat::energy_light_cooldown), 0.0, 1.0);
    }

    void Player::collect_heal_pickup(const int hearts)
    {
        if (!is_alive() || hearts <= 0)
            return;

        const int healed{ std::min(this->get_hearts() + hearts, this->get_max_hearts()) };
        this->apply_hearts(healed);
    }

    void Player::collect_haste_pickup(const double duration, const double speed_mult)
    {
        if (!is_alive() || duration <= 0.0 || speed_mult <= 1.0)
            return;

        if (!m_energy_light_haste_active)
        {
            m_movement_speed_before_haste = this->get_movement_speed();
            this->set_movement_speed(m_movement_speed_before_haste * speed_mult);
            m_energy_light_haste_active = true;
        }
        m_energy_light_haste_remaining = std::max(m_energy_light_haste_remaining, duration);
        this->ensure_energy_light_aura();
        this->update_energy_light_aura_visual();
        this->set_process(true);
    }

    void Player::on_character_movement(godot::Vector2 movement_velocity, double delta_time)
    {
        if (movement_velocity.length_squared() > 0.01f)
            m_last_move_dir = movement_velocity.normalized();

        if (is_dashing())
        {
            // Snap velocity for a punchy dash (skip walk friction lerp).
            this->set_velocity(m_dash_dir);
            const double dash_speed{ this->get_movement_speed() * combat::dash_speed_mult };
            this->translate(m_dash_dir * static_cast<float>(dash_speed * delta_time));
            this->move_and_slide();
            this->process_slide_collisions();
            return;
        }

        Character::on_character_movement(movement_velocity, delta_time);
    }

    void Player::on_character_dash()
    {
        if (!is_alive() || is_dashing() || m_dash_cooldown_remaining > 0.0)
            return;

        m_dash_dir = this->resolve_dash_direction();
        m_dash_remaining = combat::dash_duration;
        m_dash_cooldown_remaining = combat::dash_cooldown;

        m_dash_saved_mask = this->get_collision_mask();
        // Pass through enemies during dash; still blocked by walls.
        this->set_collision_mask(collision::walls_layer);
        m_dash_mask_overridden = true;

        this->start_invincibility(combat::dash_invincibility_duration);
        combat_feedback::play_player_dash(this->get_parent(), this->get_global_position());
        this->set_process(true);

        console::get()->print("{} {}", io::blue("dash"), io::yellow("go"));
    }

    void Player::on_character_polar_slash()
    {
        if (!is_alive() || m_polar_slash_cooldown_remaining > 0.0 ||
            m_polar_slash_active_remaining > 0.0)
            return;

        this->begin_polar_slash();
        console::get()->print("{} {}", io::blue("polar slash"), io::yellow("go"));
    }

    void Player::on_character_energy_light()
    {
        if (!is_alive() || m_energy_light_cooldown_remaining > 0.0)
            return;

        this->begin_energy_light();
        console::get()->print("{} {}", io::blue("energy light"), io::yellow("go"));
    }

    godot::Vector2 Player::resolve_dash_direction() const
    {
        if (m_last_move_dir.length_squared() > 0.01f)
            return m_last_move_dir.normalized();

        // Idle: dash along sprite facing (controller adds +90° so local up = aim).
        return godot::Vector2(0.0f, -1.0f).rotated(this->get_rotation()).normalized();
    }

    void Player::update_dash(const double delta_time)
    {
        if (m_dash_remaining > 0.0)
        {
            m_dash_remaining -= delta_time;
            if (m_dash_remaining <= 0.0)
            {
                m_dash_remaining = 0.0;
                this->end_dash_motion();
            }
        }

        if (m_dash_cooldown_remaining > 0.0)
        {
            m_dash_cooldown_remaining = std::max(0.0, m_dash_cooldown_remaining - delta_time);
        }
    }

    void Player::end_dash_motion()
    {
        if (m_dash_mask_overridden)
        {
            this->set_collision_mask(m_dash_saved_mask);
            m_dash_mask_overridden = false;
        }
    }

    void Player::update_skills(const double delta_time)
    {
        if (m_polar_slash_cooldown_remaining > 0.0)
            m_polar_slash_cooldown_remaining =
                std::max(0.0, m_polar_slash_cooldown_remaining - delta_time);

        if (m_energy_light_cooldown_remaining > 0.0)
            m_energy_light_cooldown_remaining =
                std::max(0.0, m_energy_light_cooldown_remaining - delta_time);

        if (m_polar_slash_active_remaining > 0.0)
        {
            this->apply_polar_slash_hits();
            m_polar_slash_active_remaining -= delta_time;
            if (m_polar_slash_visual != nullptr)
            {
                const float alpha{ static_cast<float>(std::clamp(
                    m_polar_slash_active_remaining / combat::polar_slash_active_duration, 0.0,
                    1.0)) };
                m_polar_slash_visual->set_color(godot::Color(0.45f, 0.9f, 1.0f, 0.25f + alpha * 0.45f));
            }
            if (m_polar_slash_active_remaining <= 0.0)
                this->end_polar_slash();
        }

        if (m_energy_light_haste_remaining > 0.0)
        {
            m_energy_light_haste_remaining =
                std::max(0.0, m_energy_light_haste_remaining - delta_time);
            this->update_energy_light_aura_visual();
            if (m_energy_light_haste_remaining <= 0.0)
                this->end_energy_light_haste();
        }
    }

    void Player::begin_polar_slash()
    {
        this->end_polar_slash();

        m_polar_slash_cooldown_remaining = combat::polar_slash_cooldown;
        m_polar_slash_active_remaining = combat::polar_slash_active_duration;
        m_polar_slash_hit_ids.clear();

        const godot::PackedVector2Array fan{ make_slash_fan_points() };

        m_polar_slash_hitbox = memnew(godot::Area2D);
        m_polar_slash_hitbox->set_name("PolarSlashHitbox");
        m_polar_slash_hitbox->set_collision_layer(collision::player_melee_layer);
        m_polar_slash_hitbox->set_collision_mask(collision::player_melee_mask);
        m_polar_slash_hitbox->set_monitoring(true);
        m_polar_slash_hitbox->set_monitorable(false);
        this->add_child(m_polar_slash_hitbox);

        godot::CollisionPolygon2D* poly{ memnew(godot::CollisionPolygon2D) };
        poly->set_polygon(fan);
        m_polar_slash_hitbox->add_child(poly);

        m_polar_slash_visual = memnew(godot::Polygon2D);
        m_polar_slash_visual->set_polygon(fan);
        m_polar_slash_visual->set_color(godot::Color(0.45f, 0.9f, 1.0f, 0.7f));
        m_polar_slash_visual->set_z_index(8);
        this->add_child(m_polar_slash_visual);

        this->set_process(true);
        this->apply_polar_slash_hits();
    }

    void Player::end_polar_slash()
    {
        m_polar_slash_active_remaining = 0.0;
        m_polar_slash_hit_ids.clear();

        if (m_polar_slash_hitbox != nullptr)
        {
            m_polar_slash_hitbox->queue_free();
            m_polar_slash_hitbox = nullptr;
        }
        if (m_polar_slash_visual != nullptr)
        {
            m_polar_slash_visual->queue_free();
            m_polar_slash_visual = nullptr;
        }
    }

    void Player::apply_polar_slash_hits()
    {
        if (m_polar_slash_hitbox == nullptr)
            return;

        const godot::TypedArray<godot::Node2D> bodies{ m_polar_slash_hitbox->get_overlapping_bodies() };
        bool any_hit{ false };
        for (int i = 0; i < bodies.size(); ++i)
        {
            godot::Node2D* node{ godot::Object::cast_to<godot::Node2D>(bodies[i]) };
            Enemy* enemy{ godot::Object::cast_to<Enemy>(node) };
            if (enemy == nullptr || !enemy->is_alive())
                continue;

            const uint64_t id{ enemy->get_instance_id() };
            if (m_polar_slash_hit_ids.contains(id))
                continue;

            m_polar_slash_hit_ids.insert(id);
            if (enemy->take_damage(combat::polar_slash_damage_hearts, true))
            {
                any_hit = true;
                combat_feedback::play_enemy_hit(this->get_parent(), enemy->get_global_position());
            }
        }

        if (any_hit && m_camera != nullptr)
            m_camera->add_trauma(combat::polar_slash_hit_trauma);
    }

    void Player::begin_energy_light()
    {
        m_energy_light_cooldown_remaining = combat::energy_light_cooldown;

        const int healed{ std::min(this->get_hearts() + combat::energy_light_heal_hearts,
                                   this->get_max_hearts()) };
        this->apply_hearts(healed);

        if (!m_energy_light_haste_active)
        {
            m_movement_speed_before_haste = this->get_movement_speed();
            this->set_movement_speed(m_movement_speed_before_haste * combat::energy_light_haste_mult);
            m_energy_light_haste_active = true;
        }
        m_energy_light_haste_remaining = combat::energy_light_haste_duration;

        this->ensure_energy_light_aura();
        this->update_energy_light_aura_visual();
        this->set_process(true);
    }

    void Player::end_energy_light_haste()
    {
        if (m_energy_light_haste_active)
        {
            this->set_movement_speed(m_movement_speed_before_haste);
            m_energy_light_haste_active = false;
        }
        m_energy_light_haste_remaining = 0.0;
        this->update_energy_light_aura_visual();
    }

    void Player::ensure_energy_light_aura()
    {
        if (m_energy_light_aura != nullptr)
            return;

        m_energy_light_aura = memnew(godot::Polygon2D);
        m_energy_light_aura->set_name("EnergyLightAura");
        m_energy_light_aura->set_polygon(make_aura_circle_points(combat::energy_light_aura_radius));
        m_energy_light_aura->set_color(godot::Color(1.0f, 0.82f, 0.28f, 0.0f));
        m_energy_light_aura->set_z_index(-1);
        m_energy_light_aura->set_visible(false);
        this->add_child(m_energy_light_aura);
    }

    void Player::update_energy_light_aura_visual()
    {
        if (m_energy_light_aura == nullptr)
            return;

        if (m_energy_light_haste_remaining <= 0.0)
        {
            m_energy_light_aura->set_visible(false);
            return;
        }

        const float t{ static_cast<float>(std::clamp(
            m_energy_light_haste_remaining / combat::energy_light_haste_duration, 0.0, 1.0)) };
        m_energy_light_aura->set_visible(true);
        m_energy_light_aura->set_color(godot::Color(1.0f, 0.82f, 0.28f, 0.18f + t * 0.28f));
    }

    void Player::process_slide_collisions()
    {
        for (int i = 0; i < this->get_slide_collision_count(); ++i)
        {
            godot::Ref<godot::KinematicCollision2D> collision{ this->get_slide_collision(i) };
            if (!collision.is_valid())
                continue;

            godot::Object* collider{ collision->get_collider() };
            if (collider == nullptr)
                continue;

            if (Enemy* enemy{ godot::Object::cast_to<Enemy>(collider) })
            {
                if (take_damage(combat::enemy_contact_damage_hearts))
                {
                    console::get()->print("{} {}", io::red("enemy contact"),
                                          io::yellow("-1 heart"));
                }
            }
        }

        this->process_area_traps();
    }

    void Player::process_area_traps()
    {
        godot::Node* parent{ this->get_parent() };
        if (parent == nullptr)
            return;

        std::unordered_set<uint64_t> touching{};

        const int child_count{ parent->get_child_count() };
        for (int i = 0; i < child_count; ++i)
        {
            godot::Area2D* area{ godot::Object::cast_to<godot::Area2D>(parent->get_child(i)) };
            if (area == nullptr)
                continue;

            const uint32_t layer{ area->get_collision_layer() };
            const bool is_trap{ (layer & static_cast<uint32_t>(LayerID::DamageZones)) != 0 ||
                                (layer & static_cast<uint32_t>(LayerID::DeathZones)) != 0 };
            if (!is_trap)
                continue;

            if (!area->overlaps_body(this))
                continue;

            const uint64_t id{ area->get_instance_id() };
            touching.insert(id);

            if (!m_active_zone_areas.contains(id))
                this->handle_zone_area(area);
        }

        m_active_zone_areas = std::move(touching);
    }

    void Player::handle_zone_area(godot::Area2D* area)
    {
        if (area == nullptr || !is_alive())
            return;

        const uint32_t layer{ area->get_collision_layer() };
        const auto death_mask{ static_cast<uint32_t>(LayerID::DeathZones) };
        const auto damage_mask{ static_cast<uint32_t>(LayerID::DamageZones) };

        if ((layer & death_mask) != 0)
        {
            take_damage(get_max_hearts(), true);
            console::get()->print("{} {}", io::red("death zone"), io::yellow("instant death"));
            return;
        }

        if ((layer & damage_mask) != 0)
        {
            take_damage(1);
            console::get()->print("{} {}", io::orange("damage zone"), io::yellow("-1 heart"));
        }
    }

    void Player::_bind_methods()
    {
        bind_member_function(Player, on_character_movement);
        bind_member_function(Player, on_character_dash);
        bind_member_function(Player, on_character_polar_slash);
        bind_member_function(Player, on_character_energy_light);
        bind_member_function(Player, is_dashing);
        bind_member_function(Player, get_dash_ready_ratio);
        bind_member_function(Player, get_polar_slash_ready_ratio);
        bind_member_function(Player, get_energy_light_ready_ratio);
        bind_member_function(Player, collect_heal_pickup);
        bind_member_function(Player, collect_haste_pickup);
    }
}
