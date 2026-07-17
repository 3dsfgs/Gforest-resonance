#pragma once

#include <string>
#include <string_view>
#include <vector>

#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>

#include "entity/level.hpp"

namespace rl::inline ui
{
    class MainDialog : public godot::Panel
    {
        GDCLASS(MainDialog, godot::Panel);

    public:
        MainDialog() = default;
        ~MainDialog() = default;

        void _ready() override;
        void _process(double delta_time) override;
        void _notification(int notification);

        void connect_to_level();
        void bind_level(Level* level);

        static void _bind_methods()
        {
            bind_member_function(MainDialog, connect_to_level);
            bind_member_function(MainDialog, on_level_state_changed);
        }

    protected:
        [[signal_slot]] void on_level_state_changed(int state);
        void start_narrative(std::string_view full_text, std::string_view hint);
        void stop_narrative();
        void advance_narrative(double delta_time);
        void reveal_next_character();
        void begin_line_pause();
        void finish_narrative();

    private:
        Level* find_level_in_tree();

        Level* m_level{ nullptr };
        godot::RichTextLabel* m_console_label{ nullptr };

        bool m_narrative_active{ false };
        godot::String m_narrative_text;
        godot::String m_hint_text;
        std::vector<int> m_line_lengths;
        int m_line_start_index{ 0 };
        int m_line_index{ 0 };
        int m_char_index{ 0 };
        double m_reveal_timer{ 0.0 };
        double m_line_pause_timer{ 0.0 };
        bool m_line_pause{ false };
    };
}
