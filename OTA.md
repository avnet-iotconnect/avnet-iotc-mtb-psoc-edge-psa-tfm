# OTA Support

OTA support for the project needs the steps below.

* After creating a Avnet's PSOC Edge project, you will need to import the [Edge protect bootloader project](https://github.com/Infineon/mtb-example-psoc-control-edge-protect-bootloader) by clicking the button "Add bootloader" in the ModusToolbox Assistant's Application page or copy the bootloader project manually by following steps 2 to 9 in the Operation section of the Edge Protect Bootloader code example's [README.md](https://github.com/Infineon/mtb-example-edge-protect-bootloader/blob/master/README.md).

* Add the libs below for ota into the **proj_cm33_ns** project using *Tool: Library Manager*.
    1. ota-update (release v4.5.1).
    1. ota-bootloader-abstraction (release v1.8.0).
    1. serial-memory (release v3.1.0)

* Copy the *templates* folder from the [Edge protect bootloader project](https://github.com/Infineon/mtb-example-edge-protect-bootloader) into the project. Copy the file *templates/yourboard/design.modus* to the project's directory */bsps/config* and overwrite the existing one. Open ModusToolbox Assistant's Application page and run "Configure Device". In the pop-up application, Click top menu File->Save, you will see the "Code Generated" in green color on the bottom right corner, then close the application.

* File changes needed are listed below:
    1. *mtb_shared\ota-update\release-tag\include\cy_ota_api.h*
    (not doing this causes a crash due to the long S3 signature in the URL path)
    ```
        #define CY_OTA_MQTT_FILENAME_BUFF_SIZE          (1600)
        #define CY_OTA_HTTP_FILENAME_SIZE               (1600)
    ```
    1. *mtb_shared\avnet-iotc-mtb-sdk\release-tag\source\iotc_mqtt_client.c*
    (not doing this causes a disconnect when OTA is sent)
    ```
        #define MQTT_NETWORK_BUFFER_SIZE          (4096U)
    ```

* Add or replace the files below:
    1. proj/common.mk
    1. proj/configs/update_with_epb_overwrite.json
    1. proj_cm33_ns/Makefile
    1. proj_cm33_ns/ota.mk
    1. proj_cm33_ns/app_task.c
    1. proj_cm33_ns/cy_ota_config.h
    1. proj_cm33_ns/cy_ota_flash.c
    1. proj_cm33_ns/mbedtls_config.h
    1. proj_cm33_ns/mbedtls_user_config.h
