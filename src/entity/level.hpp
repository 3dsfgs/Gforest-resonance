#pragma once

#include <atomic>
#include <vector>

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "core/constants.hpp"
#include "entity/character/enemy.hpp"
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

    /** P2-6：梦房链房型。 */
    enum class RoomKind {
        Combat,
        Whisper,
        Mood,
        Boss
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
        void set_room_kind(RoomKind kind);
        void set_is_final_room(bool is_final);
        /** P2-6：低语/心境房由 GDScript 调用以推进链条。 */
        void request_room_clear();
        /** P2-1：开战前写入；_ready 内按此应用。 */
        void set_weapon_id(const godot::String& weapon_id);
        /** P2-1：切换当前武器（数据来自 weapon_rules.json）。 */
        void apply_player_weapon(const godot::String& weapon_id);
        [[nodiscard]] godot::String get_player_weapon() const;
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
        enum class EnemySpawnKind {
            Scout,
            Brute,
            Boss
        };

        struct PendingEnemySpawn {
            godot::Vector2 position{};
            EnemySpawnKind kind{ EnemySpawnKind::Scout };
            double delay_remaining{ spawn::telegraph_duration };
        };

        struct FadingEnemy {
            Enemy* enemy{ nullptr };
            double fade_remaining{ spawn::fade_in_duration };
        };

        /** P2-3：一波内的刷怪点（来自 Marker）。 */
        struct WaveSpawnPoint {
            godot::Vector2 position{};
            EnemySpawnKind kind{ EnemySpawnKind::Scout };
        };

        void spawn_player_at_marker();
        void spawn_enemies_from_markers();
        void collect_wave_markers();
        void start_wave(int wave_index);
        void try_finish_current_wave();
        void queue_enemy_spawn(godot::Vector2 position, EnemySpawnKind kind);
        void finalize_enemy_spawn(const PendingEnemySpawn& pending);
        void update_pending_spawns(double delta_time);
        void update_fading_enemies(double delta_time);
        void update_wave_breath(double delta_time);
        void spawn_projectile_from_marker(godot::Node* obj, bool from_enemy);
        void clear_enemies();
        void clear_projectiles();
        void set_player_input_enabled(bool enabled);
        void transition_to_state(LevelState new_state);
        void reset_level();
        void handle_restart_input();
        void apply_room_camera_limits();
        void apply_forest_atmosphere();
        void complete_room();
        [[nodiscard]] static int parse_wave_index(const godot::String& marker_name);

        std::atomic<bool> m_active{ false };
        LevelState m_state{ LevelState::Playing };
        int m_enemy_count{ 0 };
        int m_room_index{ 0 };
        RoomKind m_room_kind{ RoomKind::Combat };
        bool m_is_final_room{ false };
        int m_current_wave{ 0 };
        double m_wave_breath_remaining{ -1.0 };
        godot::String m_weapon_id{ "pulse" };
        double m_fire_rate_mult{ 1.0 };
        godot::Node* m_background{ nullptr };
        ProjectileSpawner* m_projectile_spawner{ memnew(rl::ProjectileSpawner) };
        Player* m_player{ nullptr };
        godot::RigidBody2D* m_physics_box{ nullptr };
        std::vector<PendingEnemySpawn> m_pending_spawns{};
        std::vector<FadingEnemy> m_fading_enemies{};
        std::vector<std::vector<WaveSpawnPoint>> m_waves{};
    };
}
