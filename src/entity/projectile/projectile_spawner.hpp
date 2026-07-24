#pragma once

#include <chrono>

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include "core/constants.hpp"
#include "entity/projectile/projectile.hpp"
#include "util/bind.hpp"
#include "util/scene.hpp"

namespace rl
{
    using namespace std::chrono_literals;

    class ProjectileSpawner : public godot::Node2D
    {
        GDCLASS(ProjectileSpawner, godot::Node2D);

    public:
        ProjectileSpawner() = default;
        ~ProjectileSpawner() = default;

        Projectile* spawn_projectile();
        /** Consume fire-rate cooldown; returns false if still cooling down. */
        bool try_begin_shot();
        /** Always instantiate a configured projectile (no cooldown). */
        Projectile* create_projectile();

        double get_fire_rate() const;
        void set_fire_rate(double fire_rate);
        double get_spread_radians() const;
        void set_spread_radians(double spread);
        int get_pellet_count() const;
        void set_pellet_count(int count);
        int get_damage_hearts() const;
        void set_damage_hearts(int hearts);
        double get_impulse() const;
        void set_impulse(double impulse);

    protected:
        static void _bind_methods();

    private:
        using clock_t = std::chrono::high_resolution_clock;
        using millisec_t = std::chrono::milliseconds;
        static millisec_t calculate_spawn_delay(double fire_rate);

    private:
        // number of projectiles per second
        double m_fire_rate{ combat::projectile_fire_rate };
        double m_spread_radians{ combat::projectile_spread_radians };
        double m_impulse{ combat::projectile_impulse };
        int m_pellet_count{ 1 };
        int m_damage_hearts{ combat::projectile_damage_hearts };
        // time delay between shots (ms). multiplication by 100 is just to offset rounding errors.
        millisec_t m_spawn_delay{ ProjectileSpawner::calculate_spawn_delay(m_fire_rate) };
        // the time point that keeps track of when the last projectile was spawned.
        clock_t::time_point m_prev_spawn_time{ clock_t::now() };
        // preloaded packed scene that will be instantiated per spawn
        resource::preload::packed_scene<Projectile> m_scene{ path::scene::Bullet };
    };
}
