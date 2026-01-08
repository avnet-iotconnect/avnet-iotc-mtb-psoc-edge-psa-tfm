/*
 * Project-specific TF-M configuration overrides for proj_cm33_s
 *
 * This file is included via IFX_PROJECT_CONFIG_PATH at the top of
 * config_tfm_target.h, BEFORE the default values are set.
 * Use #define (not #undef/#define) for values you want to override.
 */

#ifndef IFX_TFM_CONFIG_H
#define IFX_TFM_CONFIG_H

/*
 * Increase IOVEC buffer for large certificate chains (AWS S3 sends ~5.5KB certs)
 * Default is 5120, but we need more for TLS handshake hash operations
 */
#define CRYPTO_IOVEC_BUFFER_SIZE    6144

/*
 * Heap size for the crypto backend
 * CRYPTO_ENGINE_BUF_SIZE needs to be >8KB for EC signing by attest module.
 * Increase if you see PSA_ERROR_INSUFFICIENT_MEMORY during crypto operations.
 */
/* #define CRYPTO_ENGINE_BUF_SIZE   0x2380 */

/*
 * ITS (Internal Trusted Storage) configuration
 * Uncomment and adjust if you need more storage assets or larger asset sizes
 */
/* #define ITS_MAX_ASSET_SIZE       512 */
/* #define ITS_NUM_ASSETS           10 */

/*
 * PS (Protected Storage) configuration
 * Uncomment and adjust if you need more storage assets or larger asset sizes
 */
/* #define PS_MAX_ASSET_SIZE        2048 */
/* #define PS_NUM_ASSETS            10 */

#endif /* IFX_TFM_CONFIG_H */
