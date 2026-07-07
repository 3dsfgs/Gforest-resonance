#pragma once

#include <cstdint>
#include <string>

namespace rl::inline constants
{
    namespace name
    {
        namespace level
        {
            constexpr inline auto level1{ "Level1" };
            constexpr inline auto physics_box{ "PhysicsBox" };
            constexpr inline auto spawn_point{ "SpawnPoint" };
            constexpr inline auto enemy_spawn_prefix{ "EnemySpawn" };
            constexpr inline auto enemy_spawn1{ "EnemySpawn1" };
            constexpr inline auto boundaries{ "Boundaries" };
            constexpr inline auto ground{ "Ground" };
            constexpr inline auto debug_zones{ "DebugZones" };
            constexpr inline auto ground_sprite{ "GroundSprite" };
            constexpr inline auto near_overlay{ "NearOverlay" };
            constexpr inline auto fog_overlay{ "FogOverlay" };
            constexpr inline auto room_decor{ "RoomDecor" };
            constexpr inline auto player_camera{ "PlayerCamera" };
        }

        namespace dialog
        {
            constexpr inline auto console{ "ConsolePanel" };
            constexpr inline auto canvas_layer{ "MainCanvasLayer" };
            constexpr inline auto game_sub_viewport{ "MainSubViewport" };
        }

        namespace character
        {
            constexpr inline auto player{ "Player" };
            constexpr inline auto enemy{ "Enemy" };
            constexpr inline auto firing_pt{ "FiringPoint" };
            constexpr inline auto sprite{ "PlayerSprite" };
        }

        namespace ui
        {
            constexpr inline auto heart_hud{ "HeartHud" };
        }

    }

    namespace level
    {
        /**
         * Room Template v0 — first forest clearing (元气骑士式单房，见 doc/P0-L04-森林房间模板).
         * L01 的 level1.tscn 即本房间；P1 起扩展为多房连通。
         */
        constexpr inline float playable_width{ 2000.0f };
        constexpr inline float playable_height{ 1500.0f };
        constexpr inline float wall_thickness{ 40.0f };
        constexpr inline float half_playable_width{ playable_width * 0.5f };
        constexpr inline float half_playable_height{ playable_height * 0.5f };

        constexpr inline float ground_texture_size{ 1254.0f };
        constexpr inline float fog_texture_size{ 1024.0f };
        constexpr inline float near_texture_size{ 1024.0f };
        constexpr inline float ground_sprite_scale_x{ playable_width / ground_texture_size };
        constexpr inline float ground_sprite_scale_y{ playable_height / ground_texture_size };
        constexpr inline float fog_overlay_scale_x{ playable_width / fog_texture_size };
        constexpr inline float fog_overlay_scale_y{ playable_height / fog_texture_size };
        constexpr inline float near_overlay_scale_x{ playable_width / near_texture_size };
        constexpr inline float near_overlay_scale_y{ playable_height / near_texture_size };
        constexpr inline float fog_overlay_alpha{ 0.38f };
        constexpr inline float near_overlay_alpha{ 0.72f };
        constexpr inline float door_opening_width{ 220.0f };
        constexpr inline float wall_visual_depth{ 72.0f };
    }

    namespace narrative
    {
        /** Seconds between revealed characters (P0-L03). */
        constexpr inline double char_reveal_interval{ 0.055 };
        /** Pause after each line before the next (P0-L03). */
        constexpr inline double line_pause_interval{ 1.4 };

        constexpr inline auto victory_text{
            "林间恢复了片刻的宁静。\n"
            "你停下脚步，听见自己的呼吸，轻轻落下。\n"
            "那些扰人的影子散去了——不是胜利，只是又撑过了一程。" };

        constexpr inline auto defeat_text{
            "身体先倒下，心却还醒着。\n"
            "黑暗并不意味着终点，只是提醒你：该慢一点了。\n"
            "森林不会责备跌倒的人，它只等你再次起身。" };

        constexpr inline auto victory_hint{ "按 R 可重新开始本关" };
        constexpr inline auto defeat_hint{ "按 R 重新开始" };
    }

    namespace combat
    {
        /** Invincibility duration after taking damage (P0-B02). */
        constexpr inline double invincibility_duration{ 0.75 };
        constexpr inline double invincibility_blink_interval{ 0.08 };
        constexpr inline float invincibility_blink_alpha{ 0.35f };

        /** Hearts removed per pulse projectile hit (P0-B03). */
        constexpr inline int projectile_damage_hearts{ 1 };
        /** Default enemy heart count (P0-B04). */
        constexpr inline int enemy_default_hearts{ 3 };
        /** Hearts removed when an enemy touches the player (P0-B04). */
        constexpr inline int enemy_contact_damage_hearts{ 1 };
        /** Enemy chase speed — slower than default player speed (P0-B04). */
        constexpr inline double enemy_movement_speed{ 350.0 };
        /** Ricochet self-damage radius from bounce point (pixels). */
        constexpr inline float ricochet_self_damage_radius{ 120.0f };
        constexpr inline int ricochet_self_damage_hearts{ 1 };
    }

