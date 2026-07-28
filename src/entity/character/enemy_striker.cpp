#include "core/constants.hpp"
#include "entity/character/enemy_striker.hpp"
#include "util/scene.hpp"

namespace rl
{
    EnemyStriker::EnemyStriker()
        : Enemy()
    {
        scene::node::set_unique_name(this, name::character::enemy_striker);
        m_health.set_max(combat::enemy_striker_hearts);
        m_movement_speed = combat::enemy_striker_movement_speed;
    }
}
