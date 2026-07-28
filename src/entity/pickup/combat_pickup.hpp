#pragma once

#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/classes/polygon2d.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "core/constants.hpp"
#include "util/bind.hpp"

namespace rl
{
    /** Ground pickup: heal heart or temporary haste (⭐2). */
    class CombatPickup : public godot::Area2D
    {
        GDCLASS(CombatPickup, godot::Area2D);

    public:
        enum class Kind {
            Heal,
            Haste,
        };

        CombatPickup();
        ~CombatPickup() = default;

        void _ready() override;
        void _process(double delta_time) override;

        void configure(Kind kind);
        [[nodiscard]] Kind get_kind() const;

    protected:
        [[signal_slot]] void on_body_entered(godot::Node* body);
        static void _bind_methods();

    private:
        void build_visual();
        void collect(class Player* player);

        Kind m_kind{ Kind::Heal };
        bool m_collected{ false };
        double m_elapsed{ 0.0 };
        godot::Polygon2D* m_visual{ nullptr };
        godot::Vector2 m_base_position{ 0.0f, 0.0f };
        bool m_base_captured{ false };
    };
}
