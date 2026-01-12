# OTA Support

OTA support for the project needs the steps below.

1. *mtb_shared\ota-update\release-tag\include\cy_ota_api.h*
(not doing this causes a crash due to the long S3 signature in the URL path)
```c
    #define CY_OTA_MQTT_FILENAME_BUFF_SIZE          (1600)
    #define CY_OTA_HTTP_FILENAME_SIZE               (1600)
```

1. *mtb_shared\ota-update\release-tag\include\cy_ota_api.h* 
(Increase json buff size. Even though we don't use this, it generates warnings in code)
```c
    #define CY_OTA_JSON_DOC_BUFF_SIZE               (2048)
```


