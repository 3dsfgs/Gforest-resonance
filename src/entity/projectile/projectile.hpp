#pragma once

#include <godot_cpp/classes/physics_direct_body_state2d.hpp>
#include <godot_cpp/classes/rigid_body2d.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "core/attributes.hpp"
#include "core/constants.hpp"
#include "singletons/console.hpp"
#include "util/bind.hpp"
#include "util/conversions.hpp"
#include "util/io.hpp"

namespace godot
{
    class Node;
}

namespace rl
{
    class Projectile : public godot::RigidBody2D
    {
        GDCLASS(Projectile, godot::RigidBody2D);

    public:
        Projectile() = default;
        virtual ~Projectile() = default;

        void _ready() override;
        void _process(double delta_time) override;
        void _integrate_forces(godot::PhysicsDirectBodyState2D* state) override;

        [[property]] double get_movement_speed() const;
        [[property]] double get_time_to_live() const;
        [[property]] double get_max_travel_dist() const;
        [[property]] double get_acceleration() const;
        [[property]] double get_velocity() const;

        [[property]] void set_movement_speed(double speed);
        [[property]] void set_time_to_live(double ttl);
        [[property]] void set_max_travel_dist(double dist);
        [[property]] void set_acceleration(double acceleration);
        [[property]] void set_velocity(double velocity);

        /** Scout ranged: collide with Player; do not damage allied enemies. */
        void configure_as_enemy_shot();

    protected:
        static void _bind_methods()
        {
            bind_member_function(Projectile, get_movement_speed);
            bind_member_function(Projectile, get_time_to_live);
            bind_member_function(Projectile, get_max_travel_dist);
            bind_member_function(Projectile, get_acceleration);
            bind_member_function(Projectile, get_velocity);
            bind_member_function(Projectile, set_movement_speed);
            bind_member_function(Projectile, set_time_to_live);
            bind_member_function(Projectile, set_max_travel_dist);
            bind_member_function(Projectile, set_acceleration);
            bind_member_function(Projectile, set_velocity);
            bind_member_function(Projectile, on_body_entered);
        }

        [[signal_slot]] void on_body_entered(godot::Node* body);

        void sync_facing_to_velocity(const godot::Vector2& velocity);
        void restore_speed_along_velocity(godot::PhysicsDirectBodyState2D* state);

    protected:
        godot::Vector2 m_start_pos{ 0.0, 0.0 };
        godot::Vector2 m_bounce_point{ 0.0, 0.0 };
        bool m_hit{ false };
        bool m_has_bounced{ false };
        bool m_pending_bounce_realign{ false };
        bool m_flight_speed_captured{ false };
        bool m_hostile_to_player{ false };
        int m_wall_bounce_count{ 0 };
        /** Captured flight speed after launch; restored after wall bounce. */
        double m_flight_speed{ 0.0 };
        // projectile movement velocity / impulse (pixels)
        double m_velocity{ combat::projectile_impulse };
        // projectile movement speed (pixels/s) — legacy property
        double m_movement_speed{ 1000.0 };
        // projectile acceleration (pixels/s/s)
        double m_acceleration{ 100.0 };
        // max time duration alive (seconds)
        double m_time_to_live{ combat::projectile_time_to_live };
        // max travel distance squared (pixels^2)
        double m_max_travel_dist{ combat::projectile_max_travel * combat::projectile_max_travel };
    };
}
