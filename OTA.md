# OTA Support

OTA support for the project needs the steps below.

1. *mtb_shared\ota-update\release-tag\include\cy_ota_api.h*
(not doing this causes a crash due to the long S3 signature in the URL path)
```c
    #define CY_OTA_MQTT_FILENAME_BUFF_SIZE          (1600)
    #define CY_OTA_HTTP_FILENAME_SIZE               (1600)
```
1. *mtb_shared\avnet-iotc-mtb-sdk\release-tag\source\iotc_mqtt_client.c*
(not doing this causes a disconnect when OTA is sent)
```c
    #define MQTT_NETWORK_BUFFER_SIZE          (4096U)
```
1. Add SNI-relates lines to the SDK mtb_shared\avnet-iotc-mtb-sdk\release-tag\source\iotc_ota.c:
```c
	ota_network_params.http.file = iotcl_strdup(path);
	ota_network_params.http.server.host_name = iotcl_strdup(host);
	ota_network_params.http.credentials.sni_host_name = ota_network_params.http.server.host_name;
	ota_network_params.http.credentials.sni_host_name_size = strlen(ota_network_params.http.server.host_name) + 1; // with null terminator
    ....
```


