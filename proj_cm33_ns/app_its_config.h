/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com>, Shu Liu <shu.liu@avnet.com> et al.
 */
#ifndef APP_ITS_CONFIG_H_
#define APP_ITS_CONFIG_H_

#include <stdbool.h>
#include "iotconnect.h"

#ifndef APP_DEVICE_CONFIG_ITS_UID
#define APP_DEVICE_CONFIG_ITS_UID    (7U)
#endif

// NOTE: String values will have +1 for null terminator in config data
// Use IOTCL_CONFIG_*_MAX_LEN defines from iotcl.h for DUID, CPID, ENV
#define APP_DATA_VERSION          0x32a9f200
#define PF_SIZE                            6
#define WIFI_SSID_LEN					  32
#define WIFI_PASS_LEN					  64

// call this early in the app to initialize ITS config
// and before starting the input handler task
int app_its_config_init(void);


IotConnectConnectionType app_its_config_get_platform(IotConnectConnectionType default_value);
const char* app_its_config_get_platform_as_string(const char* default_value);
const char* app_its_config_get_duid(const char* default_value);
const char* app_its_config_get_cpid(const char* default_value);
const char* app_its_config_get_env(const char* default_value);
const char* app_its_config_get_wifi_ssid(const char* default_value);
const char* app_its_config_get_wifi_pass(const char* default_value);


int app_its_config_set_platform(IotConnectConnectionType value);
int app_its_config_set_platform_as_string(const char* value);
int app_its_config_set_duid(const char* value);
int app_its_config_set_cpid(const char* value);
int app_its_config_set_env(const char* value);
int app_its_config_set_wifi_ssid(const char* value);
int app_its_config_set_wifi_pass(const char* value);

bool app_its_config_is_valid(void);

// Invoke to clear the stored config data
void app_its_config_clear(void);

// Save current app_its_data to ITS
int app_its_save_config_data(void);

#endif // APP_ITS_CONFIG_H_