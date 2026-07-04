#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "core/assert.hpp"
#include "core/constants.hpp"
#include "entity/character/player.hpp"
#include "singletons/console.hpp"
#include "ui/heart_hud.hpp"
#include "util/bind.hpp"
#include "util/engine.hpp"
#include "util/io.hpp"

namespace rl::inline ui
{
    namespace
    {
        constexpr float heart_radius{ 10.0f };
        constexpr float heart_spacing{ 28.0f };
        constexpr float heart_padding{ 8.0f };
    }

    void HeartHud::_ready()
    {
        this->set_mouse_filter(godot::Control::MOUSE_FILTER_IGNORE);
        this->set_z_index(100);

        if (engine::editor_active())
            return;

        this->call_deferred("connect_to_player");
    }

    void HeartHud::connect_to_player()
    {
        if (m_player != nullptr)
            return;

        godot::Node* const viewport{ this->get_parent()->get_parent() };
        runtime_assert(viewport != nullptr);

        godot::Node* level{ viewport->find_child(name::level::level1, true, false) };
        runtime_assert(level != nullptr);

        m_player = gdcast<Player>(level->find_child(name::character::player, true, false));
        runtime_assert(m_player != nullptr);

        update_hearts(m_player->get_hearts(), m_player->get_max_hearts());

        signal<event::hearts_changed>::connect<Player>(m_player) <=>
            signal_callback(this, on_player_hearts_changed);

        signal<event::died>::connect<Player>(m_player) <=> signal_callback(this, on_player_died);
    }

    void HeartHud::update_hearts(const int current, const int max_hearts)
    {
        m_current_hearts = current;
        m_max_hearts = max_hearts;

        const float width{ heart_padding * 2.0f + heart_spacing * static_cast<float>(max_hearts) };
        const float height{ heart_padding * 2.0f + heart_radius * 2.0f };
        this->set_custom_minimum_size({ width, height });
        this->set_size({ width, height });
        this->queue_redraw();
    }

    void HeartHud::_draw()
    {
        for (int i = 0; i < m_max_hearts; ++i)
        {
            const bool filled{ i < m_current_hearts };
            const godot::Color color = filled ? godot::Color{ 1.0f, 0.22f, 0.33f }
                                              : godot::Color{ 0.28f, 0.28f, 0.34f, 0.85f };
            const godot::Vector2 center{ heart_padding + heart_radius + static_cast<float>(i) * heart_spacing,
                                         heart_padding + heart_radius };
            this->draw_circle(center, heart_radius, color);
        }
    }

    void HeartHud::on_player_hearts_changed(const int current, const int max_hearts)
    {
        update_hearts(current, max_hearts);
    }

    void HeartHud::on_player_died()
    {
        console::get()->print("{} {}", io::red("player"), io::yellow("died"));
        update_hearts(0, m_player != nullptr ? m_player->get_max_hearts() : m_max_hearts);
    }
}
