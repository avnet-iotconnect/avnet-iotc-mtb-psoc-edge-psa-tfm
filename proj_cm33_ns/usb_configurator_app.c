/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com>, Shu Liu <shu.liu@avnet.com> et al.
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "iotcl.h"
#include "tfm_ns_interface.h"
#include "psa/internal_trusted_storage.h"
#include "iotcl_dra_json_config.h"

static char input_buffer[512];
static IotclDraJsonConfigResult json_config_result = {0};

void app_usb_configurator_on_data_available(char * write_buffer, int num_bytes_to_write) {
    strncat(input_buffer, write_buffer, num_bytes_to_write);
    if (IOTCL_SUCCESS == iotcl_dra_json_config_parse(&json_config_result, input_buffer)) {
        printf("A OK\n");
        printf("cpid: %s\n", json_config_result.cpid);
    }
}

