#pragma once

#include <godot_cpp/variant/string.hpp>

namespace godot
{
    class Node;
}

namespace rl
{
    class Player;
    class ProjectileSpawner;
}

namespace rl::birth_buff
{
    struct Profile
    {
        int year{ 2000 };
        int month{ 1 };
        int day{ 1 };
        godot::String fate_id{};
        godot::String title{};
        godot::String desc{};
        godot::String buff_summary{};
        int max_hearts_delta{ 0 };
        double movement_speed_mult{ 1.0 };
        double fire_rate_mult{ 1.0 };
        bool valid{ false };
    };

    /** Load user://profile.json written by the birthday UI. */
    [[nodiscard]] Profile load_profile();

    /** Apply profile buffs to a freshly spawned player + spawner. */
    void apply(Player* player, ProjectileSpawner* spawner, const Profile& profile);

    /** Convenience: load + apply. Returns true if a valid profile was applied. */
    bool apply_from_disk(Player* player, ProjectileSpawner* spawner);
}
