#pragma once

#include <unordered_set>

#include <godot_cpp/classes/area2d.hpp>

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

    protected:
        bool owns_active_camera() const override
        {
            return true;
        }
        godot::Color hit_flash_color() const override;

        void process_slide_collisions() override;

        static void _bind_methods();

    private:
        void process_area_traps();
        void handle_zone_area(godot::Area2D* area);

        std::unordered_set<uint64_t> m_active_zone_areas{};
    };
}
