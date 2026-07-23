#include <algorithm>

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "core/constants.hpp"
#include "entity/character/player.hpp"
#include "entity/projectile/projectile_spawner.hpp"
#include "singletons/console.hpp"
#include "util/birth_buff.hpp"
#include "util/io.hpp"

namespace rl::birth_buff
{
    namespace
    {
        constexpr const char* profile_path{ "user://profile.json" };
        constexpr double base_move_speed{ 500.0 };
        constexpr int base_max_hearts{ 5 };

        double read_double(const godot::Dictionary& dict, const char* key, const double fallback)
        {
            if (!dict.has(key))
                return fallback;
            return static_cast<double>(dict[key]);
        }

        int read_int(const godot::Dictionary& dict, const char* key, const int fallback)
        {
            if (!dict.has(key))
                return fallback;
            return static_cast<int>(dict[key]);
        }

        godot::String read_string(const godot::Dictionary& dict, const char* key)
        {
            if (!dict.has(key))
                return {};
            return dict[key];
        }
    }

    Profile load_profile()
    {
        Profile profile{};

        if (!godot::FileAccess::file_exists(profile_path))
            return profile;

        const godot::String text{
            godot::FileAccess::get_file_as_string(profile_path) };
        if (text.is_empty())
            return profile;

        const godot::Variant parsed{ godot::JSON::parse_string(text) };
        if (parsed.get_type() != godot::Variant::DICTIONARY)
            return profile;

        const godot::Dictionary root{ parsed };
        profile.year = read_int(root, "year", 2000);
        profile.month = read_int(root, "month", 1);
        profile.day = read_int(root, "day", 1);
        profile.fate_id = read_string(root, "fate_id");
        profile.title = read_string(root, "title");
        profile.desc = read_string(root, "desc");
        profile.buff_summary = read_string(root, "buff_summary");

        godot::Dictionary buffs{};
        if (root.has("buffs") && root["buffs"].get_type() == godot::Variant::DICTIONARY)
            buffs = root["buffs"];

        profile.max_hearts_delta = read_int(buffs, "max_hearts_delta", 0);
        profile.movement_speed_mult = read_double(buffs, "movement_speed_mult", 1.0);
        profile.fire_rate_mult = read_double(buffs, "fire_rate_mult", 1.0);
        profile.valid = !profile.fate_id.is_empty() || !profile.title.is_empty();
        return profile;
    }

    void apply(Player* player, ProjectileSpawner* spawner, const Profile& profile)
    {
        if (player == nullptr || !profile.valid)
            return;

        const int max_hearts{ std::max(1, base_max_hearts + profile.max_hearts_delta) };
        player->set_max_hearts(max_hearts);
        player->set_hearts(max_hearts);

        const double move_speed{ base_move_speed * std::max(0.5, profile.movement_speed_mult) };
        player->set_movement_speed(move_speed);

        if (spawner != nullptr)
        {
            const double fire_rate{ combat::projectile_fire_rate *
                                    std::max(0.5, profile.fire_rate_mult) };
            spawner->set_fire_rate(fire_rate);
        }

        console::get()->print("{} {} · {}", io::green("命格"),
                              io::yellow(profile.title.utf8().get_data()),
                              io::blue(profile.buff_summary.utf8().get_data()));
    }

    bool apply_from_disk(Player* player, ProjectileSpawner* spawner)
    {
        const Profile profile{ load_profile() };
        if (!profile.valid)
            return false;

        apply(player, spawner, profile);
        return true;
    }
}
