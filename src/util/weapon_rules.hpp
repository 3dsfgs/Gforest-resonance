#pragma once

#include <godot_cpp/variant/string.hpp>

namespace rl
{
    class ProjectileSpawner;
}

namespace rl::weapon_rules
{
    struct WeaponDef
    {
        godot::String id{ "pulse" };
        godot::String display_name{ "脉冲枪" };
        godot::String recall_line{};
        godot::String type{ "ranged" };
        godot::String sfx{};
        double fire_rate{ 7.0 };
        int damage_hearts{ 1 };
        double spread_radians{ 0.04 };
        int pellet_count{ 1 };
        double impulse{ 1500.0 };
        bool valid{ true };
    };

    /** Load (or reload) res://data/weapon_rules.json into cache. */
    void ensure_loaded();

    [[nodiscard]] WeaponDef get_default();
    [[nodiscard]] WeaponDef get(const godot::String& weapon_id);

    /** Apply ranged stats onto the player projectile spawner. */
    void apply_to_spawner(ProjectileSpawner* spawner, const WeaponDef& weapon,
                          double fire_rate_mult = 1.0);
}
