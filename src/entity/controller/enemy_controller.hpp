#pragma once

#include "entity/controller/character_controller.hpp"

namespace godot
{
    class Input;
}

namespace rl
{
    class Player;

    class EnemyController : public CharacterController
    {
        GDCLASS(EnemyController, CharacterController);

    public:
        EnemyController();
        ~EnemyController() = default;

        void process_action_input(godot::Input* const input, double delta_time) override;
        void process_movement_input(godot::Input* const input, double delta_time) override;
        void process_rotation_input(godot::Input* const input, double delta_time) override;

    protected:
        static void _bind_methods()
        {
        }

    private:
        [[nodiscard]] Player* find_player() const;
    };
}
