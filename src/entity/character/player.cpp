#include <algorithm>

#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/classes/kinematic_collision2d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/color.hpp>
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
        }

        // Keep ticking for dash cooldown / motion even after hit-flash settles.
        this->set_process(true);
    }

    void Player::_process(const double delta_time)
    {
        this->update_dash(delta_time);
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
        bind_member_function(Player, is_dashing);
        bind_member_function(Player, get_dash_ready_ratio);
    }
}
