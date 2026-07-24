#include <cstdio>

#include <godot_cpp/classes/animated_sprite2d.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/sprite_frames.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/string.hpp>

#include "core/constants.hpp"
#include "util/combat_feedback.hpp"

namespace rl::combat_feedback
{
    namespace
    {
        constexpr const char* kill_anim_name{ "kill" };

        godot::Ref<godot::AudioStream> load_audio(const godot::String& resource_path)
        {
            godot::ResourceLoader* loader{ godot::ResourceLoader::get_singleton() };
            if (loader == nullptr || resource_path.is_empty() || !loader->exists(resource_path))
                return {};

            return loader->load(resource_path);
        }

        godot::Ref<godot::Texture2D> load_texture(const godot::String& resource_path)
        {
            godot::ResourceLoader* loader{ godot::ResourceLoader::get_singleton() };
            if (loader == nullptr || !loader->exists(resource_path))
                return {};

            return loader->load(resource_path);
        }

        void play_one_shot(godot::Node* scene_parent, godot::Vector2 world_position,
                           const godot::Ref<godot::AudioStream>& stream, float volume_db)
        {
            if (scene_parent == nullptr || !stream.is_valid())
                return;

            (void)world_position;

            // Feedback SFX: non-positional player (avoids SubViewport listener issues).
            godot::AudioStreamPlayer* player{ memnew(godot::AudioStreamPlayer) };
            scene_parent->add_child(player);
            player->set_stream(stream);
            player->set_volume_db(volume_db);
            player->set_bus("Master");
            player->play();
            player->connect("finished", godot::Callable(player, "queue_free"));
        }

        godot::Ref<godot::SpriteFrames> build_kill_sprite_frames()
        {
            // Avoid static caches to reduce potential shutdown-order issues.
            godot::Ref<godot::SpriteFrames> frames{ memnew(godot::SpriteFrames) };
            frames->add_animation(kill_anim_name);
            frames->set_animation_loop(kill_anim_name, false);
            frames->set_animation_speed(kill_anim_name, combat::kill_explosion_fps);

            const float frame_duration{
                static_cast<float>(1.0 / combat::kill_explosion_fps) };

            for (int frame = 0; frame < combat::kill_explosion_frame_count; ++frame)
            {
                char path_buffer[192]{};
                std::snprintf(path_buffer, sizeof(path_buffer), "%s%02d.png",
                              path::vfx::kill_explosion_dir, frame);

                godot::Ref<godot::Texture2D> texture{ load_texture(path_buffer) };
                if (!texture.is_valid())
                    continue;

                frames->add_frame(kill_anim_name, texture, frame_duration);
            }

            return frames;
        }
    }

    void play_enemy_hit(godot::Node* scene_parent, godot::Vector2 world_position)
    {
        godot::Ref<godot::AudioStream> stream{ load_audio(path::audio::enemy_hit) };
        play_one_shot(scene_parent, world_position, stream, combat::sfx_enemy_hit_volume_db);
    }

    void play_enemy_kill(godot::Node* scene_parent, godot::Vector2 world_position)
    {
        godot::Ref<godot::AudioStream> stream{ load_audio(path::audio::enemy_kill) };
        play_one_shot(scene_parent, world_position, stream, combat::sfx_enemy_kill_volume_db);
    }

    void play_player_hurt(godot::Node* scene_parent, godot::Vector2 world_position)
    {
        godot::Ref<godot::AudioStream> stream{ load_audio(path::audio::player_hurt) };
        play_one_shot(scene_parent, world_position, stream, combat::sfx_player_hurt_volume_db);
    }

    void play_player_dash(godot::Node* scene_parent, godot::Vector2 world_position)
    {
        godot::Ref<godot::AudioStream> stream{ load_audio(path::audio::player_dash) };
        play_one_shot(scene_parent, world_position, stream, combat::sfx_player_dash_volume_db);
    }

    void play_weapon_shot(godot::Node* scene_parent, godot::Vector2 world_position,
                          const godot::String& sfx_path)
    {
        if (sfx_path.is_empty())
            return;
        godot::Ref<godot::AudioStream> stream{ load_audio(sfx_path) };
        play_one_shot(scene_parent, world_position, stream, combat::sfx_weapon_shot_volume_db);
    }

    void spawn_kill_explosion(godot::Node* scene_parent, godot::Vector2 world_position)
    {
        if (scene_parent == nullptr)
            return;

        godot::Ref<godot::SpriteFrames> frames{ build_kill_sprite_frames() };
        if (!frames.is_valid() || frames->get_frame_count(kill_anim_name) == 0)
            return;

        godot::AnimatedSprite2D* burst{ memnew(godot::AnimatedSprite2D) };
        scene_parent->add_child(burst);
        burst->set_global_position(world_position);
        burst->set_sprite_frames(frames);
        burst->set_animation(kill_anim_name);
        burst->set_scale(
            godot::Vector2(combat::kill_explosion_scale, combat::kill_explosion_scale));
        burst->set_z_index(combat::kill_explosion_z_index);
        burst->play(kill_anim_name);
        burst->connect("animation_finished", godot::Callable(burst, "queue_free"));
    }
}
