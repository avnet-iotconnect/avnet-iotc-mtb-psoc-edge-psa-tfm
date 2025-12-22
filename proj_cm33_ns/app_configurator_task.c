/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com>, Shu Liu <shu.liu@avnet.com> et al.
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "tfm_platform_api.h"

#include "iotcl.h"
#include "iotcl_dra_json_config.h"
#include "app_its_config.h"
#include "app_psa_mqtt.h"
#include "app_io.h"
#include "app_configurator_task.h"
#include "app_config.h"

static volatile bool s_configurator_in_progress = false;

static void print_device_intro(void) {
    const char* psa_cert = app_psa_mqtt_get_certificate();
    if (psa_cert && strlen(psa_cert) > 0) {
        app_io_write_str_crlf("This device certificate:");
        // write cert line by line to avoid large buffer issues
        size_t cert_len = strlen(psa_cert);
        size_t offset = 0;
        while (offset < cert_len) {
            // cert will have '\n' line endings (not \r\n or anything else)
            // find next newline or end of string and print that line
            size_t line_len = 0;
            while (psa_cert[offset + line_len] != '\n' && psa_cert[offset + line_len] != '\0') {
                line_len++;
            }
            if (line_len > 0) {
                app_io_write_data_crlf(&psa_cert[offset], line_len);
            }
            offset = offset + line_len + 1; // skip newline or null
        }
    } else {
        app_io_write_str_crlf("ERROR: Expected a generated device certificate.");
        printf("ERROR: Expected a generated device certificate!\n");
    }
    app_io_write_str_crlf("");
    char unique_id[sizeof(uint32_t) * 2 + 1] = {0}; // 8 hex digits + null terminator
    uint64_t hwuid = Cy_SysLib_GetUniqueId();
    uint32_t hwuidhi = (uint32_t)(hwuid >> 32);
    // not using low bytes in the name because they appear to be the same across all boards of the same type
    // feel free to modify the application to use these bytes
    // uint32_t hwuidlo = (uint32_t)(hwuid & 0xFFFFFFFF);
    sprintf(unique_id, "%08lx", (unsigned long) hwuidhi);
    app_io_write_str("Suggested uniquely generated device ID: ");
    app_io_write_str(IOTCONNECT_DUID_PREFIX);
    app_io_write_str_crlf(unique_id);
    app_io_write_str_crlf("");
}

void app_configurator_wait_if_in_progress(void) {
    bool was_in_progress = s_configurator_in_progress;
    if (was_in_progress) {
        printf("Application: Device configuration is in progress, waiting ...\n");
    }
    while (s_configurator_in_progress) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    if (was_in_progress) {
        printf("Application: Device configuration completed, resuming...\n");
    }
}

