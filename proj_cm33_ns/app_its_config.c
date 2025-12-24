/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com>, Shu Liu <shu.liu@avnet.com> et al.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "psa/internal_trusted_storage.h"
#include "iotconnect.h"
#include "app_its_config.h"

typedef struct AppEepromData {
    uint32_t version; // sanity check to determine whether the data is stored or not
    int32_t platform; // IotConnectConnectionType
    char duid[IOTCL_CONFIG_DUID_MAX_LEN + 1];
    char cpid[IOTCL_CONFIG_CPID_MAX_LEN + 1];
    char env[IOTCL_CONFIG_ENV_MAX_LEN + 1];
    char wifi_ssid[WIFI_SSID_LEN + 1];
    char wifi_pass[WIFI_PASS_LEN + 1];
} device_config_data_t;

static device_config_data_t app_its_data = {0};

static const char* get_str_value_or_default(const char * value, const char* default_value) {
	if (!value || 0 == strlen(value)) {
		return default_value;
	}
	return value;
}

bool app_its_config_is_valid(void) {
    bool valid = false;
    valid = app_its_data.version == APP_DATA_VERSION;
    return valid;
}

static int app_its_config_set_value(const char* name, char* target, const char* value, size_t target_max_len) {    
    if (!value){
        printf("ERROR: APP ITS: Invalid value for %s\n", name);
        return IOTCL_ERR_BAD_VALUE;
    }
    
    if (strlen(value) >= target_max_len) {
        printf("ERROR: APP ITS: Value for %s is too long (max %d)\n", name, (int)(target_max_len - 1));
        return IOTCL_ERR_BAD_VALUE;
    }
    strcpy(target, value);
    app_its_data.version = APP_DATA_VERSION; // someone is setting a value, mark data as valid so that it can be retrieved
    return IOTCL_SUCCESS;
}

int app_its_config_set_platform(IotConnectConnectionType platform) {
    app_its_data.version = APP_DATA_VERSION; // someone is setting a value, mark data as valid so that it can be retrieved
    app_its_data.platform = (int32_t) platform;
    return IOTCL_SUCCESS;
}

int app_its_config_set_platform_as_string(const char* platform_str) {
    if (strcmp(platform_str, IOTCL_PF_AWS_STR) == 0) {
        app_its_data.platform = (int32_t) IOTC_CT_AWS;
        app_its_data.version = APP_DATA_VERSION; // someone is setting a value, mark data as valid so that it can be retrieved
        return IOTCL_SUCCESS;
    } else if (strcmp(platform_str, IOTCL_PF_AZURE_STR) == 0) {
        app_its_data.platform = (int32_t) IOTC_CT_AZURE;
        app_its_data.version = APP_DATA_VERSION; // someone is setting a value, mark data as valid so that it can be retrieved
        return IOTCL_SUCCESS;
    } else {
        printf("ERROR: APP ITS: Invalid platform string: %s\n", platform_str);
        return IOTCL_ERR_BAD_VALUE;
    }
}


int app_its_config_set_duid(const char* value) {
    return app_its_config_set_value("duid", app_its_data.duid, value, sizeof(app_its_data.duid));
}
int app_its_config_set_cpid(const char* value) {
    return app_its_config_set_value("cpid", app_its_data.cpid, value, sizeof(app_its_data.cpid));
}
int app_its_config_set_env(const char* value) {
    return app_its_config_set_value("env", app_its_data.env, value, sizeof(app_its_data.env));
}
int app_its_config_set_wifi_ssid(const char* value) {
    return app_its_config_set_value("wifi_ssid", app_its_data.wifi_ssid, value, sizeof(app_its_data.wifi_ssid));
}
int app_its_config_set_wifi_pass(const char* value) {
    return app_its_config_set_value("wifi_pass", app_its_data.wifi_pass, value, sizeof(app_its_data.wifi_pass));
}

const char* app_its_config_get_duid(const char* default_value) {
    if (!app_its_config_is_valid()) {
        return default_value;
    }
    return get_str_value_or_default(app_its_data.duid, default_value);
}
const char* app_its_config_get_cpid(const char* default_value) {
	if (!app_its_config_is_valid()) {
		return default_value;
	}
	return get_str_value_or_default(app_its_data.cpid, default_value);
}

const char* app_its_config_get_env(const char* default_value) {
	if (!app_its_config_is_valid()) {
		return default_value;
	}
	return get_str_value_or_default(app_its_data.env, default_value);
}

const char* app_its_config_get_wifi_ssid(const char* default_value) {
	if (!app_its_config_is_valid()) {
		return default_value;
	}
	return get_str_value_or_default(app_its_data.wifi_ssid, default_value);
}

const char* app_its_config_get_wifi_pass(const char* default_value) {
	if (!app_its_config_is_valid()) {
		return default_value;
	}
	return get_str_value_or_default(app_its_data.wifi_pass, default_value);
}

const char* app_its_config_get_platform_as_string(const char* default_value) {
	if (!app_its_config_is_valid()) {
		return default_value;
	}
	switch(app_its_data.platform) {
        case IOTC_CT_AWS:
            return IOTCL_PF_AWS_STR;
        case IOTC_CT_AZURE:
            return IOTCL_PF_AZURE_STR;
        default:
            return default_value;
    }
}

IotConnectConnectionType app_its_config_get_platform(IotConnectConnectionType default_value) {
    if (!app_its_config_is_valid()) {
        return default_value;
    }
    return (IotConnectConnectionType) app_its_data.platform;
}

int app_its_save_config_data(void) {
    psa_status_t status;

    app_its_data.version = APP_DATA_VERSION;

    status = psa_its_set(APP_DEVICE_CONFIG_ITS_UID, sizeof(device_config_data_t), &app_its_data, PSA_STORAGE_FLAG_NONE);
    if (status != PSA_SUCCESS) {
        printf("Failed to store device config in ITS slot %d: %d\n", (int) APP_DEVICE_CONFIG_ITS_UID, (int)status);
        return IOTCL_ERR_FAILED;
    }
    return IOTCL_SUCCESS;
}

int app_its_config_init(void) {
    psa_status_t status;
    size_t get_size = 0;

    status = psa_its_get(APP_DEVICE_CONFIG_ITS_UID, 0, sizeof(device_config_data_t), &app_its_data, &get_size);

    if (status == PSA_SUCCESS && get_size == sizeof(device_config_data_t) &&
        app_its_data.version == APP_DATA_VERSION) {
        printf("ITS: Device configuration found in ITS slot %d\n", (int) APP_DEVICE_CONFIG_ITS_UID);
        return IOTCL_SUCCESS;
    } else {
        printf("ITS: No valid device configuration found in ITS slot %d\n", (int) APP_DEVICE_CONFIG_ITS_UID);
        return IOTCL_ERR_CONFIG_MISSING;
    }
}

void app_its_config_clear(void) {
    psa_status_t status;

    memset(&app_its_data, 0, sizeof(app_its_data));    

    status = psa_its_remove(APP_DEVICE_CONFIG_ITS_UID);

    if (status != PSA_SUCCESS && status != PSA_ERROR_DOES_NOT_EXIST) {
        printf("ERROR: Failed to clear device config in ITS slot %d: %d\n", (int) APP_DEVICE_CONFIG_ITS_UID, (int)status);
        return;
    }
}

    

