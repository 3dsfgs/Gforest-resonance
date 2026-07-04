#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "core/constants.hpp"
#include "entity/character/player.hpp"
#include "entity/controller/enemy_controller.hpp"
#include "util/conversions.hpp"

namespace rl
{
    EnemyController::EnemyController()
    {
        m_input_mode = InputMode::AI;
    }

    Player* EnemyController::find_player() const
    {
        godot::Node* const enemy_node{ this->get_parent() };
        if (enemy_node == nullptr)
            return nullptr;

        godot::Node* const level{ enemy_node->get_parent() };
        if (level == nullptr)
            return nullptr;

        return gdcast<Player>(level->find_child(name::character::player, true, false));
    }

    void EnemyController::process_action_input(godot::Input* const input, double delta_time)
    {
    }

    void EnemyController::process_movement_input(godot::Input* const input, double delta_time)
    {
        Player* const player{ this->find_player() };
        if (player == nullptr || !player->is_alive())
        {
            this->emit_signal(event::character_move, godot::Vector2{}, delta_time);
            return;
        }

        godot::Vector2 to_player{ player->get_global_position() - this->get_global_position() };

        constexpr double stop_distance{ 8.0 };
        if (to_player.length_squared() < stop_distance * stop_distance)
        {
            this->emit_signal(event::character_move, godot::Vector2{}, delta_time);
            return;
        }

        this->emit_signal(event::character_move, to_player.normalized(), delta_time);
    }

    void EnemyController::process_rotation_input(godot::Input* const input, double delta_time)
    {
        Player* const player{ this->find_player() };
        if (player == nullptr || !player->is_alive())
            return;

        godot::Vector2 rotation_dir{ player->get_global_position() - this->get_global_position() };
        if (rotation_dir.is_zero_approx())
            return;

        m_rotation_angle = rotation_dir.angle() + godot::Math::deg_to_rad(90.0);
        this->emit_signal(event::character_rotate, m_rotation_angle, delta_time);
    }
}
