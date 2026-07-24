#include "entity/projectile/projectile_spawner.hpp"
#include "util/bind.hpp"

namespace rl
{
    bool ProjectileSpawner::try_begin_shot()
    {
        const auto elapsed{ clock_t::now() - m_prev_spawn_time };
        if (elapsed < m_spawn_delay)
            return false;

        m_prev_spawn_time = clock_t::now();
        return true;
    }

    Projectile* ProjectileSpawner::create_projectile()
    {
        Projectile* projectile{ m_scene.instantiate() };
        if (projectile == nullptr)
            return nullptr;

        projectile->set_damage_hearts(m_damage_hearts);
        projectile->set_velocity(m_impulse);
        return projectile;
    }

    [[nodiscard]]
    Projectile* ProjectileSpawner::spawn_projectile()
    {
        if (!this->try_begin_shot())
            return nullptr;
        return this->create_projectile();
    }

    [[property]]
    double ProjectileSpawner::get_fire_rate() const
    {
        return m_fire_rate;
    }

    [[property]]
    void ProjectileSpawner::set_fire_rate(double fire_rate)
    {
        m_fire_rate = fire_rate;
        m_spawn_delay = ProjectileSpawner::calculate_spawn_delay(m_fire_rate);
    }

    double ProjectileSpawner::get_spread_radians() const
    {
        return m_spread_radians;
    }

    void ProjectileSpawner::set_spread_radians(const double spread)
    {
        m_spread_radians = spread;
    }

    int ProjectileSpawner::get_pellet_count() const
    {
        return m_pellet_count;
    }

    void ProjectileSpawner::set_pellet_count(const int count)
    {
        m_pellet_count = count < 1 ? 1 : count;
    }

    int ProjectileSpawner::get_damage_hearts() const
    {
        return m_damage_hearts;
    }

    void ProjectileSpawner::set_damage_hearts(const int hearts)
    {
        m_damage_hearts = hearts < 1 ? 1 : hearts;
    }

    double ProjectileSpawner::get_impulse() const
    {
        return m_impulse;
    }

    void ProjectileSpawner::set_impulse(const double impulse)
    {
        m_impulse = impulse;
    }

    ProjectileSpawner::millisec_t ProjectileSpawner::calculate_spawn_delay(double fire_rate)
    {
        // converts fire rate (shots per second) to the time delay between shots in ms.
        // the multiplication by 100 is just to offset the rounding errors by shifting
        // the decimal place to the right a few places before dividing.
        return (1000ms * 100) / static_cast<uint64_t>(fire_rate * 100);
    }

    void ProjectileSpawner::_bind_methods()
    {
        bind_property(ProjectileSpawner, fire_rate, double);
    }
}
