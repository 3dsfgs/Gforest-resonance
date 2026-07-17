#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/classes/kinematic_collision2d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/color.hpp>

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
        this->set_collision_mask(collision::player_mask);
    }

    void Player::_ready()
    {
        Character::_ready();
        this->set_collision_mask(collision::player_mask);
        emit_hearts_changed();
    }

    godot::Color Player::hit_flash_color() const
    {
        return godot::Color(combat::player_hit_flash_r, combat::player_hit_flash_g,
                            combat::player_hit_flash_b, 1.0f);
    }

    void Player::process_slide_collisions()
    {
        for (int i = 0; i < this->get_slide_collision_count(); ++i)
        {
            godot::Ref<godot::KinematicCollision2D> collision{ this->get_slide_collision(i) };
            if (!collision.is_valid())
                continue;

            godot::Object* collider{ collision->get_collider() };
            if (collider == nullptr)
                continue;

            if (Enemy* enemy{ godot::Object::cast_to<Enemy>(collider) })
            {
                if (take_damage(combat::enemy_contact_damage_hearts))
                {
                    console::get()->print("{} {}", io::red("enemy contact"),
                                          io::yellow("-1 heart"));
                }
            }
        }

        this->process_area_traps();
    }

    void Player::process_area_traps()
    {
        godot::Node* parent{ this->get_parent() };
        if (parent == nullptr)
            return;

        std::unordered_set<uint64_t> touching{};

        const int child_count{ parent->get_child_count() };
        for (int i = 0; i < child_count; ++i)
        {
            godot::Area2D* area{ godot::Object::cast_to<godot::Area2D>(parent->get_child(i)) };
            if (area == nullptr)
                continue;

            const uint32_t layer{ area->get_collision_layer() };
            const bool is_trap{ (layer & static_cast<uint32_t>(LayerID::DamageZones)) != 0 ||
                                (layer & static_cast<uint32_t>(LayerID::DeathZones)) != 0 };
            if (!is_trap)
                continue;

            if (!area->overlaps_body(this))
                continue;

            const uint64_t id{ area->get_instance_id() };
            touching.insert(id);

            if (!m_active_zone_areas.contains(id))
                this->handle_zone_area(area);
        }

        m_active_zone_areas = std::move(touching);
    }

    void Player::handle_zone_area(godot::Area2D* area)
    {
        if (area == nullptr || !is_alive())
            return;

        const uint32_t layer{ area->get_collision_layer() };
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
