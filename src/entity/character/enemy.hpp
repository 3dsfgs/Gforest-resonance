#pragma once

#include "core/constants.hpp"
#include "entity/character/character.hpp"
#include "util/bind.hpp"

namespace godot
{
    class Node;
}

namespace rl
{
    class Enemy : public Character
    {
        GDCLASS(Enemy, Character);

    public:
        Enemy();

        void _ready() override;

    protected:
        static void _bind_methods();

        [[signal_slot]] void on_died();
    };
}
