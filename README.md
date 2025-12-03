# Project Setup

- Launch the BSP Configurator - In MTB Assistant, choose Application page -> Configure Device
  - Ensure that View->Parameters is enabled in the menus
  - Select the *Edge Protect Solution* in the list
  - At *Edge Protect Solution - Parameters*, in the right-side panel, select Launch Edge Protect Configurator.
  - Select TF-M Profile **Large**
  - Click the Save button, close the window and then File->Save in the menu of the Configurator App

Apply a fix to secure-soeckets by manually editing *mtb_shared/secure-sockets/release-v3.12.0/source/COMPONENT_MBEDTLS/cy_tls.c* around line 1667 and surround this code with *if(tls_identity)*:
    /* IOTC_FIX: Only setup opaque key if tls_identity is provided (mTLS case).
     * For server-only TLS (no client cert), tls_identity is NULL and we skip this. */
    if(tls_identity)
    {
        mbedtls_pk_init( &tls_identity->private_key );

        /* Set the opaque private key id */
        ret = mbedtls_pk_setup_opaque(&tls_identity->private_key, tls_identity->opaque_private_key_id);
        if(ret != 0)
        {
            tls_cy_log_msg(CYLF_MIDDLEWARE, CY_LOG_ERR, "Failed to setup opaque key context 0x%x\r\n", ret);
            result = CY_RSLT_MODULE_TLS_ERROR;
            goto cleanup;
        }
    }


When launching for the frst time, the code will generate a certificate. 
Use that printed cert to register to AWS IoTCore or IoTConnect AWS trial account (use your own certificate when creating devices). 
The same certificate will be printed upon subsequent reboots.
A key derived from HUK will be used for Mutual TLS.

Set your Wi-Fi credentials and MQTT credentials at [proj_cm33_ns/mqtt_client_config.h](proj_cm33_ns/mqtt_client_config.h) accordingly.
Re-building and launching the project should connect the device to the MQTT server.
