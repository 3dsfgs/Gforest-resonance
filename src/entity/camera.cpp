#include <godot_cpp/classes/camera2d.hpp>
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
        this->set_process(false);
    }

    void Camera::add_trauma(float amount)
    {
        m_trauma = godot::Math::clamp(m_trauma + amount, 0.0f, 1.0f);
        this->set_process(true);
    }

    void Camera::_process(double delta_time)
    {
        if (engine::editor_active())
            return;

        if (m_trauma <= 0.0f)
        {
            this->set_offset(godot::Vector2(0.0f, 0.0f));
            this->set_process(false);
            return;
        }

        m_trauma = godot::Math::max(
            m_trauma - static_cast<float>(combat::camera_shake_decay * delta_time), 0.0f);

        const float shake{ m_trauma * m_trauma };
        const float offset_x{ combat::camera_shake_max_offset * shake *
                              static_cast<float>(godot::UtilityFunctions::randf_range(-1.0, 1.0)) };
        const float offset_y{ combat::camera_shake_max_offset * shake *
                              static_cast<float>(godot::UtilityFunctions::randf_range(-1.0, 1.0)) };
        this->set_offset(godot::Vector2(offset_x, offset_y));
    }
}
