#include "core/constants.hpp"
#include "entity/character/enemy_boss.hpp"
#include "util/scene.hpp"

namespace rl
{
    EnemyBoss::EnemyBoss()
        : Enemy()
    {
        scene::node::set_unique_name(this, name::character::enemy_boss);
        m_health.set_max(combat::enemy_boss_hearts);
        m_movement_speed = combat::enemy_boss_movement_speed;
    }
}
