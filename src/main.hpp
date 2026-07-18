#pragma once

#include <godot_cpp/classes/canvas_layer.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/sub_viewport.hpp>

#include "entity/level.hpp"
#include "ui/main_dialog.hpp"
#include "util/bind.hpp"

namespace rl
{
    class Main : public godot::Node
    {
        GDCLASS(Main, godot::Node);

    public:
        Main();
        ~Main() = default;

        void _ready() override;
        void _physics_process(double delta) override;

    protected:
        void apply_default_settings();

        /** Called by birthday UI after profile.json is written. */
        void begin_run();

        static void _bind_methods()
        {
            bind_member_function(Main, on_room_cleared);
            bind_member_function(Main, on_run_restart);
            bind_member_function(Main, begin_run);
            signal_binding<Main, event::signal_example>::add<double>();
        }

        [[signal_slot]] void on_room_cleared(int room_index);
        [[signal_slot]] void on_run_restart();

    private:
        void load_room(int room_index, int carry_hearts = -1);
        void bind_active_level_signals();

        double m_signal_timer{ 0.0 };
        int m_room_index{ 0 };
        godot::CanvasLayer* m_canvas_layer{ nullptr };
        godot::SubViewport* m_game_viewport{ nullptr };
        MainDialog* m_main_dialog{ nullptr };
        Level* m_active_level{ nullptr };
    };
}
