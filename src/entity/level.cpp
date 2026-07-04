#include <godot_cpp/classes/collision_polygon2d.hpp>
#include <godot_cpp/classes/marker2d.hpp>
#include <godot_cpp/classes/rigid_body2d.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "entity/character/character.hpp"
#include "entity/character/enemy.hpp"
#include "entity/controller/player_controller.hpp"
#include "entity/level.hpp"
#include "singletons/console.hpp"
#include "util/bind.hpp"
#include "util/conversions.hpp"
#include "util/debug.hpp"
#include "util/engine.hpp"
#include "util/input.hpp"
#include "util/io.hpp"

namespace rl
{
    Level::Level()
    {
        scene::node::set_unique_name(this, name::level::level1);
        this->activate(true);
    }

    void Level::_ready()
    {
        godot::Node* box{ this->find_child(name::level::physics_box, true, false) };
        m_physics_box = gdcast<godot::RigidBody2D>(box);

        resource::preload::packed_scene<Player> player_scene{ path::scene::Player };
        m_player = player_scene.instantiate();
        m_player->set_controller(memnew(PlayerController));

        this->add_child(m_player);
        this->spawn_player_at_marker();
        this->add_child(m_projectile_spawner);
        this->spawn_enemies_from_markers();

        PlayerController* controller{ gdcast<PlayerController>(m_player->get_controller()) };
        if (controller != nullptr)
        {
            signal<event::position_changed>::connect<CharacterController>(controller) <=>
                signal_callback(this, on_character_position_changed);

            signal<event::spawn_projectile>::connect<Player>(m_player) <=>
                signal_callback(this, on_player_spawn_projectile);
        }
    }

    void Level::spawn_player_at_marker()
    {
        if (m_player == nullptr)
            return;

        godot::Node* spawn_node{ this->find_child(name::level::spawn_point, true, false) };
        godot::Marker2D* spawn_point{ gdcast<godot::Marker2D>(spawn_node) };
        if (spawn_point != nullptr)
            m_player->set_global_position(spawn_point->get_global_position());
    }

    void Level::spawn_enemies_from_markers()
    {
        resource::preload::packed_scene<Enemy> enemy_scene{ path::scene::Enemy };

        const int child_count{ this->get_child_count() };
        for (int i = 0; i < child_count; ++i)
        {
            godot::Node* child{ this->get_child(i) };
            if (child == nullptr)
                continue;

            const godot::String child_name{ child->get_name() };
            if (!child_name.begins_with(name::level::enemy_spawn_prefix))
                continue;

            godot::Marker2D* marker{ gdcast<godot::Marker2D>(child) };
            if (marker == nullptr)
                continue;

            Enemy* enemy{ enemy_scene.instantiate() };
            if (enemy == nullptr)
                continue;

            enemy->set_global_position(marker->get_global_position());
            this->add_child(enemy);
        }
    }

    void Level::_process(double delta_time)
    {
        if (engine::editor_active())
            return;

        if (this->active() && input::cursor_visible()) [[unlikely]]
            input::hide_cursor();
        else if (!this->active() && !input::cursor_visible()) [[unlikely]]
            input::show_cursor();

        this->queue_redraw();
    }

    void Level::_draw()
    {
        if (this->active()) [[likely]]
        {
            godot::Point2 mouse_pos{ this->get_global_mouse_position() };
            this->draw_circle(mouse_pos, 5, { "DARK_CYAN" });
        }
    }

    void Level::activate(bool active)
    {
        m_active = active;
    }

    bool Level::active() const
    {
        return m_active;
    }

    void Level::_bind_methods()
    {
        bind_member_function(Level, on_character_position_changed);
        bind_member_function(Level, on_player_spawn_projectile);
    }

    [[signal_slot]]
    void Level::on_player_spawn_projectile(godot::Node* obj)
    {
        Projectile* projectile{ m_projectile_spawner->spawn_projectile() };
        if (projectile != nullptr)
        {
            godot::Marker2D* firing_pt{ gdcast<godot::Marker2D>(obj) };
            if (firing_pt != nullptr)
            {
                projectile->set_position(firing_pt->get_global_position());
                projectile->set_rotation(firing_pt->get_global_rotation());
            }

            this->add_child(projectile);
        }
    }

    [[signal_slot]]
    void Level::on_character_position_changed(const godot::Object* const node,
                                              godot::Vector2 location) const
    {
        runtime_assert(node != nullptr);
        auto console{ console::get() };
        console->print("{} ({},{})", io::green(to<std::string>(node->get_class()) + " location: "),
                       io::orange(location.x), io::orange(location.y));
    }
}
