#pragma once

#include <godot_cpp/classes/control.hpp>

#include "core/constants.hpp"
#include "entity/character/player.hpp"
#include "util/bind.hpp"

namespace rl::inline ui
{
    class HeartHud : public godot::Control
    {
        GDCLASS(HeartHud, godot::Control);

    public:
        HeartHud() = default;
        ~HeartHud() = default;

        void _ready() override;
        void _process(double delta_time) override;
        void _draw() override;

        void connect_to_player();
        void disconnect_from_player();
        void update_hearts(int current, int max_hearts);

    protected:
        [[signal_slot]] void on_player_hearts_changed(int current, int max_hearts);
        [[signal_slot]] void on_player_died();

        static void _bind_methods()
        {
            bind_member_function(HeartHud, connect_to_player);
            bind_member_function(HeartHud, disconnect_from_player);
            bind_member_function(HeartHud, on_player_hearts_changed);
            bind_member_function(HeartHud, on_player_died);
            bind_member_function(HeartHud, update_hearts);
        }

    private:
        void update_layout_size();

        Player* m_player{ nullptr };
        int m_current_hearts{ 5 };
        int m_max_hearts{ 5 };
        double m_dash_ready_ratio{ 1.0 };
        double m_polar_slash_ready_ratio{ 1.0 };
        double m_energy_light_ready_ratio{ 1.0 };
    };
}
