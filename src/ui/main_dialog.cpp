#include <godot_cpp/classes/canvas_item.hpp>
#include <godot_cpp/classes/canvas_layer.hpp>
#include <godot_cpp/classes/control.hpp>

#include "core/constants.hpp"
#include "singletons/console.hpp"
#include "ui/main_dialog.hpp"
#include "util/bind.hpp"
#include "util/conversions.hpp"
#include "util/engine.hpp"

namespace rl::inline ui
{
    namespace
    {
        std::vector<int> build_line_lengths(const godot::String& text)
        {
            std::vector<int> lengths;
            if (text.is_empty())
                return lengths;

            int line_start{ 0 };
            for (int i = 0; i < text.length(); ++i)
            {
                if (text[i] != U'\n')
                    continue;

                lengths.push_back(i - line_start);
                line_start = i + 1;
            }

            lengths.push_back(text.length() - line_start);
            return lengths;
        }
    }

    void MainDialog::_ready()
    {
        if (engine::editor_active())
            return;

        this->apply_release_shell();

        Console<godot::RichTextLabel>* game_console{ console::get() };

        godot::Node* root{ scene::tree::root_node(this) };
        godot::Node* label{ root->find_child(name::dialog::console, true, false) };

        m_level = this->find_level_in_tree();
        m_console_label = try_gdcast<godot::RichTextLabel>(label);

        game_console->set_context(m_console_label);

        this->call_deferred("connect_to_level");
    }

    void MainDialog::apply_release_shell()
    {
        // Exported release: hide template debug chrome (File/Debug menu + console dock).
        if (engine::debug_build())
            return;

        if (godot::Node* menu{ this->find_child("FileMenuContainer", true, false) })
        {
            if (godot::CanvasItem* item{ godot::Object::cast_to<godot::CanvasItem>(menu) })
                item->set_visible(false);
        }

        if (godot::Node* lower{ this->find_child("LowerDialogVertLayout", true, false) })
        {
            if (godot::CanvasItem* item{ godot::Object::cast_to<godot::CanvasItem>(lower) })
                item->set_visible(false);
        }

        // Keep spdlog alive until process exit; only detach the hidden console widget.
        if (console::get() != nullptr)
            console::get()->clear_context();
    }

    Level* MainDialog::find_level_in_tree()
    {
        godot::Node* viewport_node{
            this->find_child(name::dialog::game_sub_viewport, true, false) };
        if (viewport_node == nullptr)
            return nullptr;

        const int child_count{ viewport_node->get_child_count() };
        for (int i = 0; i < child_count; ++i)
        {
            if (Level* level{ godot::Object::cast_to<Level>(viewport_node->get_child(i)) })
                return level;
        }
        return nullptr;
    }

    void MainDialog::bind_level(Level* level)
    {
        m_level = level;
        this->connect_to_level();
    }

    void MainDialog::connect_to_level()
    {
        if (m_level == nullptr)
            m_level = this->find_level_in_tree();

        if (m_level == nullptr)
            return;

        signal<event::level_state_changed>::connect<Level>(m_level) <=>
            signal_callback(this, on_level_state_changed);
        this->stop_narrative();
    }

    void MainDialog::_process(double delta_time)
    {
        if (engine::editor_active())
            return;

        if (m_narrative_active)
            this->advance_narrative(delta_time);
    }

    void MainDialog::_notification(int notification)
    {
        switch (notification)
        {
            case Object::NOTIFICATION_PREDELETE:
                [[fallthrough]];
            case Node::NOTIFICATION_UNPARENTED:
            {
                if (auto* c = console::get())
                {
                    c->clear_context();
                    c->stop_logging();
                }
                break;
            }
            case Control::NOTIFICATION_MOUSE_ENTER:
            {
                if (m_level != nullptr)
                    m_level->activate(true);
                break;
            }
            case Control::NOTIFICATION_MOUSE_EXIT:
            {
                if (m_level != nullptr)
                    m_level->activate(false);
                break;
            }
        }
    }

    void MainDialog::on_level_state_changed(const int state)
    {
        const auto level_state{ static_cast<LevelState>(state) };

        if (level_state == LevelState::Playing)
        {
            this->stop_narrative();
            return;
        }

        if (level_state == LevelState::Victory)
        {
            // Day12：全屏结语由 ending_screen 接管，Console 不再播胜利叙事。
            this->stop_narrative();
            return;
        }

        if (level_state == LevelState::Defeat)
            this->start_narrative(narrative::defeat_text, narrative::defeat_hint);
    }

    void MainDialog::start_narrative(const std::string_view full_text, const std::string_view hint)
    {
        if (m_console_label == nullptr)
            return;

        this->stop_narrative();

        m_narrative_text = godot::String::utf8(full_text.data());
        m_hint_text = godot::String::utf8(hint.data());
        m_line_lengths = build_line_lengths(m_narrative_text);
        m_line_start_index = 0;
        m_line_index = 0;
        m_char_index = 0;
        m_reveal_timer = 0.0;
        m_line_pause_timer = 0.0;
        m_line_pause = false;
        m_narrative_active = !m_line_lengths.empty();

        m_console_label->clear();
    }

    void MainDialog::stop_narrative()
    {
        m_narrative_active = false;
        m_narrative_text = godot::String{};
        m_hint_text = godot::String{};
        m_line_lengths.clear();
        m_line_start_index = 0;
        m_line_index = 0;
        m_char_index = 0;
        m_reveal_timer = 0.0;
        m_line_pause_timer = 0.0;
        m_line_pause = false;

        if (m_console_label != nullptr)
            m_console_label->clear();
    }

    void MainDialog::advance_narrative(const double delta_time)
    {
        if (m_line_pause)
        {
            m_line_pause_timer -= delta_time;
            if (m_line_pause_timer > 0.0)
                return;

            m_line_pause = false;
            ++m_line_index;
            m_char_index = 0;

            if (m_line_index >= static_cast<int>(m_line_lengths.size()))
            {
                this->finish_narrative();
                return;
            }

            m_line_start_index += m_line_lengths[static_cast<std::size_t>(m_line_index - 1)] + 1;

            if (m_line_index > 0 && m_console_label != nullptr)
                m_console_label->append_text("\n");
        }

        m_reveal_timer += delta_time;
        while (m_reveal_timer >= narrative::char_reveal_interval)
        {
            m_reveal_timer -= narrative::char_reveal_interval;
            this->reveal_next_character();

            if (!m_narrative_active || m_line_pause)
                break;
        }
    }

    void MainDialog::reveal_next_character()
    {
        if (m_console_label == nullptr || m_line_index >= static_cast<int>(m_line_lengths.size()))
            return;

        const int current_line_length{ m_line_lengths[static_cast<std::size_t>(m_line_index)] };

        if (m_char_index >= current_line_length)
        {
            this->begin_line_pause();
            return;
        }

        const int absolute_index{ m_line_start_index + m_char_index };
        m_console_label->append_text(m_narrative_text.substr(absolute_index, 1));
        ++m_char_index;

        if (m_char_index >= current_line_length)
            this->begin_line_pause();
    }

    void MainDialog::begin_line_pause()
    {
        if (m_line_index + 1 >= static_cast<int>(m_line_lengths.size()))
        {
            this->finish_narrative();
            return;
        }

        m_line_pause = true;
        m_line_pause_timer = narrative::line_pause_interval;
    }

    void MainDialog::finish_narrative()
    {
        m_narrative_active = false;

        if (m_console_label == nullptr)
            return;

        m_console_label->append_text("\n\n[color=#b6d4ca]" + m_hint_text + "[/color]");
    }
}
