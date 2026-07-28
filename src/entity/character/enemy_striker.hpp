#pragma once

#include "entity/character/enemy.hpp"
#include "util/bind.hpp"
#include "util/combat_feedback.hpp"

namespace rl
{
    /** ⭐3：影刺——高速贴脸近战，无远程。 */
    class EnemyStriker : public Enemy
    {
        GDCLASS(EnemyStriker, Enemy);

    public:
        EnemyStriker();

        [[nodiscard]] combat_feedback::KillVfxKind kill_vfx_kind() const override
        {
            return combat_feedback::KillVfxKind::Striker;
        }

    protected:
        static void _bind_methods()
        {
        }
    };
}
