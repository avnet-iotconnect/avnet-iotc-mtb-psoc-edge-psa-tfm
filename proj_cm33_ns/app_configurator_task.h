#ifndef APP_CONFIGURATOR_TASK_H_
#define APP_CONFIGURATOR_TASK_H_ 
/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com>, Shu Liu <shu.liu@avnet.com> et al.
 */

#ifndef APP_CONFIGURATOR_MAX_INPUT_LEN
#define APP_CONFIGURATOR_MAX_INPUT_LEN    (512U)
#endif

#ifndef APP_CONFIGURATOR_TASK_PRIORITY
#define APP_CONFIGURATOR_TASK_PRIORITY    (2U)
#endif

#ifndef APP_CONFIGURATOR_TASK_STACK_SIZE
#define APP_CONFIGURATOR_TASK_STACK_SIZE  (1024U * 4U + APP_CONFIGURATOR_MAX_INPUT_LEN) // input buff will be on stack
#endif


void app_configurator_task(void * param);
void app_configurator_task_start(void);
void app_configurator_wait_if_in_progress(void);

#endif // APP_CONFIGURATOR_TASK_H_
