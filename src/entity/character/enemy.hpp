#pragma once

#include "core/constants.hpp"
#include "entity/character/character.hpp"
#include "util/bind.hpp"
#include "util/combat_feedback.hpp"

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

        [[nodiscard]] virtual combat_feedback::KillVfxKind kill_vfx_kind() const
        {
            return combat_feedback::KillVfxKind::Default;
        }

    protected:
        static void _bind_methods();

        [[signal_slot]] void on_died();
    };
}
