/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com>, Shu Liu <shu.liu@avnet.com> et al.
 */

#ifndef PSA_CREDENTIALS_H
#define PSA_CREDENTIALS_H

#include "iotconnect.h"

void psa_mqtt_setup_huk(void);
void setup_iotconnect_sdk_credentials(IotConnectClientConfig* config);

#endif // PSA_CREDENTIALS_H
