/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com>, Shu Liu <shu.liu@avnet.com> et al.
 */

#ifndef APP_CONFIGURATOR_TASK_H_
#define APP_CONFIGURATOR_TASK_H_ 

// This value should be large enougth to hold the entire JSON configuration pasted by user
#ifndef APP_CONFIGURATOR_MAX_INPUT_LEN
#define APP_CONFIGURATOR_MAX_INPUT_LEN    (512U)
#endif

#ifndef APP_CONFIGURATOR_TASK_PRIORITY
#define APP_CONFIGURATOR_TASK_PRIORITY    (2U)
#endif

#ifndef APP_CONFIGURATOR_TASK_STACK_SIZE
#define APP_CONFIGURATOR_TASK_STACK_SIZE  (1024U * 2U + APP_CONFIGURATOR_MAX_INPUT_LEN) // input buff will be on stack
#endif

void app_configurator_task(void * param);

// Call this first, or explicitly start the task with FreeRTOS
void app_configurator_task_start(void);

// Blocking call. If the configurator is in progress, wait until it is done
void app_configurator_wait_if_in_progress(void);

#endif // APP_CONFIGURATOR_TASK_H_
