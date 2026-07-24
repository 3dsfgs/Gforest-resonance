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
        if (behavior == Behavior::HeartDemon)
            m_special_cooldown = 1.5;
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

    Character* EnemyController::find_body() const
    {
        return try_gdcast<Character>(this->get_parent());
    }

    bool EnemyController::boss_in_phase2() const
    {
        Character* const body{ this->find_body() };
        if (body == nullptr || body->get_max_hearts() <= 0)
            return false;
        return static_cast<float>(body->get_hearts()) <=
               static_cast<float>(body->get_max_hearts()) * combat::enemy_boss_phase2_ratio;
    }

    void EnemyController::update_timers(const double delta_time)
    {
        if (m_shoot_cooldown > 0.0)
            m_shoot_cooldown = std::max(0.0, m_shoot_cooldown - delta_time);
        if (m_special_cooldown > 0.0)
            m_special_cooldown = std::max(0.0, m_special_cooldown - delta_time);
        if (m_charge_cooldown > 0.0)
            m_charge_cooldown = std::max(0.0, m_charge_cooldown - delta_time);
        if (m_charge_active_remaining > 0.0)
            m_charge_active_remaining = std::max(0.0, m_charge_active_remaining - delta_time);
        // windup 在移动分支内自行倒数，避免归零后丢失「开冲」边沿。
    }

    void EnemyController::process_action_input(godot::Input* const input, double delta_time)
    {
        (void)input;
        (void)delta_time;

        Player* const player{ this->find_player() };
        if (player == nullptr || !player->is_alive())
            return;

        if (m_behavior == Behavior::ScoutRanged)
            this->try_scout_shoot(player);
        else if (m_behavior == Behavior::HeartDemon)
            this->try_boss_actions(player);
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

    void EnemyController::fire_boss_burst(Player* player)
    {
        Character* const body{ this->find_body() };
        if (body == nullptr)
            return;

        const godot::Vector2 to_player{ player->get_global_position() - body->get_global_position() };
        if (to_player.length_squared() < 1.0f)
            return;

        // FiringPoint 在 CollisionPoly（本地 -90°）下；与普攻瞄准同一套朝向约定。
        const double base_angle{ static_cast<double>(to_player.angle()) +
                                 godot::Math::deg_to_rad(90.0) };
        const double saved_rotation{ static_cast<double>(body->get_rotation()) };
        const int count{ combat::enemy_boss_burst_count };
        const int mid{ count / 2 };

        for (int i = 0; i < count; ++i)
        {
            const double offset{ static_cast<double>(i - mid) * combat::enemy_boss_burst_spread };
            body->set_rotation(base_angle + offset);
            this->emit_signal(event::character_shoot);
        }

        body->set_rotation(saved_rotation);
        m_special_cooldown = combat::enemy_boss_special_cooldown;
        m_shoot_cooldown = std::max(m_shoot_cooldown, 0.55);
    }

    void EnemyController::try_boss_actions(Player* player)
    {
        if (m_charge_windup_remaining > 0.0 || m_charge_active_remaining > 0.0)
            return;

        const godot::Vector2 to_player{ player->get_global_position() - this->get_global_position() };
        const float dist{ to_player.length() };

        // 优先特殊：心影爆发（扇形）——冷却好且在射程内就放。
        if (m_special_cooldown <= 0.0 && dist <= combat::enemy_boss_ranged_range &&
            dist >= 80.0f)
        {
            this->fire_boss_burst(player);
            return;
        }

        if (m_shoot_cooldown > 0.0)
            return;
        if (dist > combat::enemy_boss_ranged_range || dist < combat::enemy_boss_ranged_min_range)
            return;

        this->emit_signal(event::character_shoot);
        m_shoot_cooldown = 1.0 / std::max(0.1, combat::enemy_boss_fire_rate);
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
            m_charge_windup_remaining = std::max(0.0, m_charge_windup_remaining - delta_time);
            if (m_charge_windup_remaining <= 1e-6)
            {
                m_charge_dir = to_player.length_squared() > 1.0f ? to_player.normalized()
                                                                 : godot::Vector2(1, 0);
                m_charge_active_remaining = combat::enemy_charge_duration;
                m_charge_cooldown = combat::enemy_charge_cooldown;
                Character* const body{ this->find_body() };
                if (body != nullptr)
                    body->set_movement_speed(combat::enemy_brute_movement_speed *
                                             combat::enemy_charge_speed_mult);
            }
            this->emit_signal(event::character_move, godot::Vector2{}, delta_time);
            return;
        }

        // Restore walk speed after charge ends.
        Character* const body{ this->find_body() };
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

    void EnemyController::process_boss_movement(Player* player, const double delta_time)
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
            m_charge_windup_remaining = std::max(0.0, m_charge_windup_remaining - delta_time);
            if (m_charge_windup_remaining <= 1e-6)
            {
                m_charge_dir = to_player.length_squared() > 1.0f ? to_player.normalized()
                                                                 : godot::Vector2(1, 0);
                m_charge_active_remaining = combat::enemy_charge_duration;
                m_charge_cooldown = this->boss_in_phase2()
                                        ? combat::enemy_boss_charge_cooldown_phase2
                                        : combat::enemy_boss_charge_cooldown;
                Character* const body{ this->find_body() };
                if (body != nullptr)
                    body->set_movement_speed(combat::enemy_boss_movement_speed *
                                             combat::enemy_charge_speed_mult);
            }
            this->emit_signal(event::character_move, godot::Vector2{}, delta_time);
            return;
        }

        Character* const body{ this->find_body() };
        if (body != nullptr &&
            godot::Math::abs(body->get_movement_speed() - combat::enemy_boss_movement_speed) > 1.0)
        {
            body->set_movement_speed(combat::enemy_boss_movement_speed);
        }

        // 暗影冲撞：全程可用；半血后冷却更短、触发更积极。
        const float charge_range{ this->boss_in_phase2()
                                      ? combat::enemy_charge_trigger_range * 1.25f
                                      : combat::enemy_charge_trigger_range };
        if (m_charge_cooldown <= 0.0 && dist <= charge_range && dist > 28.0f)
        {
            m_charge_windup_remaining = combat::enemy_charge_windup;
            this->emit_signal(event::character_move, godot::Vector2{}, delta_time);
            return;
        }

        if (to_player.length_squared() < 1.0f)
        {
            this->emit_signal(event::character_move, godot::Vector2{}, delta_time);
            return;
        }

        const godot::Vector2 radial{ to_player.normalized() };
        const godot::Vector2 tangent{ godot::Vector2(-radial.y, radial.x) *
                                      static_cast<float>(m_orbit_sign) };
        const float prefer{ combat::enemy_boss_orbit_range };
        const float band{ combat::enemy_boss_orbit_band };

        godot::Vector2 move{};
        if (dist < prefer - band)
            move = -radial * 0.85f + tangent * 0.55f;
        else if (dist > prefer + band)
            move = radial * 0.75f + tangent * 0.65f;
        else
            move = tangent;

        m_orbit_flip_timer -= delta_time;
        if (m_orbit_flip_timer <= 0.0)
        {
            m_orbit_sign = -m_orbit_sign;
            m_orbit_flip_timer = 2.2 + (dist > prefer ? 0.6 : 1.1);
        }

        this->emit_signal(event::character_move, move.normalized(), delta_time);
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
        else if (m_behavior == Behavior::HeartDemon)
            this->process_boss_movement(player, delta_time);
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
        if ((m_behavior == Behavior::BruteCharge || m_behavior == Behavior::HeartDemon) &&
            m_charge_active_remaining > 0.0)
            rotation_dir = m_charge_dir;
        else
            rotation_dir = player->get_global_position() - this->get_global_position();

        if (rotation_dir.is_zero_approx())
            return;

        m_rotation_angle = rotation_dir.angle() + godot::Math::deg_to_rad(90.0);
        this->emit_signal(event::character_rotate, m_rotation_angle, delta_time);
    }
}
