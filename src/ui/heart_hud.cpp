#include <algorithm>

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "core/assert.hpp"
#include "core/constants.hpp"
#include "entity/character/player.hpp"
#include "entity/level.hpp"
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
        constexpr float dash_bar_height{ 8.0f };
        constexpr float dash_bar_gap{ 6.0f };
        constexpr float dash_bar_min_width{ 120.0f };
    }

    void HeartHud::_ready()
    {
        this->set_mouse_filter(godot::Control::MOUSE_FILTER_IGNORE);
        this->set_z_index(100);

        if (engine::editor_active())
            return;

        this->set_process(true);
        this->call_deferred("connect_to_player");
    }

    void HeartHud::_process(const double delta_time)
    {
        (void)delta_time;

        if (m_player == nullptr)
            return;

        const double ratio{ m_player->get_dash_ready_ratio() };
        if (godot::Math::abs(ratio - m_dash_ready_ratio) > 0.001)
        {
            m_dash_ready_ratio = ratio;
            this->queue_redraw();
        }
    }

    void HeartHud::disconnect_from_player()
    {
        if (m_player == nullptr)
            return;

        const godot::Callable hearts_cb{ this, "on_player_hearts_changed" };
        const godot::Callable died_cb{ this, "on_player_died" };

        if (m_player->is_connected(event::hearts_changed, hearts_cb))
            m_player->disconnect(event::hearts_changed, hearts_cb);

        if (m_player->is_connected(event::died, died_cb))
            m_player->disconnect(event::died, died_cb);

        m_player = nullptr;
        m_dash_ready_ratio = 1.0;
        this->queue_redraw();
    }

    void HeartHud::connect_to_player()
    {
        godot::Node* const viewport{ this->get_parent() != nullptr ? this->get_parent()->get_parent()
                                                                   : nullptr };
        if (viewport == nullptr)
            return;

        Level* level{ nullptr };
        const int child_count{ viewport->get_child_count() };
        for (int i = 0; i < child_count; ++i)
        {
            level = godot::Object::cast_to<Level>(viewport->get_child(i));
            if (level != nullptr)
                break;
        }

        if (level == nullptr)
            return;

        Player* player{ godot::Object::cast_to<Player>(
            level->find_child(name::character::player, true, false)) };
        if (player == nullptr)
        {
            // Level::_ready may not have spawned the player yet.
            this->call_deferred("connect_to_player");
            return;
        }

        if (m_player == player)
            return;

        this->disconnect_from_player();
        m_player = player;

        update_hearts(m_player->get_hearts(), m_player->get_max_hearts());
        m_dash_ready_ratio = m_player->get_dash_ready_ratio();

        signal<event::hearts_changed>::connect<Player>(m_player) <=>
            signal_callback(this, on_player_hearts_changed);

        signal<event::died>::connect<Player>(m_player) <=> signal_callback(this, on_player_died);
    }

    void HeartHud::update_layout_size()
    {
        const float hearts_width{
            heart_padding * 2.0f + heart_spacing * static_cast<float>(m_max_hearts) };
        const float width{ std::max(hearts_width, dash_bar_min_width) };
        const float height{ heart_padding * 2.0f + heart_radius * 2.0f + dash_bar_gap +
                            dash_bar_height };
        this->set_custom_minimum_size({ width, height });
        this->set_size({ width, height });
    }

    void HeartHud::update_hearts(const int current, const int max_hearts)
    {
        m_current_hearts = current;
        m_max_hearts = max_hearts;
        this->update_layout_size();
        this->queue_redraw();
    }

    void HeartHud::_draw()
    {
        for (int i = 0; i < m_max_hearts; ++i)
        {
            const bool filled{ i < m_current_hearts };
            const godot::Color color = filled ? godot::Color{ 1.0f, 0.22f, 0.33f }
                                              : godot::Color{ 0.28f, 0.28f, 0.34f, 0.85f };
            const godot::Vector2 center{ heart_padding + heart_radius +
                                             static_cast<float>(i) * heart_spacing,
                                         heart_padding + heart_radius };
            this->draw_circle(center, heart_radius, color);
        }

        const float bar_y{ heart_padding + heart_radius * 2.0f + dash_bar_gap };
        const float bar_width{ this->get_size().x - heart_padding * 2.0f };
        const godot::Rect2 track{ heart_padding, bar_y, bar_width, dash_bar_height };
        this->draw_rect(track, godot::Color{ 0.18f, 0.2f, 0.24f, 0.9f });

        const float fill_width{ bar_width * static_cast<float>(m_dash_ready_ratio) };
        if (fill_width > 0.5f)
        {
            const bool ready{ m_dash_ready_ratio >= 0.999 };
            const godot::Color fill = ready ? godot::Color{ 0.35f, 0.85f, 0.95f, 0.95f }
                                            : godot::Color{ 0.45f, 0.55f, 0.7f, 0.9f };
            this->draw_rect(godot::Rect2{ heart_padding, bar_y, fill_width, dash_bar_height },
                            fill);
        }
    }

    void HeartHud::on_player_hearts_changed(const int current, const int max_hearts)
    {
        update_hearts(current, max_hearts);
    }

    void HeartHud::on_player_died()
    {
        console::get()->print("{} {}", io::red("player"), io::yellow("died"));
        update_hearts(0, m_max_hearts);
    }
}
