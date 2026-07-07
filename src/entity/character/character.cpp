#include <array>
#include <tuple>
#include <type_traits>
#include <vector>
#include <algorithm>

#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/marker2d.hpp>
#include <godot_cpp/classes/rectangle_shape2d.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "core/assert.hpp"
#include "core/concepts.hpp"
#include "core/constants.hpp"
#include "entity/camera.hpp"
#include "entity/character/character.hpp"
#include "entity/controller/character_controller.hpp"
#include "util/bind.hpp"
#include "util/engine.hpp"
#include "util/input.hpp"
#include "util/io.hpp"
#include "util/scene.hpp"

namespace rl
{
    Character::Character()
    {
        this->set_motion_mode(MotionMode::MOTION_MODE_FLOATING);
    }

    void Character::_ready()
    {
        this->add_child(m_camera);
        //m_camera->make_current();
        this->set_process(false);

        m_firing_point = gdcast<godot::Marker2D>(
            this->find_child(name::character::firing_pt, true, false));

        m_sprite = gdcast<godot::Sprite2D>(
            this->find_child(name::character::sprite, true, false));

        runtime_assert(m_firing_point != nullptr);

        if (m_character_controller != nullptr)
        {
            this->add_child(m_character_controller);

            signal<event::character_move>::connect<CharacterController>(m_character_controller) <=>
                signal_callback(this, on_character_movement);

            signal<event::character_rotate>::connect<CharacterController>(m_character_controller) <=>
                signal_callback(this, on_character_rotate);

            signal<event::character_shoot>::connect<CharacterController>(m_character_controller) <=>
                signal_callback(this, on_character_shoot);
        }
    }

    void Character::set_controller(CharacterController* controller)
    {
        m_character_controller = controller;
        runtime_assert(m_character_controller != nullptr);
    }

    CharacterController* Character::get_controller() const
    {
        return m_character_controller;
    }

    [[signal_slot]]
    void Character::on_character_movement(godot::Vector2 movement_velocity, double delta_time)
    {
        double increment = m_movement_friction * delta_time;
        godot::Vector2 velocity{ this->get_velocity().lerp(movement_velocity, increment) };
        velocity = velocity.clamp({ -1.0, -1.0 }, { 1.0, 1.0 });
        this->translate(velocity * this->get_movement_speed() * delta_time);
        this->set_velocity(velocity);
        this->move_and_slide();
        this->process_slide_collisions();
    }

    [[signal_slot]]
    void Character::on_character_rotate(double rotation_angle, double delta_time)
    {
        const double smoothed_angle = godot::Math::lerp_angle(
            static_cast<double>(this->get_rotation()), rotation_angle, m_rotation_speed * delta_time);
        this->set_rotation(smoothed_angle);
    }

    [[signal_slot]]
    void Character::on_character_shoot()
    {
        // TODO: fix this
        this->emit_signal(event::spawn_projectile, m_firing_point);
    }

    [[property]]
    double Character::get_movement_speed() const
    {
        return m_movement_speed;
    }

    [[property]]
    void Character::set_movement_speed(const double move_speed)
    {
        m_movement_speed = move_speed;
    }

    [[property]]
    double Character::get_movement_friction() const
    {
        return m_movement_friction;
    }

    [[property]]
    void Character::set_movement_friction(const double move_friction)
    {
        m_movement_friction = move_friction;
    }

    [[property]]
    double Character::get_rotation_speed() const
    {
        return m_rotation_speed;
    }

    [[property]]
    void Character::set_rotation_speed(const double rotation_speed)
    {
        m_rotation_speed = rotation_speed;
    }

    bool Character::is_alive() const
    {
        return m_health.is_alive();
    }

    bool Character::is_invincible() const
    {
        return m_invincibility_remaining > 0.0;
    }

    bool Character::take_damage(const int hearts, const bool bypass_invincibility)
    {
        if (!m_health.is_alive())
            return false;

        if (!bypass_invincibility && is_invincible())
            return false;

        const int lost = m_health.apply_damage(hearts);
        if (lost <= 0)
            return false;

        emit_hearts_changed();

        if (m_health.is_alive() && !bypass_invincibility)
            start_invincibility();

        if (!m_health.is_alive())
        {
            end_invincibility();
            this->emit_signal(event::died);
        }

        return true;
    }

    void Character::reset_hearts()
    {
        end_invincibility();
        m_health.reset();
        emit_hearts_changed();
    }

    void Character::emit_hearts_changed()
    {
        this->emit_signal(event::hearts_changed, m_health.hearts, m_health.max_hearts);
    }

    void Character::process_slide_collisions()
    {
    }

    void Character::_process(const double delta_time)
    {
        if (!is_invincible())
        {
            end_invincibility();
            return;
        }

        m_invincibility_remaining -= delta_time;

        m_blink_timer += delta_time;
        if (m_blink_timer >= combat::invincibility_blink_interval)
        {
            m_blink_timer -= combat::invincibility_blink_interval;
            m_blink_visible = !m_blink_visible;
            update_invincibility_visual();
        }

        if (!is_invincible())
            end_invincibility();
    }

    void Character::start_invincibility()
    {
        m_invincibility_remaining = combat::invincibility_duration;
        m_blink_timer = 0.0;
        m_blink_visible = true;
        this->set_process(true);
        update_invincibility_visual();
    }

    void Character::end_invincibility()
    {
        m_invincibility_remaining = 0.0;
        m_blink_timer = 0.0;
        m_blink_visible = true;
        this->set_process(false);
        update_invincibility_visual();
    }

    void Character::update_invincibility_visual()
    {
        if (m_sprite == nullptr)
            return;

        const float alpha = is_invincible() && !m_blink_visible ? combat::invincibility_blink_alpha
                                                                : 1.0f;
        m_sprite->set_modulate(godot::Color(1.0f, 1.0f, 1.0f, alpha));
    }

    [[property]]
    int Character::get_hearts() const
    {
        return m_health.hearts;
    }

    [[property]]
    int Character::get_max_hearts() const
    {
        return m_health.max_hearts;
    }

    [[property]]
    void Character::set_max_hearts(const int max_hearts)
    {
        m_health.set_max(max_hearts);
        emit_hearts_changed();
    }

    [[property]]
    void Character::set_hearts(const int hearts)
    {
        m_health.hearts = std::clamp(hearts, 0, m_health.max_hearts);
        emit_hearts_changed();
    }
}
