#include <godot_cpp/classes/camera2d.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "core/constants.hpp"
#include "entity/camera.hpp"
#include "util/engine.hpp"

namespace rl
{
    Camera::Camera()
    {
        this->set_name("PlayerCamera");
        this->set_margin_drawing_enabled(true);
        auto anchor{ AnchorMode::ANCHOR_MODE_DRAG_CENTER };
        this->set_anchor_mode(anchor);
        this->set_position_smoothing_enabled(true);
        this->set_position_smoothing_speed(camera::position_smoothing_speed);
        this->set_process(false);
    }

    void Camera::set_lookahead_enabled(const bool enabled)
    {
        m_lookahead_enabled = enabled;
        if (enabled)
            this->set_process(true);
    }

    void Camera::add_trauma(const float amount)
    {
        m_trauma = godot::Math::clamp(m_trauma + amount, 0.0f, 1.0f);
        this->set_process(true);
    }

    godot::Vector2 Camera::compute_lookahead(const double delta_time)
    {
        if (!m_lookahead_enabled)
            return m_lookahead_offset;

        godot::Node2D* parent{ godot::Object::cast_to<godot::Node2D>(this->get_parent()) };
        if (parent == nullptr)
            return m_lookahead_offset;

        const godot::Vector2 player_pos{ parent->get_global_position() };
        const godot::Vector2 mouse_pos{ parent->get_global_mouse_position() };
        godot::Vector2 desired{ mouse_pos - player_pos };

        const float max_len{ camera::lookahead_max_offset };
        if (desired.length() > max_len)
            desired = desired.normalized() * max_len;

        const float t{ godot::Math::clamp(
            static_cast<float>(camera::lookahead_lerp_speed * delta_time), 0.0f, 1.0f) };
        m_lookahead_offset = m_lookahead_offset.lerp(desired, t);
        return m_lookahead_offset;
    }

    godot::Vector2 Camera::compute_shake_offset()
    {
        if (m_trauma <= 0.0f)
            return godot::Vector2(0.0f, 0.0f);

        const float shake{ m_trauma * m_trauma };
        const float offset_x{ combat::camera_shake_max_offset * shake *
                              static_cast<float>(godot::UtilityFunctions::randf_range(-1.0, 1.0)) };
        const float offset_y{ combat::camera_shake_max_offset * shake *
                              static_cast<float>(godot::UtilityFunctions::randf_range(-1.0, 1.0)) };
        return godot::Vector2(offset_x, offset_y);
    }

    void Camera::_process(const double delta_time)
    {
        if (engine::editor_active())
            return;

        if (m_trauma > 0.0f)
        {
            m_trauma = godot::Math::max(
                m_trauma - static_cast<float>(combat::camera_shake_decay * delta_time), 0.0f);
        }

        const godot::Vector2 look{ compute_lookahead(delta_time) };
        const godot::Vector2 shake{ compute_shake_offset() };
        this->set_offset(look + shake);

        if (m_trauma <= 0.0f && !m_lookahead_enabled)
            this->set_process(false);
    }
}
