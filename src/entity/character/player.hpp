#pragma once

#include <unordered_set>

#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/classes/polygon2d.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "core/constants.hpp"
#include "entity/character/character.hpp"
#include "util/bind.hpp"

namespace rl
{
    class Player : public Character
    {
        GDCLASS(Player, Character);

    public:
        Player();
        ~Player() = default;

        void _ready() override;
        void _process(double delta_time) override;

        [[nodiscard]] bool is_dashing() const;
        /** 1 = ready, 0 = just used. */
        [[nodiscard]] double get_dash_ready_ratio() const;
        [[nodiscard]] double get_polar_slash_ready_ratio() const;
        [[nodiscard]] double get_energy_light_ready_ratio() const;

        /** ⭐2 地面拾取：回血 / 短时加速。 */
        void collect_heal_pickup(int hearts);
        void collect_haste_pickup(double duration, double speed_mult);

    protected:
        bool owns_active_camera() const override
        {
            return true;
        }
        godot::Color hit_flash_color() const override;

        void process_slide_collisions() override;
        void on_character_movement(godot::Vector2 movement_velocity, double delta_time) override;

        [[signal_slot]] void on_character_dash();
        [[signal_slot]] void on_character_polar_slash();
        [[signal_slot]] void on_character_energy_light();

        static void _bind_methods();

    private:
        void process_area_traps();
        void handle_zone_area(godot::Area2D* area);
        void update_dash(double delta_time);
        void end_dash_motion();
        [[nodiscard]] godot::Vector2 resolve_dash_direction() const;

        void update_skills(double delta_time);
        void begin_polar_slash();
        void end_polar_slash();
        void apply_polar_slash_hits();
        void begin_energy_light();
        void end_energy_light_haste();
        void ensure_energy_light_aura();
        void update_energy_light_aura_visual();

        std::unordered_set<uint64_t> m_active_zone_areas{};
        std::unordered_set<uint64_t> m_polar_slash_hit_ids{};
        godot::Vector2 m_last_move_dir{ 0.0f, -1.0f };
        godot::Vector2 m_dash_dir{ 0.0f, -1.0f };
        double m_dash_remaining{ 0.0 };
        double m_dash_cooldown_remaining{ 0.0 };
        uint32_t m_dash_saved_mask{ 0 };
        bool m_dash_mask_overridden{ false };

        double m_polar_slash_cooldown_remaining{ 0.0 };
        double m_polar_slash_active_remaining{ 0.0 };
        godot::Area2D* m_polar_slash_hitbox{ nullptr };
        godot::Polygon2D* m_polar_slash_visual{ nullptr };

        double m_energy_light_cooldown_remaining{ 0.0 };
        double m_energy_light_haste_remaining{ 0.0 };
        double m_movement_speed_before_haste{ 0.0 };
        bool m_energy_light_haste_active{ false };
        godot::Polygon2D* m_energy_light_aura{ nullptr };
    };
}
