#include "core/constants.hpp"
#include "entity/character/enemy_brute.hpp"
#include "util/scene.hpp"

namespace rl
{
    EnemyBrute::EnemyBrute()
        : Enemy()
    {
        scene::node::set_unique_name(this, name::character::enemy_brute);
        m_health.set_max(combat::enemy_brute_hearts);
        m_movement_speed = combat::enemy_brute_movement_speed;
    }
}
