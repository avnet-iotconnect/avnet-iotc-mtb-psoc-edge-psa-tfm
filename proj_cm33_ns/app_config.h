/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com>, Shu Liu <shu.liu@avnet.com> et al.
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "iotconnect.h"

// see IotConnectConnectionType: IOTC_CT_AWS or IOTC_CT_AZURE
#define IOTCONNECT_CONNECTION_TYPE IOTC_CT_UNDEFINED
#define IOTCONNECT_CPID "your-cpid"
#define IOTCONNECT_ENV  "your-env"

// If Device Unique ID (DUID) is not provided, a generated DUID will be used using the below prefix
#define IOTCONNECT_DUID "" 
// prefix for the auto-generated name based on chip HWUID that will be used if IOTCONNECT_DUID is not supplied
#define IOTCONNECT_DUID_PREFIX "psoc-edge-rm-"

// you can choose to use your own NTP server to obtain network time, or simply time.google.com for better stability
#define IOTCONNECT_SNTP_SERVER "pool.ntp.org"

#endif /* APP_CONFIG_H */
