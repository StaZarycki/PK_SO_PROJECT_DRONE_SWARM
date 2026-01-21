#include "operator_manager.h"
#include "drone_manager.h"

Base base;

/**
 * @brief Initializes the operator's base.
 *
 * Initializes the operator's base with 75 max drones, 0 current drones,
 * 1 platform, and two empty passages.
 *
 * @return 0 on success.
 */
int init()
{
  base.max_drones = 75;
  base.current_drones = 0;
  base.platforms = 1;
  base.passages[0] = 0;
  base.passages[1] = 0;

  return 0;
}

/**
 * @brief Spawn drones to reach the maximum allowed number.
 *
 * Spawns drones until the maximum number of drones allowed by the
 * operator is reached. The number of drones spawned is the difference
 * between the maximum number of drones and the current number of drones.
 *
 * @return 0 on success.
 */
int add_drones(void)
{
  spawn_drones(base.max_drones - base.current_drones);

  return 0;
}

/**
 * @brief Increase the maximum number of drones that can be controlled by the operator.
 *
 * The maximum number of drones that can be controlled by the operator is
 * doubled.
 *
 * @return 0 on success.
 */
int add_platform(void)
{
  base.max_drones *= 2;

  return 0;
}

/**
 * @brief Decrease the maximum number of drones that can be controlled by the operator.
 *
 * The maximum number of drones that can be controlled by the operator is
 * halved.
 *
 * @return 0 on success.
 */
int remove_platform(void)
{
  base.max_drones /= 2;

  return 0;
}

/**
 * @brief Get the status of a passage.
 *
 * @param n 0 or 1, corresponding to the first or second passage.
 * @return 0 if the passage is empty, 1 if it is occupied.
 */
int check_passage(int n)
{
  return base.passages[n];
}
