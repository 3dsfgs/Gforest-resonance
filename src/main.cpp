#include "core/assert.hpp"
#include "core/constants.hpp"
#include "main.hpp"
#include "singletons/console.hpp"
#include "ui/heart_hud.hpp"
#include "util/bind.hpp"
#include "util/conversions.hpp"
#include "util/engine.hpp"
#include "util/input.hpp"
#include "util/io.hpp"
#include "util/scene.hpp"

namespace rl
{
    Main::Main()
    {
        resource::preload::packed_scene<MainDialog> dialog{ path::ui::MainDialog };

        m_main_dialog = dialog.instantiate();
        runtime_assert(m_main_dialog != nullptr);

        if (m_main_dialog != nullptr)
        {
            m_canvas_layer = try_gdcast<godot::CanvasLayer>(
                m_main_dialog->find_child(name::dialog::canvas_layer, true, false));

            m_game_viewport = gdcast<godot::SubViewport>(
                m_main_dialog->find_child(name::dialog::game_sub_viewport, true, false));

            runtime_assert(m_game_viewport != nullptr);
            this->add_child(m_main_dialog);
        }
    }

    void Main::_ready()
    {
        this->apply_default_settings();
        // Day9+：不自动进关；标题/生日 UI 调用 begin_run()。
    }

    void Main::begin_run()
    {
        if (m_active_level != nullptr)
            return;

        this->load_room(0);
    }

    void Main::return_to_title()
    {
        this->unload_active_level();
        input::show_cursor();
        console::get()->print("{}", io::green("return to title"));
    }

    void Main::_physics_process(double delta)
    {
        if (engine::editor_active())
            return;

        m_signal_timer += delta;
        if (m_signal_timer > 1.0)
        {
            this->emit_signal(event::signal_example, delta);
            m_signal_timer -= 1.0;
        }
    }

    void Main::apply_default_settings()
    {
        engine::set_fps(60);
        input::use_accumulated_inputs(false);

        if (not engine::editor_active())
            engine::root_window()->set_size({ 1920, 1080 });
    }

    void Main::unload_active_level()
    {
        if (m_main_dialog != nullptr)
        {
            if (godot::Node* hud_node{
                    m_main_dialog->find_child(name::ui::heart_hud, true, false) })
            {
                if (auto* hud{ godot::Object::cast_to<HeartHud>(hud_node) })
                    hud->disconnect_from_player();
            }
        }

        if (m_active_level == nullptr)
            return;

        if (m_game_viewport != nullptr)
            m_game_viewport->remove_child(m_active_level);

        m_active_level->queue_free();
        m_active_level = nullptr;
        m_room_index = 0;
    }

    void Main::bind_active_level_signals()
    {
        if (m_active_level == nullptr)
            return;

        signal<event::room_cleared>::connect<Level>(m_active_level) <=>
            signal_callback(this, on_room_cleared);
        signal<event::run_restart>::connect<Level>(m_active_level) <=>
            signal_callback(this, on_run_restart);
        signal<event::level_state_changed>::connect<Level>(m_active_level) <=>
            signal_callback(this, on_level_state_changed);

        if (m_main_dialog != nullptr)
        {
            m_main_dialog->bind_level(m_active_level);
            if (godot::Node* hud_node{
                    m_main_dialog->find_child(name::ui::heart_hud, true, false) })
            {
                if (auto* hud{ godot::Object::cast_to<HeartHud>(hud_node) })
                    hud->call_deferred("connect_to_player");
            }
        }
    }

    void Main::load_room(const int room_index, const int carry_hearts)
    {
        if (m_game_viewport == nullptr)
            return;

        if (room_index < 0 || room_index >= level::room_count)
            return;

        const int previous_hearts{ carry_hearts };

        this->unload_active_level();

        m_room_index = room_index;
        resource::preload::packed_scene<Level> level_scene{
            path::scene::room_paths[room_index] };
        m_active_level = level_scene.instantiate();
        runtime_assert(m_active_level != nullptr);

        m_active_level->set_room_index(room_index);
        m_game_viewport->add_child(m_active_level);
        this->bind_active_level_signals();

        if (previous_hearts > 0)
            m_active_level->apply_player_hearts(previous_hearts);

        console::get()->print("{} {}", io::green("enter room"),
                              io::blue(std::to_string(room_index + 1)));
    }

    [[signal_slot]]
    void Main::on_room_cleared(const int room_index)
    {
        if (m_active_level == nullptr)
            return;

        const int hearts{ m_active_level->get_player_hearts() };
        console::get()->print("{} {}", io::green("room cleared"),
                              io::blue(std::to_string(room_index + 1)));
        this->load_room(room_index + 1, hearts);
    }

    [[signal_slot]]
    void Main::on_run_restart()
    {
        // Day12：Victory 不再走这里；Defeat 仍可能 reset 当前房（Level 内处理）。
        // 若仍收到整局重开，回到房 1。
        console::get()->print("{}", io::green("run restart"));
        this->load_room(0);
    }

    [[signal_slot]]
    void Main::on_level_state_changed(const int state)
    {
        if (static_cast<LevelState>(state) != LevelState::Victory)
            return;

        console::get()->print("{}", io::green("run victory"));
        this->emit_signal(event::run_victory);
    }
}
