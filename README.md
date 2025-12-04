# Project Setup

- Program and launch the application

Once the project has been created, 
apply a fix to secure-soeckets by manually editing 
*mtb_shared/secure-sockets/release-v3.12.0/source/COMPONENT_MBEDTLS/cy_tls.c* around line 1667 
and surround this code with *if(tls_identity)*:
```c
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
```

Set your Wi-Fi credentials at [proj_cm33_ns/wifi_config.h](proj_cm33_ns/wifi_config.h).
Set your IOTCONNECT_ENV and IOTCONNECT_CPID at [proj_cm33_ns/wifi_config.h](proj_cm33_ns/wifi_config.h) 
per your /IOTCONNECT account.
If IOTCONNECT_DUID is not set, the device will generate a unique DUID and will print it during startup.

When launching for the frst time, the code will derive a private key based on the immutable 
Hardware Unique Key (HUK) that is provisioned on the board,
and generate a certificate that will be stored in PSA Internal Trusted Storage (ITS).
The same certificate will be retrieved and printed upon subsequent reboots.

Use the board certificate and your device DUID to register the device with /IOTCONNECT. 
Ensure to use the *Use your own certificate* option when creating devices.

Once the device is registered in /IOTCONNECT,
restart the device in order to have it connect.
