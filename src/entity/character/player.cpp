#include <godot_cpp/classes/kinematic_collision2d.hpp>
#include <godot_cpp/classes/ref.hpp>

#include "core/constants.hpp"
#include "entity/character/character.hpp"
#include "entity/character/enemy.hpp"
#include "entity/character/player.hpp"
#include "singletons/console.hpp"
#include "util/conversions.hpp"
#include "util/io.hpp"
#include "util/scene.hpp"

namespace rl
{
    Player::Player()
        : Character()
    {
        scene::node::set_unique_name(this, name::character::player);
        m_health.set_max(5);
    }

    void Player::_ready()
    {
        Character::_ready();
        emit_hearts_changed();
    }

    void Player::process_slide_collisions()
    {
        std::unordered_set<uint64_t> touching{};

        for (int i = 0; i < this->get_slide_collision_count(); ++i)
        {
            godot::Ref<godot::KinematicCollision2D> collision{ this->get_slide_collision(i) };
            if (!collision.is_valid())
                continue;

            godot::Object* collider{ collision->get_collider() };
            if (collider == nullptr)
                continue;

            const uint64_t id{ collider->get_instance_id() };
            touching.insert(id);

            if (Enemy* enemy{ godot::Object::cast_to<Enemy>(collider) })
            {
                if (take_damage(combat::enemy_contact_damage_hearts))
                {
                    console::get()->print("{} {}", io::red("enemy contact"),
                                          io::yellow("-1 heart"));
                }
                continue;
            }

            if (!m_active_zone_colliders.contains(id))
                handle_zone_contact(gdcast<godot::PhysicsBody2D>(collider));
        }

        m_active_zone_colliders = std::move(touching);
    }

    void Player::handle_zone_contact(godot::PhysicsBody2D* body)
    {
        if (body == nullptr || !is_alive())
            return;

        const uint32_t layer{ body->get_collision_layer() };
        const auto death_mask{ static_cast<uint32_t>(LayerID::DeathZones) };
        const auto damage_mask{ static_cast<uint32_t>(LayerID::DamageZones) };

        if ((layer & death_mask) != 0)
        {
            take_damage(get_max_hearts(), true);
            console::get()->print("{} {}", io::red("death zone"), io::yellow("instant death"));
            return;
        }

        if ((layer & damage_mask) != 0)
        {
            take_damage(1);
            console::get()->print("{} {}", io::orange("damage zone"), io::yellow("-1 heart"));
        }
    }

    void Player::_bind_methods()
    {
    }
}
