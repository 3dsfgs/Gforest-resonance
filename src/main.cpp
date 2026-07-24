#include "core/assert.hpp"
#include "core/constants.hpp"
#include "main.hpp"
#include "entity/level.hpp"
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
    namespace
    {
        void seed_legacy_run(std::vector<RunRoomEntry>& rooms)
        {
            rooms.clear();
            for (int i = 0; i < level::room_count; ++i)
            {
                RunRoomEntry entry{};
                entry.scene_path = godot::String{ path::scene::room_paths[i] };
                entry.display_name = godot::String::utf8(level::room_display_names[i]);
                entry.room_kind = i >= level::room_count - 1 ? "boss" : "combat";
                entry.is_final = i >= level::room_count - 1;
                rooms.push_back(entry);
            }
        }
    }

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

        if (m_run_rooms.empty())
            seed_legacy_run(m_run_rooms);

        m_weapon_id = "pulse";
        this->load_room(0);
    }

    void Main::set_run_weapon(const godot::String& weapon_id)
    {
        m_weapon_id = weapon_id.is_empty() ? godot::String{ "pulse" } : weapon_id;
        if (m_active_level != nullptr)
            m_active_level->apply_player_weapon(m_weapon_id);
    }

    godot::String Main::get_run_weapon() const
    {
        return m_weapon_id;
    }

    void Main::configure_run(const godot::Array& rooms)
    {
        m_run_rooms.clear();

        for (int i = 0; i < rooms.size(); ++i)
        {
            const godot::Variant& item{ rooms[i] };
            if (item.get_type() != godot::Variant::DICTIONARY)
                continue;

            const godot::Dictionary dict{ item };
            RunRoomEntry entry{};
            entry.scene_path = dict.get("scene_path", godot::String{});
            entry.display_name = dict.get("display_name", godot::String{});
            entry.room_kind = dict.get("room_kind", godot::String{ "combat" });
            entry.is_final = dict.get("is_final", false);

            if (entry.scene_path.is_empty())
                continue;

            m_run_rooms.push_back(entry);
        }

        if (m_run_rooms.empty())
            seed_legacy_run(m_run_rooms);
    }

    RoomKind Main::parse_room_kind(const godot::String& room_kind) const
    {
        if (room_kind == "whisper")
            return RoomKind::Whisper;
        if (room_kind == "mood")
            return RoomKind::Mood;
        if (room_kind == "boss")
            return RoomKind::Boss;
        return RoomKind::Combat;
    }

    void Main::advance_to_room(const int room_index, const int carry_hearts)
    {
        this->load_room(room_index, carry_hearts);
    }

    void Main::return_to_title()
    {
        this->unload_active_level();
        m_run_rooms.clear();
        m_weapon_id = "pulse";
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

        if (room_index < 0 || room_index >= static_cast<int>(m_run_rooms.size()))
            return;

        const int previous_hearts{ carry_hearts };
        const RunRoomEntry& room{ m_run_rooms[static_cast<std::size_t>(room_index)] };

        this->unload_active_level();

        m_room_index = room_index;
        resource::preload::packed_scene<Level> level_scene{ room.scene_path };
        m_active_level = level_scene.instantiate();
        runtime_assert(m_active_level != nullptr);

        m_active_level->set_room_index(room_index);
        m_active_level->set_room_kind(this->parse_room_kind(room.room_kind));
        m_active_level->set_is_final_room(room.is_final);
        m_active_level->set_weapon_id(m_weapon_id);
        m_game_viewport->add_child(m_active_level);
        this->bind_active_level_signals();

        if (previous_hearts > 0)
            m_active_level->apply_player_hearts(previous_hearts);

        console::get()->print("{} {} ({})", io::green("enter room"),
                              io::blue(std::to_string(room_index + 1)),
                              io::yellow(room.room_kind.utf8().get_data()));
    }

    [[signal_slot]]
    void Main::on_room_cleared(const int room_index)
    {
        if (m_active_level == nullptr)
            return;

        const int hearts{ m_active_level->get_player_hearts() };
        m_weapon_id = m_active_level->get_player_weapon();
        const int next_index{ room_index + 1 };

        if (next_index < 0 || next_index >= static_cast<int>(m_run_rooms.size()))
            return;

        const godot::String display_name{ m_run_rooms[static_cast<std::size_t>(next_index)]
                                              .display_name };

        console::get()->print("{} {}", io::green("room cleared"),
                              io::blue(std::to_string(room_index + 1)));
        this->emit_signal(event::room_advance_requested, next_index, hearts, display_name);
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
