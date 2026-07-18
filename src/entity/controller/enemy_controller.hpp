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
        enum class Behavior {
            ScoutRanged,
            BruteCharge
        };

        EnemyController();
        ~EnemyController() = default;

        void set_behavior(Behavior behavior);

        void process_action_input(godot::Input* const input, double delta_time) override;
        void process_movement_input(godot::Input* const input, double delta_time) override;
        void process_rotation_input(godot::Input* const input, double delta_time) override;

    protected:
        static void _bind_methods()
        {
        }

    private:
        [[nodiscard]] Player* find_player() const;
        void update_timers(double delta_time);
        void process_scout_movement(Player* player, double delta_time);
        void process_brute_movement(Player* player, double delta_time);
        void try_scout_shoot(Player* player);

        Behavior m_behavior{ Behavior::ScoutRanged };

        double m_shoot_cooldown{ 0.0 };
        double m_charge_cooldown{ 0.0 };
        double m_charge_windup_remaining{ 0.0 };
        double m_charge_active_remaining{ 0.0 };
        godot::Vector2 m_charge_dir{ 0.0, 0.0 };
    };
}
