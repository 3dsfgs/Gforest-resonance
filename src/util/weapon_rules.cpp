#include <algorithm>
#include <unordered_map>
#include <vector>

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "core/constants.hpp"
#include "entity/projectile/projectile_spawner.hpp"
#include "singletons/console.hpp"
#include "util/io.hpp"
#include "util/weapon_rules.hpp"

namespace rl::weapon_rules
{
    namespace
    {
        std::unordered_map<std::string, WeaponDef> g_weapons{};
        /** 禁止用全局 godot::String：DLL 静态初始化早于 GDExtension 接口绑定。 */
        std::string g_default_id{ "pulse" };
        bool g_loaded{ false };

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

        godot::String read_string(const godot::Dictionary& dict, const char* key,
                                  const godot::String& fallback = {})
        {
            if (!dict.has(key))
                return fallback;
            return dict[key];
        }

        WeaponDef parse_weapon(const godot::Dictionary& dict)
        {
            WeaponDef def{};
            def.id = read_string(dict, "id", "pulse");
            def.display_name = read_string(dict, "display_name", def.id);
            def.recall_line = read_string(dict, "recall_line");
            def.type = read_string(dict, "type", "ranged");
            def.sfx = read_string(dict, "sfx");
            def.fire_rate = read_double(dict, "fire_rate", combat::projectile_fire_rate);
            def.damage_hearts = std::max(1, read_int(dict, "damage_hearts", 1));
            def.spread_radians =
                read_double(dict, "spread_radians", combat::projectile_spread_radians);
            def.pellet_count = std::max(1, read_int(dict, "pellet_count", 1));
            def.impulse = read_double(dict, "impulse", combat::projectile_impulse);
            def.valid = !def.id.is_empty();
            return def;
        }

        WeaponDef fallback_pulse()
        {
            WeaponDef def{};
            def.id = "pulse";
            def.display_name = godot::String::utf8("脉冲枪");
            def.recall_line = godot::String::utf8("熟悉的光还在手里。");
            def.fire_rate = combat::projectile_fire_rate;
            def.damage_hearts = combat::projectile_damage_hearts;
            def.spread_radians = combat::projectile_spread_radians;
            def.pellet_count = 1;
            def.impulse = combat::projectile_impulse;
            def.sfx = path::audio::weapon_pulse;
            return def;
        }
    }

    void ensure_loaded()
    {
        if (g_loaded)
            return;

        g_loaded = true;
        g_weapons.clear();
        g_default_id = "pulse";

        if (!godot::FileAccess::file_exists(path::data::weapon_rules))
        {
            console::get()->print("{} {}", io::orange("weapon_rules"),
                                  io::yellow("missing, using pulse fallback"));
            const WeaponDef pulse{ fallback_pulse() };
            g_weapons.emplace(std::string{ pulse.id.utf8().get_data() }, pulse);
            return;
        }

        const godot::String text{
            godot::FileAccess::get_file_as_string(path::data::weapon_rules) };
        const godot::Variant parsed{ godot::JSON::parse_string(text) };
        if (parsed.get_type() != godot::Variant::DICTIONARY)
        {
            console::get()->print("{} {}", io::orange("weapon_rules"),
                                  io::yellow("parse failed"));
            const WeaponDef pulse{ fallback_pulse() };
            g_weapons.emplace(std::string{ pulse.id.utf8().get_data() }, pulse);
            return;
        }

        const godot::Dictionary root{ parsed };
        {
            const godot::String def_id{ read_string(root, "default", "pulse") };
            g_default_id = def_id.is_empty() ? "pulse" : std::string{ def_id.utf8().get_data() };
        }

        if (!root.has("weapons") || root["weapons"].get_type() != godot::Variant::ARRAY)
            return;

        const godot::Array weapons{ root["weapons"] };
        for (int i = 0; i < weapons.size(); ++i)
        {
            if (weapons[i].get_type() != godot::Variant::DICTIONARY)
                continue;
            WeaponDef def{ parse_weapon(weapons[i]) };
            if (!def.valid)
                continue;
            g_weapons.emplace(std::string{ def.id.utf8().get_data() }, def);
        }

        if (g_weapons.empty())
        {
            const WeaponDef pulse{ fallback_pulse() };
            g_weapons.emplace(std::string{ pulse.id.utf8().get_data() }, pulse);
        }

        console::get()->print("{} {} weapons", io::green("weapon_rules"),
                              io::blue(std::to_string(g_weapons.size())));
    }

    WeaponDef get_default()
    {
        ensure_loaded();
        return get(godot::String{ g_default_id.c_str() });
    }

    WeaponDef get(const godot::String& weapon_id)
    {
        ensure_loaded();
        const std::string key{ weapon_id.utf8().get_data() };
        const auto it{ g_weapons.find(key) };
        if (it != g_weapons.end())
            return it->second;

        const auto def_it{ g_weapons.find(g_default_id) };
        if (def_it != g_weapons.end())
            return def_it->second;

        return fallback_pulse();
    }

    void apply_to_spawner(ProjectileSpawner* spawner, const WeaponDef& weapon,
                          const double fire_rate_mult)
    {
        if (spawner == nullptr)
            return;

        const double rate{ weapon.fire_rate * std::max(0.5, fire_rate_mult) };
        spawner->set_fire_rate(rate);
        spawner->set_spread_radians(weapon.spread_radians);
        spawner->set_pellet_count(weapon.pellet_count);
        spawner->set_damage_hearts(weapon.damage_hearts);
        spawner->set_impulse(weapon.impulse);
    }
}
