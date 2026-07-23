#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace rl::combat_feedback
{
    /** One-shot SFX at world position (auto-freed). */
    void play_enemy_hit(godot::Node* scene_parent, godot::Vector2 world_position);
    void play_enemy_kill(godot::Node* scene_parent, godot::Vector2 world_position);
    void play_player_hurt(godot::Node* scene_parent, godot::Vector2 world_position);
    void play_player_dash(godot::Node* scene_parent, godot::Vector2 world_position);

    /** AnimatedSprite2D kill burst at world position (auto-freed). */
    void spawn_kill_explosion(godot::Node* scene_parent, godot::Vector2 world_position);
}
