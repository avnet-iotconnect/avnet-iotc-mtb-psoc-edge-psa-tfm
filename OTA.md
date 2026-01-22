# OTA Support

#### Enable OTA support

- In the file *mtb_shared\ota-update\release-tag\include\cy_ota_api.h*,
increase the buff sizes below for the long HTTPS OTA URL:

```c
    #define CY_OTA_MQTT_FILENAME_BUFF_SIZE          (1600)
    #define CY_OTA_HTTP_FILENAME_SIZE               (1600)
```

```c
    #define CY_OTA_JSON_DOC_BUFF_SIZE               (2048)
```

- In the file *proj_cm33_ns/ota.mk*, set the OTA flag below.
Alternatively, create an Environment Variable "OTA_SUPPORT" and set its value to "1". After that, restart the IDE.

```c
    OTA_SUPPORT?=1
```

#### Create an OTA firmware

- In the file *common.mk* (in the root project directory), change the image type from BOOT to UPDATE

```makefile
    IMG_TYPE?=UPDATE  // change from BOOT to UPDATE to make an OTA update build
```

- In the file *proj_cm33_ns/app_task.c*, increase the #define APP_VERSION_BASE,
so that APP_VERSION_BASE can be used to verify if the OTA update is successful or not.

- Build the project. The OTA .tar file will be generated in the project folder /build.

#### /IOTCONNECT settings for OTA

- Log into your IoTConnect account, *Settings -> Configurations*
and ensure that the toggle for *Firmware Configuration* is enabled.

- Click on *Firmware Configuration*, on the right panel:

    - Add *.tar* to **File Extensions**. Separate each file extension type with comma. 
    - Set **Maximum File Size** to at least 5MB. Use the dropdown to select the file size unit. Then, click **Save**.
    - Set **OTA URL Expiry** to at least 25 (minutes). Then, click **Save**.

#### /IOTCONNECT issue an OTA update

- Navigate to the side menu, *Devices -> Device*. On the bottom panel, click on "Firmware". 
- Click on "Create Firmware" on the top right. Input the **Name**,
select your device's **Template**, input the **Version**,
and upload the **File** by browsing to the OTA .tar file. Then, click **Save**. 
- Search for the firmware name you just created. You could use the filter to search for it.
- In the **Software Upgrades** column, click on the number under "Draft". 
- Click on the green button "Test OTA" in **Actions** column. It will jump to a new browser tab. 
- Ensure that you board is connected to /IOTCONNECT. 
- Select you device in **Devices**, then **Save**. 
- Click the **Update** button to issue an OTA update.
It will take around 5 minutes and after that, the board will reboot itself.