void app_configurator_task(void * param) {
    (void) param;

    bool had_existing_config = app_its_config_is_valid();

    char input_buffer[APP_CONFIGURATOR_MAX_INPUT_LEN];

    printf("App Configurator Task: Started\n");
    if (app_io_init() != 0) {
        printf("App Configurator Task: Failed to initialize IO\n");
        vTaskDelete(NULL);
        return;
    }
    app_io_write_str_crlf("Press ENTER to begin the device configuration");
    app_io_read_lines(input_buffer, sizeof(input_buffer), false);

    app_io_write_str_crlf("");
    app_io_write_str_crlf("");
    app_io_write_str_crlf("Welcome to the /IOTCONNECT Device Configurator.");
    print_device_intro();

    s_configurator_in_progress = true;

    if (had_existing_config) {
        app_io_write_str_crlf("Settings are already stored. Current settings:");
        app_io_write_str_crlf("");        
        app_io_write_str("WiFi SSID:     ");
        app_io_write_str_crlf(app_its_config_get_wifi_ssid("[not set]"));

        const char * pass = app_its_config_get_wifi_pass(NULL);
        if (!pass || strlen(pass) == 0) {
            app_io_write_str_crlf("WiFi Password: [open network]");
        } else {
            app_io_write_str_crlf("WiFi Password: ******");
        }

        app_io_write_str("Platform:      ");
        app_io_write_str_crlf(app_its_config_get_platform_as_string("[not set]"));
        app_io_write_str("ENV:           ");
        app_io_write_str_crlf(app_its_config_get_env("[not set]"));
        app_io_write_str("CPID:          ");
        app_io_write_str_crlf(app_its_config_get_cpid("[not set]"));
        app_io_write_str("DUID:          ");
        app_io_write_str_crlf(app_its_config_get_duid("[not set]"));
        app_io_write_str_crlf("");

        // we want the user to explicitly confirm clearing the config before proceeding.
        // It is complicated to re-use the password etc., so just restart the configurator.
        while (true) {
            app_io_write_str_crlf("Existing configuration must be cleared before proceeding.");
            app_io_write_str_crlf("Do you want to clear the existing configuration? (y/N): ");
            app_io_read_lines(input_buffer, sizeof(input_buffer), false);
            if (strlen(input_buffer) != 0 && (input_buffer[0] == 'Y' || input_buffer[0] == 'y')) {
                app_its_config_clear();
                app_io_write_str_crlf("Configuration cleared.");
                app_io_write_str_crlf("");
                break;
            }
        }
    }

    app_io_write_str_crlf("NOTE: Do not use BACKSPACE as it will lead to unexpected behavior.");

    do {
        
        app_its_config_get_wifi_ssid(input_buffer);

        app_io_write_str("Enter WiFi SSID: ");
        
        app_io_read_lines(input_buffer, sizeof(input_buffer), false);
        if (strlen(input_buffer) == 0) {
                app_io_write_str_crlf("WiFi SSID cannot be empty, please try again.");
        } else {
            if (IOTCL_SUCCESS != app_its_config_set_wifi_ssid(input_buffer)) {
                app_io_write_str_crlf("ERROR: Invalid WiFi SSID, please try again.");
                printf("ERROR: Invalid WiFi SSID input: %s\n", input_buffer);
                continue; // we will continue the loop below, but just in case...
            }
        }
    } while (NULL == app_its_config_get_wifi_ssid(NULL));

    // NOTE: No real easy way to combine "ENTER", considering defaults or validation when open network etc.
    // Just force re-entering the password each time and accept anything entered.
    app_io_write_str("Enter WiFi Password: ");
    app_io_start_password_masking();
    app_io_read_lines(input_buffer, sizeof(input_buffer), false);
    app_io_stop_password_masking();
    app_its_config_set_wifi_pass(input_buffer);
    
    int parsing_result;
    
    app_io_write_str_crlf("Device configuration can be found at your /IOTCONNECT device's Info pannel.");
    do {
        IotclDraJsonConfigResult json_config_result = {0};

        app_io_write_str_crlf("Copy and paste the Device JSON configuration (iotcDeviceConfig.json).");
        app_io_write_str_crlf("When done, press CTRL+D:");
        app_io_read_lines(input_buffer, sizeof(input_buffer), true);
        parsing_result = iotcl_dra_json_config_parse(&json_config_result, input_buffer);
        parsing_result = iotcl_dra_json_config_parse(&json_config_result, input_buffer);
        if (parsing_result == IOTCL_SUCCESS) {
            app_io_write_str("Platform: ");
            app_io_write_str_crlf(json_config_result.pf);
            app_io_write_str("ENV:      ");
            app_io_write_str_crlf(json_config_result.env);
            app_io_write_str("CPID:     ");
            app_io_write_str_crlf(json_config_result.cpid);
            app_io_write_str("DUID:     ");
            app_io_write_str_crlf(json_config_result.duid);
            
            app_its_config_set_platform_as_string(json_config_result.pf);
            app_its_config_set_env(json_config_result.env);
            app_its_config_set_cpid(json_config_result.cpid);
            app_its_config_set_duid(json_config_result.duid);

            iotcl_dra_json_config_free(&json_config_result);
            app_io_write_str_crlf("/IOTCONNECT device configuration applied successfully.");
            printf("/IOTCONNECT device configuration applied successfully.");  // const string re-use. \n after
            printf("\n");
        } else {
            app_io_write_str_crlf("Configuration parsing failed. Please try again. Ensure to paste the entire JSON.");
            printf("Configuration failed with error: %d.\n", parsing_result);
        }
    } while (parsing_result != IOTCL_SUCCESS);

    if (app_its_save_config_data() != IOTCL_SUCCESS) {
        app_io_write_str_crlf("ERROR: Failed to store device configuration.");
        printf("ERROR: Failed to store device configuration.\n");
        vTaskDelete(NULL);
        return;
    }

    app_io_write_str_crlf("");
    app_io_write_str_crlf("Device configuration complete. Resetting device...");
    app_io_write_str_crlf("");
    printf("\n");
    printf("Device configuration complete. Resetting device..."); // const string re-use \n after
    printf("\n");
    vTaskDelay(pdMS_TO_TICKS(2000));

    // NOTE: NVIC_SystemReset() does not work, as it is not allowed in TF-M NS context.
    enum tfm_platform_err_t result = tfm_platform_system_reset(); // unclear here on return type
    if (result != TFM_PLATFORM_ERR_SUCCESS) {
        app_io_write_str_crlf("ERROR: Failed to reset device. Please reset manually.");
        printf("ERROR: Failed to reset device. Error was: %d.\n", (int) result);
    }
    
    if (!had_existing_config) {
        printf("The board will now reset to apply the new configuration.\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
        tfm_platform_system_reset();
    }

    s_configurator_in_progress = false;
    vTaskDelete(NULL);
}

void app_configurator_task_start(void) {
    BaseType_t result;

    result = xTaskCreate(app_configurator_task, "App Config", APP_CONFIGURATOR_TASK_STACK_SIZE,
                NULL, APP_CONFIGURATOR_TASK_PRIORITY, NULL);
    if (result != pdPASS) {
        printf("ERROR: Failed to create App Configurator Task\n");
    }
}