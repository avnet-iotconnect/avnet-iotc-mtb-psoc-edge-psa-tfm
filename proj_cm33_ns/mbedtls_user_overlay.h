#ifndef MBEDTLS_USER_OVERLAY_HEADER
#define MBEDTLS_USER_OVERLAY_HEADER

#include "configs/mbedtls_user_config.h"

// We apply our overrides here:
// Certificate creation and handling for self-signed cert generation
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_X509_CREATE_C
#define MBEDTLS_X509_CRT_WRITE_C

// PSA-based key data: Store EC keys as PSA opaque keys (PSA key handles instead of raw key material)
// This ensures private keys are never exposed in user-space memory
#define MBEDTLS_PK_USE_PSA_EC_DATA

// These should be defacto for PSA. 
// From mtb_shared\wifi-core-freertos-lwip-mbedtls\release-v3.1.0\configs\mbedtls_user_config.h:
// NOTE: 25519 could have issues with some HTTPS servers. Migth be good to remove it as well if it is active.
#undef MBEDTLS_ECP_DP_SECP192R1_ENABLED
#undef MBEDTLS_ECP_DP_SECP224R1_ENABLED
// we will use this for the MQTT mutual TLS with ECC
//#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#undef MBEDTLS_ECP_DP_SECP384R1_ENABLED
#undef MBEDTLS_ECP_DP_SECP521R1_ENABLED
#undef MBEDTLS_ECP_DP_SECP192K1_ENABLED
#undef MBEDTLS_ECP_DP_SECP224K1_ENABLED
#undef MBEDTLS_ECP_DP_SECP256K1_ENABLED
#undef MBEDTLS_ECP_DP_BP256R1_ENABLED
#undef MBEDTLS_ECP_DP_BP384R1_ENABLED
#undef MBEDTLS_ECP_DP_BP512R1_ENABLED
// This is likely needed for RSA TLS 1.3 with /IOTCONNECT discovery/identity servers
// but we coudldn't get it to work with PSA + RSA private keys
// It want Montgomery with AWS, and we don't have it enabled in the PSA profile
#undef MBEDTLS_ECP_DP_CURVE25519_ENABLED
#undef MBEDTLS_ECP_DP_CURVE448_ENABLED

// needed for TLS v1.3:
#define PSA_WANT_ALG_HKDF_EXTRACT               1
#define PSA_WANT_ALG_HKDF_EXPAND                1
// or:
// #undef MBEDTLS_SSL_PROTO_TLS1_3


// in reality MBEDTLS_SSL_PROTO_TLS1_3 can work with MQTT, but HTTP client seems to have issues
#undef MBEDTLS_SSL_PROTO_TLS1_3

#ifdef MBEDTLS_SSL_PROTO_TLS1_3
// TLS 1.3 related defines
// #define MBEDTLS_SSL_TLS1_3_COMPATIBILITY_MODE
#define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE
#define MBEDTLS_HKDF_C
#define MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_EPHEMERAL_ENABLED
#define MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_PSK_EPHEMERAL_ENABLED
#endif // MBEDTLS_SSL_PROTO_TLS1_3

// Ensure ECDSA is supported along with our key SECP256R1 curve
#define MBEDTLS_ECDSA_C
#define MBEDTLS_ECDSA_DETERMINISTIC_ENABLED
#define MBEDTLS_ECP_C
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED


// Force ECC-only cipher suites to avoid RSA operations entirely
// Enable ECC-based key exchanges
#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDH_ECDSA_ENABLED

// ECDH support - required for ECDHE key exchange (both ECDHE_RSA and ECDHE_ECDSA)
// Note: Even with PSA_WANT_ALG_ECDH, legacy ECDH_C may be needed for cipher suite compilation
#define MBEDTLS_ECDH_C

// Ensure we have the necessary cipher suites for ECC
#define MBEDTLS_CIPHER_MODE_CBC
#define MBEDTLS_CIPHER_MODE_GCM


// RSA key exchange configuration:
// - ECDHE_RSA: ENABLED - Allows server with RSA cert (e.g., AmazonRootCA1) + ECDHE key exchange
//   Your client still uses ECC for client auth, server uses RSA cert
// - RSA/DHE_RSA/RSA_PSK: DISABLED - Pure RSA key exchanges not needed
// - ECDH_RSA: DISABLED - Static ECDH with RSA cert, not commonly used
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDH_RSA_ENABLED
#undef MBEDTLS_KEY_EXCHANGE_RSA_ENABLED
#undef MBEDTLS_KEY_EXCHANGE_RSA_PSK_ENABLED
#undef MBEDTLS_KEY_EXCHANGE_DHE_RSA_ENABLED

// RSA support for verifying server certificates (AmazonRootCA1 is RSA-2048)
// Required for ECDHE_RSA cipher suites - server uses RSA cert, key exchange is ECDHE
#define MBEDTLS_RSA_C
#define MBEDTLS_PKCS1_V15

// TLS record buffer sizes
// IN: Increased to 6KB to handle server certificate chain (AWS IoT Core sends ~5KB)
// OUT: Keep at 4KB - PSA key export had issues with larger buffers (Session 9)
#undef MBEDTLS_SSL_IN_CONTENT_LEN
#define MBEDTLS_SSL_IN_CONTENT_LEN 6144

#undef MBEDTLS_SSL_OUT_CONTENT_LEN
#define MBEDTLS_SSL_OUT_CONTENT_LEN 4096

// We want more prints
#define MBEDTLS_DEBUG_C
#undef MBEDTLS_VERBOSE
#define MBEDTLS_VERBOSE 1

//////////////////////////////////////
#endif // MBEDTLS_USER_OVERLAY_HEADER
