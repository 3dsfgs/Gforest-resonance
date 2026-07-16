#pragma once

#include <unordered_set>

#include <godot_cpp/classes/physics_body2d.hpp>

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
        void handle_zone_contact(godot::PhysicsBody2D* body);

        std::unordered_set<uint64_t> m_active_zone_colliders{};
    };
}
