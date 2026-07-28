#include <godot_cpp/classes/collision_shape2d.hpp>
#include <godot_cpp/classes/circle_shape2d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>

#include "core/constants.hpp"
#include "entity/character/player.hpp"
#include "entity/pickup/combat_pickup.hpp"
#include "singletons/console.hpp"
#include "util/bind.hpp"
#include "util/io.hpp"

namespace rl
{
    namespace
    {
        godot::PackedVector2Array make_heart_points(const float scale)
        {
            static const godot::Vector2 k_unit[]{
                { 0.0f, 3.5f },  { -10.0f, -2.0f }, { -5.5f, -9.5f }, { 0.0f, -6.0f },
                { 5.5f, -9.5f }, { 10.0f, -2.0f },  { 6.0f, 4.0f },   { 0.0f, 8.5f },
                { -6.0f, 4.0f }, { -10.0f, -2.0f },
            };
            godot::PackedVector2Array points;
            for (const auto& p : k_unit)
                points.push_back({ p.x * scale, p.y * scale });
            return points;
        }

        godot::PackedVector2Array make_diamond_points(const float radius)
        {
            godot::PackedVector2Array points;
            points.push_back({ 0.0f, -radius });
            points.push_back({ radius * 0.7f, 0.0f });
            points.push_back({ 0.0f, radius });
            points.push_back({ -radius * 0.7f, 0.0f });
            return points;
        }
    }

    CombatPickup::CombatPickup()
    {
        this->set_collision_layer(collision::pickups_layer);
        this->set_collision_mask(collision::pickups_mask);
        this->set_monitoring(true);
        this->set_monitorable(false);
    }

    void CombatPickup::configure(const Kind kind)
    {
        m_kind = kind;
    }

    CombatPickup::Kind CombatPickup::get_kind() const
    {
        return m_kind;
    }

    void CombatPickup::_ready()
    {
        this->build_visual();

        godot::CollisionShape2D* shape_node{ memnew(godot::CollisionShape2D) };
        godot::Ref<godot::CircleShape2D> circle{ memnew(godot::CircleShape2D) };
        circle->set_radius(combat::pickup_collect_radius);
        shape_node->set_shape(circle);
        this->add_child(shape_node);

        signal<event::body_entered>::connect<CombatPickup>(this) <=>
            signal_callback(this, on_body_entered);

        this->set_process(true);
    }

    void CombatPickup::build_visual()
    {
        m_visual = memnew(godot::Polygon2D);
        m_visual->set_z_index(6);
        if (m_kind == Kind::Heal)
        {
            m_visual->set_polygon(make_heart_points(1.1f));
            m_visual->set_color(godot::Color(1.0f, 0.35f, 0.42f, 0.92f));
            this->set_name("HealPickup");
        }
        else
        {
            m_visual->set_polygon(make_diamond_points(14.0f));
            m_visual->set_color(godot::Color(0.45f, 0.95f, 0.75f, 0.9f));
            this->set_name("HastePickup");
        }
        this->add_child(m_visual);
    }

    void CombatPickup::_process(const double delta_time)
    {
        m_elapsed += delta_time;
        if (!m_base_captured)
        {
            m_base_position = this->get_position();
            m_base_captured = true;
        }

        const float bob{ static_cast<float>(
            godot::Math::sin(m_elapsed * combat::pickup_bob_speed) * combat::pickup_bob_amplitude) };
        this->set_position({ m_base_position.x, m_base_position.y + bob });
    }

    void CombatPickup::on_body_entered(godot::Node* body)
    {
        if (m_collected || body == nullptr)
            return;

        Player* player{ godot::Object::cast_to<Player>(body) };
        if (player == nullptr || !player->is_alive())
            return;

        this->collect(player);
    }

    void CombatPickup::collect(Player* player)
    {
        if (m_collected || player == nullptr)
            return;

        m_collected = true;
        if (m_kind == Kind::Heal)
        {
            player->collect_heal_pickup(combat::pickup_heal_hearts);
            console::get()->print("{} {}", io::green("pickup"), io::yellow("+1 heart"));
        }
        else
        {
            player->collect_haste_pickup(combat::pickup_haste_duration, combat::pickup_haste_mult);
            console::get()->print("{} {}", io::green("pickup"), io::yellow("haste"));
        }
        this->queue_free();
    }

    void CombatPickup::_bind_methods()
    {
        bind_member_function(CombatPickup, on_body_entered);
    }
}
