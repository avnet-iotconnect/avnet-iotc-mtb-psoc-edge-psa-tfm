#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "psa/crypto.h"
#include "mbedtls/pk.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/pem.h"
#include "cy_tcpip_port_secure_sockets.h"
#include "cy_syslib.h"
#include "psa/crypto_values.h"
#include "psa/internal_trusted_storage.h"
#include "mbedtls/ecdsa.h"

// for TFM
#include "tfm_ns_interface.h"
#include "os_wrapper/common.h"

/* Isolated function to test PSA algorithm support with iterations */
void test_psa_algorithms() {
    typedef struct {
        const char *name;
        psa_key_lifetime_t lifetime;
        psa_key_usage_t usage;
        psa_algorithm_t alg;
        psa_key_type_t type;
        size_t bits;
    } test_case_t;

    test_case_t tests[] = {
        {"Volatile EC Sign", PSA_KEY_LIFETIME_VOLATILE, PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_SIGN_HASH, PSA_ALG_ECDSA(PSA_ALG_SHA_256), PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1), 256},
        {"Volatile EC Derive", PSA_KEY_LIFETIME_VOLATILE, PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_DERIVE, PSA_ALG_ECDH, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1), 256},
        {"Volatile EC Sign+Derive", PSA_KEY_LIFETIME_VOLATILE, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_DERIVE, PSA_ALG_ECDSA(PSA_ALG_SHA_256), PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1), 256},
        {"Volatile Make Deterministic Key", PSA_KEY_LIFETIME_VOLATILE, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_DERIVE, PSA_ALG_DETERMINISTIC_ECDSA(PSA_ALG_SHA_256), PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1), 256},
        {"Persistent EC Sign", PSA_KEY_LIFETIME_PERSISTENT, PSA_KEY_USAGE_SIGN_HASH, PSA_ALG_ECDSA(PSA_ALG_SHA_256), PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1), 256},
        {"Volatile RSA Sign/Verify (PKCS#1 v1.5)", PSA_KEY_LIFETIME_VOLATILE, PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH, PSA_ALG_RSA_PKCS1V15_SIGN(PSA_ALG_SHA_256), PSA_KEY_TYPE_RSA_KEY_PAIR, 2048},
    };

    psa_status_t status;
    psa_key_id_t test_key_id;
    size_t num_tests = sizeof(tests) / sizeof(test_case_t);

    printf("=== PSA Support Test (Iterative) ===\n");

    status = psa_crypto_init();
    printf("PSA Init: %ld\n", (long)status);
    if (status != PSA_SUCCESS) return;

    for (size_t i = 0; i < num_tests; i++) {
        test_case_t *tc = &tests[i];
        psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;

        printf("\nTest %d: %s\n", (int) i, tc->name);

        psa_set_key_lifetime(&attr, tc->lifetime);
        psa_set_key_usage_flags(&attr, tc->usage);
        psa_set_key_algorithm(&attr, tc->alg);
        psa_set_key_type(&attr, tc->type);
        psa_set_key_bits(&attr, tc->bits);
        if (tc->lifetime == PSA_KEY_LIFETIME_PERSISTENT) {
            psa_destroy_key(9U);
            psa_set_key_id(&attr, 9U);
        }

        status = psa_generate_key(&attr, &test_key_id);
        printf("  Generate Key: %ld\n", (long)status);

        if (status == PSA_SUCCESS) {
            if (tc->usage & PSA_KEY_USAGE_SIGN_HASH) {
                uint8_t sig[256];
                size_t sig_len;
                uint8_t digest[32] = {0};
                status = psa_sign_hash(test_key_id, tc->alg, digest, sizeof(digest), sig, sizeof(sig), &sig_len);
                printf("  Sign: %ld\n", (long)status);
                if (status == PSA_SUCCESS) {
                    // Test verify
                    status = psa_verify_hash(test_key_id, tc->alg, digest, sizeof(digest), sig, sig_len);
                    printf("  Verify: %ld\n", (long)status);
                }
            }

            if (tc->usage & PSA_KEY_USAGE_VERIFY_HASH && tc->type == PSA_KEY_TYPE_RSA_KEY_PAIR) {
                // RSA verification test - sign with same key and verify
                uint8_t sig[256];
                size_t sig_len;
                uint8_t digest[32] = {0};
                status = psa_sign_hash(test_key_id, tc->alg, digest, sizeof(digest), sig, sizeof(sig), &sig_len);
                printf("  RSA Sign: %ld\n", (long)status);
                if (status == PSA_SUCCESS) {
                    status = psa_verify_hash(test_key_id, tc->alg, digest, sizeof(digest), sig, sig_len);
                    printf("  RSA Verify: %ld\n", (long)status);
                }
            }

            if (tc->usage & PSA_KEY_USAGE_DERIVE && tc->alg == PSA_ALG_ECDH) {
                // Simple ECDH test with valid dummy peer public key (secp256r1 uncompressed)
                uint8_t shared[32];
                size_t secret_len;
                uint8_t dummy_peer[65] = {
                    0x04, // uncompressed
                    0x6B, 0x17, 0xD1, 0xF2, 0xE1, 0x2C, 0x42, 0x47, 0xF8, 0xBC, 0xE6, 0xE5, 0x63, 0xA4, 0x40, 0xF2,
                    0x77, 0x03, 0x7D, 0x81, 0x2D, 0xEB, 0x33, 0xA0, 0xF4, 0xA1, 0x39, 0x45, 0xD8, 0x98, 0xC2, 0x96,
                    0x4F, 0xE3, 0x42, 0xE2, 0xFE, 0x1A, 0x7F, 0x9B, 0x8E, 0xE7, 0xEB, 0x4A, 0x7C, 0x0F, 0x9E, 0x16,
                    0x2B, 0xCE, 0x33, 0x57, 0x6B, 0x31, 0x5E, 0xCE, 0xCB, 0xB6, 0x40, 0x68, 0x37, 0xBF, 0x51, 0xF5
                };
                status = psa_raw_key_agreement(tc->alg, test_key_id, dummy_peer, sizeof(dummy_peer), shared, sizeof(shared), &secret_len);
                printf("  ECDH: %ld\n", (long)status);
            }

            if (tc->alg == PSA_ALG_GCM) {
                // Test AES GCM encryption
                uint8_t plaintext[16] = "Hello AES GCM!";
                uint8_t ciphertext[16 + 16]; // + tag
                size_t ciphertext_len;
                uint8_t nonce[12] = {0};
                uint8_t aad[0]; // no AAD
                status = psa_aead_encrypt(test_key_id, tc->alg, nonce, sizeof(nonce), aad, sizeof(aad), plaintext, sizeof(plaintext), ciphertext, sizeof(ciphertext), &ciphertext_len);
                printf("  AES GCM Encrypt: %ld\n", (long)status);
            }

            psa_destroy_key(test_key_id);
        }
    }

    printf("\n=== End PSA Support Test ===\n");
}
