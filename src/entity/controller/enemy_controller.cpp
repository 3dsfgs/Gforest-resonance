#include <algorithm>

#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "core/constants.hpp"
#include "entity/character/character.hpp"
#include "entity/character/player.hpp"
#include "entity/controller/enemy_controller.hpp"
#include "util/conversions.hpp"

namespace rl
{
    EnemyController::EnemyController()
    {
        m_input_mode = InputMode::AI;
    }

    void EnemyController::set_behavior(const Behavior behavior)
    {
        m_behavior = behavior;
    }

    Player* EnemyController::find_player() const
    {
        godot::Node* const enemy_node{ this->get_parent() };
        if (enemy_node == nullptr)
            return nullptr;

        godot::Node* const level{ enemy_node->get_parent() };
        if (level == nullptr)
            return nullptr;

        return try_gdcast<Player>(level->find_child(name::character::player, true, false));
    }

    void EnemyController::update_timers(const double delta_time)
    {
        if (m_shoot_cooldown > 0.0)
            m_shoot_cooldown = std::max(0.0, m_shoot_cooldown - delta_time);
        if (m_charge_cooldown > 0.0)
            m_charge_cooldown = std::max(0.0, m_charge_cooldown - delta_time);
        if (m_charge_windup_remaining > 0.0)
            m_charge_windup_remaining = std::max(0.0, m_charge_windup_remaining - delta_time);
        if (m_charge_active_remaining > 0.0)
            m_charge_active_remaining = std::max(0.0, m_charge_active_remaining - delta_time);
    }

    void EnemyController::process_action_input(godot::Input* const input, double delta_time)
    {
        (void)input;
        this->update_timers(delta_time);

        if (m_behavior != Behavior::ScoutRanged)
            return;

        Player* const player{ this->find_player() };
        if (player == nullptr || !player->is_alive())
            return;

        this->try_scout_shoot(player);
    }

    void EnemyController::try_scout_shoot(Player* player)
    {
        if (m_shoot_cooldown > 0.0)
            return;

        const godot::Vector2 to_player{ player->get_global_position() - this->get_global_position() };
        const float dist{ to_player.length() };
        if (dist > combat::enemy_ranged_range || dist < combat::enemy_ranged_min_range)
            return;

        this->emit_signal(event::character_shoot);
        m_shoot_cooldown = 1.0 / std::max(0.1, combat::enemy_ranged_fire_rate);
    }

    void EnemyController::process_scout_movement(Player* player, const double delta_time)
    {
        godot::Vector2 to_player{ player->get_global_position() - this->get_global_position() };
        const float dist{ to_player.length() };

        // Prefer mid-range: back off if too close, chase if too far.
        if (dist < combat::enemy_ranged_min_range)
        {
            this->emit_signal(event::character_move, -to_player.normalized(), delta_time);
            return;
        }

        if (dist > combat::enemy_ranged_range * 0.85f)
        {
            this->emit_signal(event::character_move, to_player.normalized(), delta_time);
            return;
        }

        this->emit_signal(event::character_move, godot::Vector2{}, delta_time);
    }

    void EnemyController::process_brute_movement(Player* player, const double delta_time)
    {
        godot::Vector2 to_player{ player->get_global_position() - this->get_global_position() };
        const float dist{ to_player.length() };

        if (m_charge_active_remaining > 0.0)
        {
            this->emit_signal(event::character_move, m_charge_dir, delta_time);
            return;
        }

        if (m_charge_windup_remaining > 0.0)
        {
            // Wind-up: stop and face player; direction locked when charge starts.
            if (m_charge_windup_remaining <= delta_time + 1e-6)
            {
                m_charge_dir = to_player.length_squared() > 1.0f ? to_player.normalized()
                                                                 : godot::Vector2(1, 0);
                m_charge_active_remaining = combat::enemy_charge_duration;
                m_charge_cooldown = combat::enemy_charge_cooldown;
                Character* const body{ try_gdcast<Character>(this->get_parent()) };
                if (body != nullptr)
                    body->set_movement_speed(combat::enemy_brute_movement_speed *
                                             combat::enemy_charge_speed_mult);
            }
            this->emit_signal(event::character_move, godot::Vector2{}, delta_time);
            return;
        }

        // Restore walk speed after charge ends.
        Character* const body{ try_gdcast<Character>(this->get_parent()) };
        if (body != nullptr &&
            godot::Math::abs(body->get_movement_speed() -
                             combat::enemy_brute_movement_speed) > 1.0)
        {
            body->set_movement_speed(combat::enemy_brute_movement_speed);
        }

        if (m_charge_cooldown <= 0.0 && dist <= combat::enemy_charge_trigger_range &&
            dist > 24.0f)
        {
            m_charge_windup_remaining = combat::enemy_charge_windup;
            this->emit_signal(event::character_move, godot::Vector2{}, delta_time);
            return;
        }

        constexpr double stop_distance{ 10.0 };
        if (to_player.length_squared() < stop_distance * stop_distance)
        {
            this->emit_signal(event::character_move, godot::Vector2{}, delta_time);
            return;
        }

        this->emit_signal(event::character_move, to_player.normalized(), delta_time);
    }

    void EnemyController::process_movement_input(godot::Input* const input, double delta_time)
    {
        (void)input;
        this->update_timers(delta_time);

        Player* const player{ this->find_player() };
        if (player == nullptr || !player->is_alive())
        {
            this->emit_signal(event::character_move, godot::Vector2{}, delta_time);
            return;
        }

        if (m_behavior == Behavior::BruteCharge)
            this->process_brute_movement(player, delta_time);
        else
            this->process_scout_movement(player, delta_time);
    }

    void EnemyController::process_rotation_input(godot::Input* const input, double delta_time)
    {
        (void)input;

        Player* const player{ this->find_player() };
        if (player == nullptr || !player->is_alive())
            return;

        godot::Vector2 rotation_dir;
        if (m_behavior == Behavior::BruteCharge && m_charge_active_remaining > 0.0)
            rotation_dir = m_charge_dir;
        else
            rotation_dir = player->get_global_position() - this->get_global_position();

        if (rotation_dir.is_zero_approx())
            return;

        m_rotation_angle = rotation_dir.angle() + godot::Math::deg_to_rad(90.0);
        this->emit_signal(event::character_rotate, m_rotation_angle, delta_time);
    }
}
