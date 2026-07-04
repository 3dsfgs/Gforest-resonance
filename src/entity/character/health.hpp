#pragma once

#include <algorithm>

namespace rl
{
    /** Discrete heart-based health used by Character (P0-B01). */
    struct HeartHealth
    {
        int max_hearts{ 5 };
        int hearts{ 5 };

        [[nodiscard]] bool is_alive() const
        {
            return hearts > 0;
        }

        void reset()
        {
            hearts = max_hearts;
        }

        void set_max(const int max)
        {
            max_hearts = std::max(1, max);
            hearts = std::min(hearts, max_hearts);
        }

        /** @return hearts actually lost (0 if already dead). */
        int apply_damage(const int amount = 1)
        {
            if (!is_alive() || amount <= 0)
                return 0;

            const int lost = std::min(amount, hearts);
            hearts -= lost;
            return lost;
        }
    };
}
