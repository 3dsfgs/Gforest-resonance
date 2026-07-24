#pragma once

#include <vector>

#include <godot_cpp/classes/canvas_layer.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/sub_viewport.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>

#include "entity/level.hpp"
#include "ui/main_dialog.hpp"
#include "util/bind.hpp"

namespace rl
{
    struct RunRoomEntry {
        godot::String scene_path{};
        godot::String display_name{};
        godot::String room_kind{};
        bool is_final{ false };
    };

    class Main : public godot::Node
    {
        GDCLASS(Main, godot::Node);

    public:
        Main();
        ~Main() = default;

        void _ready() override;
        void _physics_process(double delta) override;

        /** Called by birthday UI after profile.json is written. */
        void begin_run();
        /** P2-6：GDScript 在 begin_run 前注入当晚梦房链。 */
        void configure_run(const godot::Array& rooms);
        /** P0-4：GDScript 过场播完后加载下一房。 */
        void advance_to_room(int room_index, int carry_hearts);
        /** P2-1：心境房唤醒武器后写入，跨房保留。 */
        void set_run_weapon(const godot::String& weapon_id);
        [[nodiscard]] godot::String get_run_weapon() const;
        /** Day12：卸下关卡，回到标题前调用。 */
        void return_to_title();

    protected:
        void apply_default_settings();

        static void _bind_methods()
        {
            bind_member_function(Main, on_room_cleared);
            bind_member_function(Main, on_run_restart);
            bind_member_function(Main, on_level_state_changed);
            bind_member_function(Main, begin_run);
            bind_member_function(Main, configure_run);
            bind_member_function(Main, advance_to_room);
            bind_member_function(Main, set_run_weapon);
            bind_member_function(Main, get_run_weapon);
            bind_member_function(Main, return_to_title);
            signal_binding<Main, event::signal_example>::add<double>();
            signal_binding<Main, event::run_victory>::add<>();
            signal_binding<Main, event::room_advance_requested>::add<int, int, godot::String>();
        }

        [[signal_slot]] void on_room_cleared(int room_index);
        [[signal_slot]] void on_run_restart();
        [[signal_slot]] void on_level_state_changed(int state);

    private:
        void load_room(int room_index, int carry_hearts = -1);
        void bind_active_level_signals();
        void unload_active_level();
        [[nodiscard]] RoomKind parse_room_kind(const godot::String& room_kind) const;

        double m_signal_timer{ 0.0 };
        int m_room_index{ 0 };
        godot::String m_weapon_id{ "pulse" };
        std::vector<RunRoomEntry> m_run_rooms{};
        godot::CanvasLayer* m_canvas_layer{ nullptr };
        godot::SubViewport* m_game_viewport{ nullptr };
        MainDialog* m_main_dialog{ nullptr };
        Level* m_active_level{ nullptr };
    };
}
