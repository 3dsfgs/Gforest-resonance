#pragma once

#include "entity/character/enemy.hpp"
#include "util/bind.hpp"
#include "util/combat_feedback.hpp"

namespace rl
{
    /** P2-4：心魔 Boss——高血量，特殊攻击由 HeartDemon AI 驱动。 */
    class EnemyBoss : public Enemy
    {
        GDCLASS(EnemyBoss, Enemy);

    public:
        EnemyBoss();

        [[nodiscard]] combat_feedback::KillVfxKind kill_vfx_kind() const override
        {
            return combat_feedback::KillVfxKind::Boss;
        }

    protected:
        static void _bind_methods()
        {
        }
    };
}
