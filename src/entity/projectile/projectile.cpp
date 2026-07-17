#include <godot_cpp/classes/physics_body2d.hpp>
#include <godot_cpp/classes/physics_direct_body_state2d.hpp>
#include <godot_cpp/classes/physics_material.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "core/constants.hpp"
#include "entity/character/character.hpp"
#include "entity/character/enemy.hpp"
#include "entity/projectile/projectile.hpp"
#include "singletons/console.hpp"
#include "util/bind.hpp"
#include "util/combat_feedback.hpp"
#include "util/conversions.hpp"
#include "util/engine.hpp"
#include "util/io.hpp"

namespace rl
{
    void Projectile::_ready()
    {
        godot::Ref<godot::PhysicsMaterial> material{ memnew(godot::PhysicsMaterial) };
        material->set_bounce(combat::projectile_physics_bounce);
        material->set_friction(combat::projectile_physics_friction);
        this->set_physics_material_override(material);

        m_start_pos = this->get_global_position();
        auto forward{ this->get_transform()[0].normalized() };
        this->apply_impulse(forward * m_velocity);

        signal<event::body_entered>::connect<Projectile>(this) <=>
            signal_callback(this, on_body_entered);
    }

    void Projectile::sync_facing_to_velocity(const godot::Vector2& velocity)
    {
        if (velocity.length_squared() < 1.0)
            return;

        this->set_rotation(velocity.angle());
    }

    void Projectile::restore_speed_along_velocity(godot::PhysicsDirectBodyState2D* state)
    {
        if (state == nullptr)
            return;

        const godot::Vector2 velocity{ state->get_linear_velocity() };
        if (velocity.length_squared() < 1.0 || m_flight_speed <= 0.0)
            return;

        const godot::Vector2 dir{ velocity.normalized() };
        state->set_linear_velocity(dir * m_flight_speed);
        this->set_rotation(dir.angle());
    }

    void Projectile::_integrate_forces(godot::PhysicsDirectBodyState2D* state)
    {
        if (state == nullptr || m_hit)
            return;

        const godot::Vector2 velocity{ state->get_linear_velocity() };

        if (!m_flight_speed_captured && velocity.length_squared() > 1.0)
        {
            m_flight_speed = velocity.length();
            m_flight_speed_captured = true;
        }

        if (m_pending_bounce_realign)
        {
            // Physics bounce already flipped velocity; re-apply launch speed + face new heading.
            restore_speed_along_velocity(state);
            m_pending_bounce_realign = false;
            return;
        }

        sync_facing_to_velocity(velocity);
    }

    [[signal_slot]]
    void Projectile::on_body_entered(godot::Node* body)
    {
        if (m_hit || body == nullptr)
            return;

        if (Enemy* enemy{ godot::Object::cast_to<Enemy>(body) })
        {
            if (enemy->take_damage(combat::projectile_damage_hearts))
            {
                combat_feedback::play_enemy_hit(this->get_parent(),
                                                enemy->get_global_position());
                console::get()->print("{} {} ({}/{})", io::yellow("pulse hit"),
                                      io::green(to<std::string>(enemy->get_name())),
                                      io::orange(enemy->get_hearts()),
                                      io::orange(enemy->get_max_hearts()));
            }

            m_hit = true;
            this->queue_free();
            return;
        }

        if (Character* character{ godot::Object::cast_to<Character>(body) })
        {
            if (!m_has_bounced)
                return;

            const float dist_from_bounce{
                static_cast<float>(character->get_global_position().distance_to(m_bounce_point))
            };
            if (dist_from_bounce > combat::ricochet_self_damage_radius)
                return;

            if (character->take_damage(combat::ricochet_self_damage_hearts))
            {
                console::get()->print("{} {}", io::red("ricochet self-hit"),
                                      io::yellow("-1 heart"));
            }

            m_hit = true;
            this->queue_free();
            return;
        }

        if (godot::PhysicsBody2D* physics_body{ godot::Object::cast_to<godot::PhysicsBody2D>(body) })
        {
            const uint32_t layer{ physics_body->get_collision_layer() };
            const auto walls_mask{ static_cast<uint32_t>(LayerID::Walls) };
            const auto physics_objects_mask{ static_cast<uint32_t>(LayerID::PhysicsObjects) };
            if ((layer & walls_mask) == 0 && (layer & physics_objects_mask) == 0)
                return;

            if (m_wall_bounce_count >= combat::projectile_max_wall_bounces)
            {
                m_hit = true;
                this->queue_free();
                return;
            }

            ++m_wall_bounce_count;
            m_has_bounced = true;
            m_bounce_point = this->get_global_position();
            m_pending_bounce_realign = true;
            this->set_collision_mask(this->get_collision_mask() | collision::player_layer);
        }
    }

    void Projectile::_process(double delta_time)
    {
        if (engine::editor_active())
            return;

        m_time_to_live -= delta_time;
        if (m_time_to_live <= 0)
        {
            this->queue_free();
            return;
        }

        godot::Vector2 curr_pos{ this->get_global_position() };
        double dist_traveled{ m_start_pos.distance_squared_to(curr_pos) };
        if (dist_traveled >= m_max_travel_dist)
        {
            this->queue_free();
            return;
        }
    }

    [[property]]
    double Projectile::get_movement_speed() const
    {
        return m_movement_speed;
    }

    [[property]]
    double Projectile::get_time_to_live() const
    {
        return m_time_to_live;
    }

    [[property]]
    double Projectile::get_acceleration() const
    {
        return m_acceleration;
    }

    [[property]]
    double Projectile::get_max_travel_dist() const
    {
        return godot::Math::sqrt(m_max_travel_dist);
    }

    [[property]]
    double Projectile::get_velocity() const
    {
        return m_velocity;
    }

    [[property]]
    void Projectile::set_movement_speed(double speed)
    {
        m_movement_speed = speed;
    }

    [[property]]
    void Projectile::set_time_to_live(double ttl)
    {
        m_time_to_live = ttl;
    }

    [[property]]
    void Projectile::set_acceleration(double acceleration)
    {
        m_acceleration = acceleration;
    }

    [[property]]
    void Projectile::set_max_travel_dist(double dist)
    {
        m_max_travel_dist = dist * dist;
    }

    [[property]]
    void Projectile::set_velocity(double velocity)
    {
        m_velocity = velocity;
    }
}
