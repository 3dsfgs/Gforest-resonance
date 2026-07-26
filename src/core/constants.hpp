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
        constexpr inline auto level2{ "Level2" };
        constexpr inline auto level3{ "Level3" };
        constexpr inline auto physics_box{ "PhysicsBox" };
        constexpr inline auto spawn_point{ "SpawnPoint" };
        constexpr inline auto enemy_spawn_prefix{ "EnemySpawn" };
        constexpr inline auto enemy_brute_spawn_prefix{ "EnemyBruteSpawn" };
        constexpr inline auto enemy_striker_spawn_prefix{ "EnemyStrikerSpawn" };
        constexpr inline auto enemy_boss_spawn_prefix{ "EnemyBossSpawn" };
        constexpr inline auto enemy_spawn1{ "EnemySpawn1" };
        constexpr inline auto damage_zone{ "DamageZone" };
        constexpr inline auto death_pit{ "DeathPit" };
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
            constexpr inline auto enemy_brute{ "EnemyBrute" };
            constexpr inline auto enemy_striker{ "EnemyStriker" };
            constexpr inline auto enemy_boss{ "EnemyBoss" };
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

        /** Day4–5：三房顺序；最后一房清怪才 Victory。 */
        constexpr inline int room_count{ 3 };

        /** v0.4 P0-4：过场地名卡（情绪三幕）。 */
        constexpr inline const char* room_display_names[room_count]{
            "雾缘",
            "深心",
            "光扉",
        };
    }

    namespace camera
    {
        /** P0-2：位置阻尼（Godot Camera2D position_smoothing_speed）。 */
        constexpr inline float position_smoothing_speed{ 9.0f };
        /** P0-2：朝鼠标方向的最大前瞻偏移（像素）。 */
        constexpr inline float lookahead_max_offset{ 72.0f };
        /** P0-2：前瞻 lerp 速度。 */
        constexpr inline float lookahead_lerp_speed{ 10.0f };
    }

    namespace spawn
    {
        /** P0-3：预警圈持续时间（秒）。 */
        constexpr inline double telegraph_duration{ 0.4 };
        /** P0-3：敌人生成后淡入时长（秒）。 */
        constexpr inline double fade_in_duration{ 0.25 };
        constexpr inline float telegraph_radius{ 28.0f };
        /** P2-3：波次间喘息（秒），清波后稍停再刷下一波。 */
        constexpr inline double wave_breath_duration{ 1.15 };
    }

    namespace atmosphere
    {
        /** Day13 森林雾 / 暖绿调色。 */
        constexpr inline float fog_density{ 0.28f };
        constexpr inline float fog_drift_speed{ 0.05f };
        constexpr inline float fog_scale{ 3.2f };
        constexpr inline int fog_z_index{ 18 };
        /** 背景向森林绿插值权重（保留各房原有 tint）。 */
        constexpr inline float background_forest_blend{ 0.4f };
        constexpr inline float light_energy{ 0.72f };
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

        /**
         * Day11–12 全屏结语（GDScript ending_screen 为主展示面；
         * 此处保留同步文案供文档/兜底）。
         */
        constexpr inline auto ending_epilogue{
            "你走出了这片森林。\n"
            "困住你的从来不是黑暗，而是忘了自己能发光。\n"
            "那些弯下腰的时刻不是认输，是为了下一次起跳。\n"
            "呼吸还在。脚步还在。你已经比进来时，更像自己。" };
    }

    namespace combat
    {
        /** Invincibility duration after taking damage (P0-B02 / Day1 手感修订). */
        constexpr inline double invincibility_duration{ 1.0 };
        constexpr inline double invincibility_blink_interval{ 0.1 };
        constexpr inline float invincibility_blink_alpha{ 0.25f };

        /** Hearts removed per pulse projectile hit (P0-B03). */
        constexpr inline int projectile_damage_hearts{ 1 };
        /** Day6 快而脆（侦察机）默认血量。 */
        constexpr inline int enemy_default_hearts{ 2 };
        /** Hearts removed when an enemy touches the player (P0-B04). */
        constexpr inline int enemy_contact_damage_hearts{ 1 };
        /** Day6 快而脆追击速度。 */
        constexpr inline double enemy_movement_speed{ 420.0 };
        /** Day6 慢而肉（坦克）血量。 */
        constexpr inline int enemy_brute_hearts{ 5 };
        /** Day6 慢而肉追击速度。 */
        constexpr inline double enemy_brute_movement_speed{ 200.0 };
        /** ⭐3 影刺：低血高速贴脸。 */
        constexpr inline int enemy_striker_hearts{ 2 };
        constexpr inline double enemy_striker_movement_speed{ 520.0 };
        /** P2-4 心魔 Boss：高血量 + 中速环绕。 */
        constexpr inline int enemy_boss_hearts{ 12 };
        constexpr inline double enemy_boss_movement_speed{ 260.0 };
        constexpr inline float enemy_boss_orbit_range{ 340.0f };
        constexpr inline float enemy_boss_orbit_band{ 70.0f };
        constexpr inline float enemy_boss_ranged_range{ 580.0f };
        constexpr inline float enemy_boss_ranged_min_range{ 160.0f };
        constexpr inline double enemy_boss_fire_rate{ 0.9 };
        /** 特殊攻击「心影爆发」：扇形连射。 */
        constexpr inline double enemy_boss_special_cooldown{ 3.0 };
        constexpr inline int enemy_boss_burst_count{ 5 };
        constexpr inline double enemy_boss_burst_spread{ 0.38 };
        /** 半血以下冲撞更频繁；冲撞全程可用。 */
        constexpr inline float enemy_boss_phase2_ratio{ 0.5f };
        constexpr inline double enemy_boss_charge_cooldown{ 2.8 };
        constexpr inline double enemy_boss_charge_cooldown_phase2{ 1.5 };
        /** Day6 坦克冲撞：触发距离 / 冲刺速度倍率 / 持续时间 / 冷却。 */
        constexpr inline float enemy_charge_trigger_range{ 320.0f };
        constexpr inline float enemy_charge_speed_mult{ 3.2f };
        constexpr inline double enemy_charge_duration{ 0.45 };
        constexpr inline double enemy_charge_cooldown{ 2.2 };
        constexpr inline double enemy_charge_windup{ 0.25 };
        /** Day6 侦察机远程：射程 / 射速（发/秒）。 */
        constexpr inline float enemy_ranged_range{ 520.0f };
        constexpr inline float enemy_ranged_min_range{ 140.0f };
        constexpr inline double enemy_ranged_fire_rate{ 1.2 };
        /** Ricochet self-damage radius from bounce point (pixels). */
        constexpr inline float ricochet_self_damage_radius{ 120.0f };
        constexpr inline int ricochet_self_damage_hearts{ 1 };

        /** Day3 脉冲手感：射速 / 弹道 / 散射 / 撞墙反弹。 */
        constexpr inline double projectile_fire_rate{ 7.0 };
        /** apply_impulse 冲量大小（配合 bullet.tscn 小质量）。 */
        constexpr inline double projectile_impulse{ 1500.0 };
        constexpr inline double projectile_time_to_live{ 2.5 };
        constexpr inline double projectile_max_travel{ 1200.0 };
        /** 发射角随机半宽（弧度）；0 = 无散射。 */
        constexpr inline double projectile_spread_radians{ 0.04 };
        constexpr inline float projectile_physics_bounce{ 0.9f };
        constexpr inline float projectile_physics_friction{ 0.05f };
        /** 撞墙最多反弹次数；再撞墙则销毁。 */
        constexpr inline int projectile_max_wall_bounces{ 1 };
        /**
         * 枪口前移（世界像素）：大贴图以中心为原点时，再沿射击方向推出一段，
         * 避免子弹后半截嵌进枪管/身体。与 bullet.tscn 内 Sprite 偏移叠加。
         */
        // constexpr inline double projectile_muzzle_forward_offset{ 12.0 };

        /** Day1 打击感：命中闪色时长（玩家/敌人共用时长，颜色分开）。敌人的无敌帧可能后续需要修改 */
        constexpr inline double hit_flash_duration{ 0.12 };
        /** 敌人被打：青白闪（打击确认，不震屏）。 */
        constexpr inline float enemy_hit_flash_r{ 4.0f };
        constexpr inline float enemy_hit_flash_g{ 5.0f };
        constexpr inline float enemy_hit_flash_b{ 5.5f };
        /** 玩家被打：红橙闪（危险警告）。 */
        constexpr inline float player_hit_flash_r{ 5.0f };
        constexpr inline float player_hit_flash_g{ 1.4f };
        constexpr inline float player_hit_flash_b{ 1.1f };

        /** Day1 打击感：仅玩家受击触发屏震（trauma 累加、平方衰减）。 */
        constexpr inline float hit_shake_trauma{ 0.7f };
        constexpr inline float camera_shake_max_offset{ 22.0f };
        constexpr inline float camera_shake_decay{ 2.0f };

        /** Day2 打击感：击杀爆炸序列帧数量（simple/simpleexplosion00..08）。 */
        constexpr inline int kill_explosion_frame_count{ 9 };
        constexpr inline double kill_explosion_fps{ 18.0 };
        constexpr inline float kill_explosion_scale{ 1.35f };
        constexpr inline int kill_explosion_z_index{ 12 };

        /** Day2 音效音量（dB）。0 dB 为基准响度。 */
        constexpr inline float sfx_enemy_hit_volume_db{ 0.0f };
        constexpr inline float sfx_enemy_kill_volume_db{ 0.0f };
        constexpr inline float sfx_player_hurt_volume_db{ 0.0f };
        /** Day7–8 冲刺音效。 */
        constexpr inline float sfx_player_dash_volume_db{ -2.0f };
        /** P2-5 武器开火。 */
        constexpr inline float sfx_weapon_shot_volume_db{ -6.0f };

        /**
         * Day7–8 冲刺/闪避：距离 ≈ move_speed * dash_speed_mult * dash_duration
         * 默认约 500 * 3.8 * 0.16 ≈ 304px；冷却与短无敌可按走测微调。
         */
        constexpr inline double dash_duration{ 0.16 };
        constexpr inline double dash_speed_mult{ 3.8 };
        constexpr inline double dash_cooldown{ 0.9 };
        constexpr inline double dash_invincibility_duration{ 0.22 };

        /** ⭐1 极地斩：扇形近战 hitbox。 */
        constexpr inline int polar_slash_damage_hearts{ 1 };
        constexpr inline double polar_slash_cooldown{ 2.0 };
        constexpr inline double polar_slash_active_duration{ 0.18 };
        constexpr inline float polar_slash_range{ 96.0f };
        constexpr inline float polar_slash_half_width{ 70.0f };
        constexpr inline float polar_slash_hit_trauma{ 0.18f };

        /** ⭐1 能量之光：+1 心 + 短时加速。 */
        constexpr inline int energy_light_heal_hearts{ 1 };
        constexpr inline double energy_light_cooldown{ 6.0 };
        constexpr inline double energy_light_haste_duration{ 3.0 };
        constexpr inline double energy_light_haste_mult{ 1.35 };
        constexpr inline float energy_light_aura_radius{ 48.0f };

        /** ⭐2 战斗拾取：击杀掉落回血心 / 短时加速。 */
        constexpr inline double pickup_heal_drop_chance{ 0.28 };
        constexpr inline double pickup_haste_drop_chance{ 0.14 };
        constexpr inline int pickup_heal_hearts{ 1 };
        constexpr inline double pickup_haste_duration{ 4.0 };
        constexpr inline double pickup_haste_mult{ 1.25 };
        constexpr inline float pickup_collect_radius{ 22.0f };
        constexpr inline float pickup_bob_amplitude{ 4.0f };
        constexpr inline double pickup_bob_speed{ 3.2 };
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
        constexpr inline auto character_dash{ "character_dash" };
        constexpr inline auto character_polar_slash{ "character_polar_slash" };
        constexpr inline auto character_energy_light{ "character_energy_light" };
        constexpr inline auto level_state_changed{ "level_state_changed" };
        constexpr inline auto room_cleared{ "room_cleared" };
        constexpr inline auto run_restart{ "run_restart" };
        constexpr inline auto run_victory{ "run_victory" };
        /** P0-4：清房后请求过场，GDScript 播完再 advance_to_room。 */
        constexpr inline auto room_advance_requested{ "room_advance_requested" };
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
        PlayerMelee = 0x00000080,
        PlayerAuras = 0x00000100,
        Pickups = 0x00000200,
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
     * | Player       | Player            | Walls, NPCs                           | mask 10     |
     * | Enemy        | NPCs              | Walls, Player                          | mask 9      |
     * | Projectile   | Projectiles       | Walls, NPCs                            | mask 10     |
     * | Projectile*  | Projectiles       | + PhysicsObjects (B03 ricochet debug)  | mask 74     |
     * | Walls        | Walls             | —                                      | mask 0      |
     * | DamageZones  | DamageZones       | Player (Area2D trap)                   | mask 1      |
     * | DeathZones   | DeathZones        | Player (Area2D trap)                   | mask 1      |
     *
     * Day4–5: Damage/Death are Area2D traps (passable). Player no longer solid-collides them.
     * *After wall/box ricochet, Projectile runtime-adds Player to mask for self-damage.
     */
    namespace collision
    {
        constexpr inline uint32_t player_layer{ static_cast<uint32_t>(LayerID::Player) };
        constexpr inline uint32_t enemy_layer{ static_cast<uint32_t>(LayerID::NPCs) };
        constexpr inline uint32_t projectile_layer{ static_cast<uint32_t>(LayerID::Projectiles) };
        constexpr inline uint32_t walls_layer{ static_cast<uint32_t>(LayerID::Walls) };

        /** Solid collisions only — traps are Area2D, detected separately. */
        constexpr inline uint32_t player_mask{ static_cast<uint32_t>(LayerID::Walls) |
                                               static_cast<uint32_t>(LayerID::NPCs) };
        constexpr inline uint32_t enemy_mask{ static_cast<uint32_t>(LayerID::Walls) |
                                              static_cast<uint32_t>(LayerID::Player) };
        constexpr inline uint32_t projectile_mask{ static_cast<uint32_t>(LayerID::Walls) |
                                                   static_cast<uint32_t>(LayerID::NPCs) };
        /** B03 ricochet off DebugZones/PhysicsBox — used in bullet.tscn. */
        constexpr inline uint32_t projectile_mask_ricochet{
            projectile_mask | static_cast<uint32_t>(LayerID::PhysicsObjects) };
        /** Area2D trap mask: detect Player body. */
        constexpr inline uint32_t trap_mask{ static_cast<uint32_t>(LayerID::Player) };

        constexpr inline uint32_t player_melee_layer{ static_cast<uint32_t>(LayerID::PlayerMelee) };
        constexpr inline uint32_t player_melee_mask{ static_cast<uint32_t>(LayerID::NPCs) };
        constexpr inline uint32_t player_auras_layer{ static_cast<uint32_t>(LayerID::PlayerAuras) };
        constexpr inline uint32_t pickups_layer{ static_cast<uint32_t>(LayerID::Pickups) };
        constexpr inline uint32_t pickups_mask{ static_cast<uint32_t>(LayerID::Player) };
    }

    namespace path

    {
        namespace scene
        {
            /** Day4–5 三房序列。 */
            constexpr inline auto Level1{ "res://scenes/levels/level1.tscn" };
            constexpr inline auto Level2{ "res://scenes/levels/level2.tscn" };
            constexpr inline auto Level3{ "res://scenes/levels/level3.tscn" };
            constexpr inline auto Player{ "res://scenes/characters/player.tscn" };
            constexpr inline auto Bullet{ "res://scenes/projectiles/bullet.tscn" };
            constexpr inline auto Enemy{ "res://scenes/characters/enemy.tscn" };
            /** Day6：慢而肉变体。 */
            constexpr inline auto EnemyBrute{ "res://scenes/characters/enemy_brute.tscn" };
            /** ⭐3：影刺近战贴脸。 */
            constexpr inline auto EnemyStriker{ "res://scenes/characters/enemy_striker.tscn" };
            /** P2-4：心魔 Boss。 */
            constexpr inline auto EnemyBoss{ "res://scenes/characters/enemy_boss.tscn" };

            constexpr inline const char* room_paths[level::room_count]{
                Level1,
                Level2,
                Level3,
            };
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
            constexpr inline auto BirthdayGate{ "res://scenes/ui/birthday_gate.tscn" };
            constexpr inline auto TitleScreen{ "res://scenes/ui/title_screen.tscn" };
        }

        namespace data
        {
            constexpr inline auto buff_rules{ "res://data/buff_rules.json" };
            constexpr inline auto weapon_rules{ "res://data/weapon_rules.json" };
        }

        namespace shader
        {
            /** Day13 房内漂移雾。 */
            constexpr inline auto forest_fog{ "res://shaders/forest_fog.gdshader" };
        }

        namespace art
        {
            /** Day10–11 标题全出血背景；缺失时用纯色占位。 */
            constexpr inline auto title_bg{ "res://assets/art/title/title_bg.png" };
        }

        namespace audio
        {
            /** Kenney retro pack — pulse hit confirm. */
            constexpr inline auto enemy_hit{ "res://assets/audio/sfx/retro/hit2.ogg" };
            /** Kenney retro pack — enemy defeat. */
            constexpr inline auto enemy_kill{ "res://assets/audio/sfx/retro/explosion2.ogg" };
            /** Kenney retro pack — player hurt. */
            constexpr inline auto player_hurt{ "res://assets/audio/sfx/retro/hurt2.ogg" };
            /** Kenney retro pack — dash whoosh 占位（可换成专用冲刺音）。 */
            constexpr inline auto player_dash{ "res://assets/audio/sfx/retro/laser1.ogg" };
            /** P2-5 武器开火占位（与 weapon_rules.json 对齐）。 */
            constexpr inline auto weapon_pulse{ "res://assets/audio/sfx/sci_fi/lasersmall_000.ogg" };
            constexpr inline auto weapon_pistol{ "res://assets/audio/sfx/sci_fi/laserretro_001.ogg" };
            constexpr inline auto weapon_shotgun{ "res://assets/audio/sfx/sci_fi/laserlarge_000.ogg" };
        }

        namespace music
        {
            /** Day11–12 分场景 BGM（Kenney loops 占位，可整段替换）。 */
            constexpr inline auto title{ "res://assets/audio/music/loops/flowing_rocks.ogg" };
            constexpr inline auto explore{ "res://assets/audio/music/loops/space_cadet.ogg" };
            constexpr inline auto combat{ "res://assets/audio/music/loops/mission_plausible.ogg" };
            constexpr inline auto boss{ "res://assets/audio/music/loops/infinite_descent.ogg" };
            constexpr inline auto ending{ "res://assets/audio/music/loops/night_at-the-beach.ogg" };
        }

        namespace vfx
        {
            /** Simple explosion sheet frames (00–08). */
            constexpr inline auto kill_explosion_dir{
                "res://assets/art/explosions/simple/simpleexplosion" };
        }
    }
}
