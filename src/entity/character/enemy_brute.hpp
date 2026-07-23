#pragma once

#include "entity/character/enemy.hpp"
#include "util/bind.hpp"

namespace rl
{
    /** Day6：慢而肉坦克敌人（高血量、低移速）。 */
    class EnemyBrute : public Enemy
    {
        GDCLASS(EnemyBrute, Enemy);

    public:
        EnemyBrute();

    protected:
        static void _bind_methods()
        {
        }
    };
}
