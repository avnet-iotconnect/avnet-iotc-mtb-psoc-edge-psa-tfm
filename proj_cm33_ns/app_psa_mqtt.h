/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com>, Shu Liu <shu.liu@avnet.com> et al.
 */

#ifndef APP_PSA_MQTT_H
#define APP_PSA_MQTT_H

#include "iotconnect.h"

// Ensure this does not conflict with APP_DEVICE_CONFIG_ITS_UID
#ifndef APP_PSA_CERT_ITS_UID
#define APP_PSA_CERT_ITS_UID (8U)
#endif

// call this early to setup HUK-based key and derived cert
void app_psa_mqtt_setup_huk(void); 

// retrieve PEM-encoded certificate for printing
const char* app_psa_mqtt_get_certificate(void);

// call this to setup IotConnectClientConfig with PSA-based credentials
void app_psa_mqtt_setup_sdk_credentials(IotConnectClientConfig* config);

#endif // APP_PSA_MQTT_H
