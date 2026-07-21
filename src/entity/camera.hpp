#pragma once

#include <godot_cpp/classes/camera2d.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace rl
{
    class Camera : public godot::Camera2D
    {
        GDCLASS(Camera, godot::Camera2D);

    public:
        Camera();
        ~Camera() = default;

        void _process(double delta_time) override;

        /** Add screen-shake "trauma" (0..1). Shake magnitude scales with trauma^2. */
        void add_trauma(float amount);

        /** P0-2：玩家镜头朝鼠标方向轻微偏移。 */
        void set_lookahead_enabled(bool enabled);

    protected:
        static void _bind_methods()
        {
        }

    private:
        godot::Vector2 compute_lookahead(double delta_time);
        godot::Vector2 compute_shake_offset();

        float m_trauma{ 0.0f };
        bool m_lookahead_enabled{ false };
        godot::Vector2 m_lookahead_offset{};
    };
}
