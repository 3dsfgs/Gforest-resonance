#pragma once

#include <godot_cpp/classes/camera2d.hpp>

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

    protected:
        static void _bind_methods()
        {
        }

    private:
        float m_trauma{ 0.0f };
    };
}
