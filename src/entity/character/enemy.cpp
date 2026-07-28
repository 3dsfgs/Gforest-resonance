#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "core/constants.hpp"
#include "entity/character/character.hpp"
#include "entity/character/enemy.hpp"
#include "entity/pickup/combat_pickup.hpp"
#include "singletons/console.hpp"
#include "util/bind.hpp"
#include "util/combat_feedback.hpp"
#include "util/conversions.hpp"
#include "util/io.hpp"
#include "util/scene.hpp"

namespace rl
{
    namespace
    {
        void try_spawn_combat_pickup(godot::Node* parent, const godot::Vector2& death_pos)
        {
            if (parent == nullptr)
                return;

            CombatPickup::Kind kind{ CombatPickup::Kind::Heal };
            const double roll{ godot::UtilityFunctions::randf() };
            if (roll < combat::pickup_heal_drop_chance)
                kind = CombatPickup::Kind::Heal;
            else if (roll < combat::pickup_heal_drop_chance + combat::pickup_haste_drop_chance)
                kind = CombatPickup::Kind::Haste;
            else
                return;

            CombatPickup* pickup{ memnew(CombatPickup) };
            pickup->configure(kind);
            parent->add_child(pickup);
            pickup->set_global_position(death_pos);
        }
    }

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
        combat_feedback::spawn_kill_explosion(parent, death_pos, this->kill_vfx_kind());
        combat_feedback::play_enemy_kill(parent, death_pos);
        try_spawn_combat_pickup(parent, death_pos);

        this->queue_free();
    }

    void Enemy::_bind_methods()
    {
        bind_member_function(Enemy, on_died);
    }
}
