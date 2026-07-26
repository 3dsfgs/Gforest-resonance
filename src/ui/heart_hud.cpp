#include <algorithm>
#include <array>

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
        constexpr float heart_half_width{ 11.0f };
        constexpr float heart_half_height{ 10.0f };
        constexpr float heart_spacing{ 28.0f };
        constexpr float heart_padding{ 8.0f };
        constexpr float dash_bar_height{ 8.0f };
        constexpr float skill_bar_height{ 6.0f };
        constexpr float dash_bar_gap{ 6.0f };
        constexpr float skill_bar_gap{ 4.0f };
        constexpr float dash_bar_min_width{ 120.0f };

        godot::PackedVector2Array heart_polygon(const godot::Vector2 center)
        {
            // 经典心形轮廓（局部坐标 → 平移到 center）
            static const std::array<godot::Vector2, 10> k_unit_points{ {
                { 0.0f, 3.5f },
                { -10.0f, -2.0f },
                { -5.5f, -9.5f },
                { 0.0f, -6.0f },
                { 5.5f, -9.5f },
                { 10.0f, -2.0f },
                { 6.0f, 4.0f },
                { 0.0f, 8.5f },
                { -6.0f, 4.0f },
                { -10.0f, -2.0f },
            } };

            godot::PackedVector2Array points;
            points.resize(static_cast<int>(k_unit_points.size()));
            for (std::size_t i = 0; i < k_unit_points.size(); ++i)
            {
                const godot::Vector2 scaled{ k_unit_points[i].x * (heart_half_width / 10.0f),
                                             k_unit_points[i].y * (heart_half_height / 10.0f) };
                points[static_cast<int>(i)] = center + scaled;
            }
            return points;
        }

        void draw_heart(godot::Control* control, const godot::Vector2 center, const godot::Color fill,
                        const godot::Color outline)
        {
            const godot::PackedVector2Array points{ heart_polygon(center) };
            control->draw_colored_polygon(points, fill);
            for (int i = 0; i < points.size(); ++i)
            {
                const int next{ (i + 1) % points.size() };
                control->draw_line(points[i], points[next], outline, 1.5f, true);
            }
        }

        void draw_ready_bar(godot::Control* control, const float x, const float y, const float width,
                            const float height, const double ready_ratio,
                            const godot::Color ready_color, const godot::Color cooling_color)
        {
            const godot::Rect2 track{ x, y, width, height };
            control->draw_rect(track, godot::Color{ 0.18f, 0.2f, 0.24f, 0.9f });

            const float fill_width{ width * static_cast<float>(ready_ratio) };
            if (fill_width <= 0.5f)
                return;

            const bool ready{ ready_ratio >= 0.999 };
            control->draw_rect(godot::Rect2{ x, y, fill_width, height },
                               ready ? ready_color : cooling_color);
        }
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

        const double dash{ m_player->get_dash_ready_ratio() };
        const double slash{ m_player->get_polar_slash_ready_ratio() };
        const double light{ m_player->get_energy_light_ready_ratio() };
        const bool changed{ godot::Math::abs(dash - m_dash_ready_ratio) > 0.001 ||
                            godot::Math::abs(slash - m_polar_slash_ready_ratio) > 0.001 ||
                            godot::Math::abs(light - m_energy_light_ready_ratio) > 0.001 };
        if (!changed)
            return;

        m_dash_ready_ratio = dash;
        m_polar_slash_ready_ratio = slash;
        m_energy_light_ready_ratio = light;
        this->queue_redraw();
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
        m_polar_slash_ready_ratio = 1.0;
        m_energy_light_ready_ratio = 1.0;
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
        m_polar_slash_ready_ratio = m_player->get_polar_slash_ready_ratio();
        m_energy_light_ready_ratio = m_player->get_energy_light_ready_ratio();

        signal<event::hearts_changed>::connect<Player>(m_player) <=>
            signal_callback(this, on_player_hearts_changed);

        signal<event::died>::connect<Player>(m_player) <=> signal_callback(this, on_player_died);
    }

    void HeartHud::update_layout_size()
    {
        const float hearts_width{
            heart_padding * 2.0f + heart_spacing * static_cast<float>(m_max_hearts) };
        const float width{ std::max(hearts_width, dash_bar_min_width) };
        const float height{ heart_padding * 2.0f + heart_half_height * 2.0f + dash_bar_gap +
                            dash_bar_height + skill_bar_gap + skill_bar_height + skill_bar_gap +
                            skill_bar_height };
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
            const godot::Color fill = filled ? godot::Color{ 1.0f, 0.78f, 0.22f, 0.95f }
                                             : godot::Color{ 0.28f, 0.28f, 0.34f, 0.55f };
            const godot::Color outline = filled ? godot::Color{ 0.55f, 0.32f, 0.08f, 0.9f }
                                                : godot::Color{ 0.4f, 0.4f, 0.48f, 0.7f };
            const godot::Vector2 center{ heart_padding + heart_half_width +
                                             static_cast<float>(i) * heart_spacing,
                                         heart_padding + heart_half_height };
            draw_heart(this, center, fill, outline);
        }

        const float bar_width{ this->get_size().x - heart_padding * 2.0f };
        float bar_y{ heart_padding + heart_half_height * 2.0f + dash_bar_gap };

        // Dash (cyan)
        draw_ready_bar(this, heart_padding, bar_y, bar_width, dash_bar_height, m_dash_ready_ratio,
                       godot::Color{ 0.35f, 0.85f, 0.95f, 0.95f },
                       godot::Color{ 0.45f, 0.55f, 0.7f, 0.9f });
        bar_y += dash_bar_height + skill_bar_gap;

        // Polar slash Q (cold cyan)
        draw_ready_bar(this, heart_padding, bar_y, bar_width, skill_bar_height,
                       m_polar_slash_ready_ratio, godot::Color{ 0.4f, 0.85f, 1.0f, 0.95f },
                       godot::Color{ 0.25f, 0.45f, 0.6f, 0.9f });
        bar_y += skill_bar_height + skill_bar_gap;

        // Energy light E (warm gold)
        draw_ready_bar(this, heart_padding, bar_y, bar_width, skill_bar_height,
                       m_energy_light_ready_ratio, godot::Color{ 1.0f, 0.78f, 0.28f, 0.95f },
                       godot::Color{ 0.55f, 0.4f, 0.18f, 0.9f });
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