    namespace event
    {
        constexpr inline auto hearts_changed{ "hearts_changed" };
        constexpr inline auto died{ "died" };
        constexpr inline auto position_changed{ "position_changed" };
        constexpr inline auto entered_area{ "entered_area" };
        constexpr inline auto exited_area{ "exited_area" };
        constexpr inline auto spawn_projectile{ "spawn_projectile" };
        constexpr inline auto character_move{ "character_move" };
        constexpr inline auto character_rotate{ "character_rotate" };
        constexpr inline auto character_shoot{ "character_shoot" };
        constexpr inline auto level_state_changed{ "level_state_changed" };
        constexpr inline auto body_entered{ "body_entered" };
        constexpr inline auto body_exited{ "body_exited" };
        constexpr inline auto signal_example{ "custom_signal_example" };
    }

    enum class LayerID : uint32_t {
        Player = 0x00000001,
        NPCs = 0x00000002,
        Projectiles = 0x00000004,
        Walls = 0x00000008,
        DamageZones = 0x00000010,
        DeathZones = 0x00000020,
        PhysicsObjects = 0x00000040,
        Layer08 = 0x00000080,
        Layer09 = 0x00000100,
        Layer10 = 0x00000200,
        Layer11 = 0x00000400,
        Layer12 = 0x00000800,
        Layer13 = 0x00001000,
        Layer14 = 0x00002000,
        Layer15 = 0x00004000,
        Layer16 = 0x00008000,
    };

    /**
     * Physics collision matrix (P0-B06).
     * Layer names are registered in project.godot [layer_names].
     *
     * | Entity       | layer (LayerID)   | mask includes                          | scene value |
     * |--------------|-------------------|----------------------------------------|-------------|
     * | Player       | Player            | Walls, DamageZones, DeathZones, NPCs   | mask 58     |
     * | Enemy        | NPCs              | Walls, Player                          | mask 9      |
     * | Projectile   | Projectiles       | Walls, NPCs                            | mask 10     |
     * | Projectile*  | Projectiles       | + PhysicsObjects (B03 ricochet debug)  | mask 74     |
     * | Walls        | Walls             | —                                      | mask 0      |
     * | DamageZones  | DamageZones       | —                                      | mask 0      |
     * | DeathZones   | DeathZones        | —                                      | mask 0      |
     *
     * *After wall/box ricochet, Projectile runtime-adds Player to mask for self-damage.
     */
    namespace collision
    {
        constexpr inline uint32_t player_layer{ static_cast<uint32_t>(LayerID::Player) };
        constexpr inline uint32_t enemy_layer{ static_cast<uint32_t>(LayerID::NPCs) };
        constexpr inline uint32_t projectile_layer{ static_cast<uint32_t>(LayerID::Projectiles) };
        constexpr inline uint32_t walls_layer{ static_cast<uint32_t>(LayerID::Walls) };

        constexpr inline uint32_t player_mask{
            static_cast<uint32_t>(LayerID::Walls) | static_cast<uint32_t>(LayerID::DamageZones) |
            static_cast<uint32_t>(LayerID::DeathZones) | static_cast<uint32_t>(LayerID::NPCs) };
        constexpr inline uint32_t enemy_mask{ static_cast<uint32_t>(LayerID::Walls) |
                                              static_cast<uint32_t>(LayerID::Player) };
        constexpr inline uint32_t projectile_mask{ static_cast<uint32_t>(LayerID::Walls) |
                                                   static_cast<uint32_t>(LayerID::NPCs) };
        /** B03 ricochet off DebugZones/PhysicsBox — used in bullet.tscn. */
        constexpr inline uint32_t projectile_mask_ricochet{
            projectile_mask | static_cast<uint32_t>(LayerID::PhysicsObjects) };
    }

    namespace path

    {
        namespace scene
        {
            /** Room Template v0 — forest clearing; runtime root 仍为 Level1。 */
            constexpr inline auto Level1{ "res://scenes/levels/level1.tscn" };
            constexpr inline auto Player{ "res://scenes/characters/player.tscn" };
            constexpr inline auto Bullet{ "res://scenes/projectiles/bullet.tscn" };
            constexpr inline auto Enemy{ "res://scenes/characters/enemy.tscn" };
        }

        namespace room_assets
        {
            constexpr inline auto ground{ "res://assets/parallax/parallax_far.png" };
            constexpr inline auto fog_overlay{ "res://assets/parallax/parallax_mid.png" };
            constexpr inline auto near_overlay{ "res://assets/parallax/parallax_near.png" };
        }

        namespace ui
        {
            constexpr inline auto MainDialog{ "res://scenes/ui/main_dialog.tscn" };
        }
    }
}
