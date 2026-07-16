#include "core/constants.hpp"
#include "entity/character/character.hpp"
#include "entity/character/enemy.hpp"
#include "singletons/console.hpp"
#include "util/bind.hpp"
#include "util/combat_feedback.hpp"
#include "util/conversions.hpp"
#include "util/io.hpp"
#include "util/scene.hpp"

namespace rl
{
    Enemy::Enemy()
        : Character()
    {
        scene::node::set_unique_name(this, name::character::enemy);
        m_health.set_max(combat::enemy_default_hearts);
        m_movement_speed = combat::enemy_movement_speed;
    }

    void Enemy::_ready()
    {
        Character::_ready();
        emit_hearts_changed();

        signal<event::died>::connect<Enemy>(this) <=> signal_callback(this, on_died);
    }

    [[signal_slot]]
    void Enemy::on_died()
    {
        console::get()->print("{} {}", io::red("enemy defeated"),
                              io::green(to<std::string>(this->get_name())));

        godot::Node* parent{ this->get_parent() };
        const godot::Vector2 death_pos{ this->get_global_position() };
        combat_feedback::spawn_kill_explosion(parent, death_pos);
        combat_feedback::play_enemy_kill(parent, death_pos);

        this->queue_free();
    }

    void Enemy::_bind_methods()
    {
        bind_member_function(Enemy, on_died);
    }
}
