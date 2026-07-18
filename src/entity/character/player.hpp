#pragma once

#include <unordered_set>

#include <godot_cpp/classes/area2d.hpp>
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

    protected:
        bool owns_active_camera() const override
        {
            return true;
        }
        godot::Color hit_flash_color() const override;

        void process_slide_collisions() override;
        void on_character_movement(godot::Vector2 movement_velocity, double delta_time) override;

        [[signal_slot]] void on_character_dash();

        static void _bind_methods();

    private:
        void process_area_traps();
        void handle_zone_area(godot::Area2D* area);
        void update_dash(double delta_time);
        void end_dash_motion();
        [[nodiscard]] godot::Vector2 resolve_dash_direction() const;

        std::unordered_set<uint64_t> m_active_zone_areas{};
        godot::Vector2 m_last_move_dir{ 0.0f, -1.0f };
        godot::Vector2 m_dash_dir{ 0.0f, -1.0f };
        double m_dash_remaining{ 0.0 };
        double m_dash_cooldown_remaining{ 0.0 };
        uint32_t m_dash_saved_mask{ 0 };
        bool m_dash_mask_overridden{ false };
    };
}
