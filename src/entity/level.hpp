#pragma once

#include <atomic>
#include <vector>

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/sprite2d.hpp>

#include "core/constants.hpp"
#include "entity/character/player.hpp"
#include "entity/controller/player_controller.hpp"
#include "entity/projectile/projectile_spawner.hpp"
#include "util/bind.hpp"
#include "util/scene.hpp"

namespace godot
{
    class RigidBody2D;
}

namespace rl
{
    class Player;

    enum class LevelState {
        Playing,
        Victory,
        Defeat
    };

    class Level : public godot::Node2D
    {
        GDCLASS(Level, godot::Node2D);

    public:
        Level();
        ~Level() = default;

        virtual void _ready() override;
        void _draw() override;
        void _process(double delta_time) override;

        void activate(bool active = true);
        bool active() const;

        [[nodiscard]] LevelState get_state() const;
        void set_room_index(int room_index);
        [[nodiscard]] int get_room_index() const;
        [[nodiscard]] int get_player_hearts() const;
        void apply_player_hearts(int hearts);

    protected:
        static void _bind_methods();

        [[signal_slot]] void on_player_spawn_projectile(godot::Node* obj);
        [[signal_slot]] void on_enemy_spawn_projectile(godot::Node* obj);
        [[signal_slot]] void on_character_position_changed(const godot::Object* const obj,
                                                           godot::Vector2 location) const;
        [[signal_slot]] void on_player_died();
        [[signal_slot]] void on_enemy_died();

    private:
        void spawn_player_at_marker();
        void spawn_enemies_from_markers();
        void spawn_projectile_from_marker(godot::Node* obj, bool from_enemy);
        void clear_enemies();
        void clear_projectiles();
        void set_player_input_enabled(bool enabled);
        void transition_to_state(LevelState new_state);
        void reset_level();
        void handle_restart_input();
        void apply_room_camera_limits();

        std::atomic<bool> m_active{ false };
        LevelState m_state{ LevelState::Playing };
        int m_enemy_count{ 0 };
        int m_room_index{ 0 };
        godot::Node* m_background{ nullptr };
        ProjectileSpawner* m_projectile_spawner{ memnew(rl::ProjectileSpawner) };
        Player* m_player{ nullptr };
        godot::RigidBody2D* m_physics_box{ nullptr };
    };
}
