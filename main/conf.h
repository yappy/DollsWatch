#ifndef CONF_H_
#define CONF_H_

#include <stdint.h>

const uint32_t MAIN_TASK_STACK_SIZE = 8 * 1024;
const uint32_t MAIN_TASK_CORE = 1;
// (low) 0.. (configMAX_PRIORITIES - 1) (high)
const uint32_t MAIN_TASK_PRIORITY = 1;
const uint32_t MAIN_LOOP_SLEEP = 100;

#endif // CONF_H_
