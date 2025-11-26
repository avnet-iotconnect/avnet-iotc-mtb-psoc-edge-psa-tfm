/*******************************************************************************
* File Name        : ipc_communication.h
*
* Description      : This file contains the headers and structures
*                    used to setup the IPC Pipes between CM33 and CM55
*
* Related Document : See README.md
*
********************************************************************************
* Copyright 2025, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/
/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com>, Shu Liu <shu.liu@avnet.com> et al.
 */

#ifndef SOURCE_IPC_COMMUNICATION_H
#define SOURCE_IPC_COMMUNICATION_H

/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdint.h>
#include "cybsp.h"
#include "cy_pdl.h"
#include "cy_ipc_pipe.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define CY_IPC_MAX_ENDPOINTS            (5UL)
#define CY_IPC_CYPIPE_CLIENT_CNT        (8UL)

/*******************************************************************************
 * IPC1 Configuration for CM33 <-> CM55 Communication
 * 
 * PSE84 has two IPC instances:
 *   IPC0 (CPUSS_IPC):    Channels 0-15,  Interrupts 0-7  - Protected by TF-M
 *   IPC1 (APPCPUSS_IPC): Channels 16-31, Interrupts 8-15 - For user applications
 *
 * In a TF-M environment, IPC0 is protected by PPC and cannot be accessed
 * from the non-secure world. We must use IPC1 for CM33 NS <-> CM55 communication.
 *
 * Available IPC1 resources:
 *   - Channels: 17-30 (channel 16 reserved by PDL, channel 31 reserved for semaphores)
 *   - Interrupts: 9-15
 ******************************************************************************/

/* IPC1 Channel and Interrupt assignments for custom pipe */
#define CY_IPC_CHAN_CYPIPE_EP1          (18UL)   /* IPC1 channel 2 (PDL index 18) for CM33 */
#define CY_IPC_INTR_CYPIPE_EP1          (9UL)    /* IPC1 interrupt 1 (PDL index 9) for CM33 */
#define CY_IPC_CHAN_CYPIPE_EP2          (19UL)   /* IPC1 channel 3 (PDL index 19) for CM55 */
#define CY_IPC_INTR_CYPIPE_EP2          (10UL)   /* IPC1 interrupt 2 (PDL index 10) for CM55 */

/* IPC Pipe Endpoint-1 config (CM33) */
#define CY_IPC_CYPIPE_CHAN_MASK_EP1     CY_IPC_CH_MASK(CY_IPC_CHAN_CYPIPE_EP1)
#define CY_IPC_CYPIPE_INTR_MASK_EP1     CY_IPC_INTR_MASK(CY_IPC_INTR_CYPIPE_EP1)
#define CY_IPC_INTR_CYPIPE_PRIOR_EP1    (1UL)
#define CY_IPC_INTR_CYPIPE_MUX_EP1      (CY_IPC1_INTR_MUX(CY_IPC_INTR_CYPIPE_EP1))
#define CM33_IPC_PIPE_EP_ADDR           (1UL)
#define CM33_IPC_PIPE_CLIENT_ID         (3UL)

/* IPC Pipe Endpoint-2 config (CM55) */
#define CY_IPC_CYPIPE_CHAN_MASK_EP2     CY_IPC_CH_MASK(CY_IPC_CHAN_CYPIPE_EP2)
#define CY_IPC_CYPIPE_INTR_MASK_EP2     CY_IPC_INTR_MASK(CY_IPC_INTR_CYPIPE_EP2)
#define CY_IPC_INTR_CYPIPE_PRIOR_EP2    (1UL)
#define CY_IPC_INTR_CYPIPE_MUX_EP2      (CY_IPC1_INTR_MUX(CY_IPC_INTR_CYPIPE_EP2))
#define CM55_IPC_PIPE_EP_ADDR           (2UL)
#define CM55_IPC_PIPE_CLIENT_ID         (5UL)

/* Combined Interrupt Mask */
#define CY_IPC_CYPIPE_INTR_MASK         ( CY_IPC_CYPIPE_CHAN_MASK_EP1 | CY_IPC_CYPIPE_CHAN_MASK_EP2)

/*******************************************************************************
* Enumeration
*******************************************************************************/

/* The actual payload being sent via IPC. This will vary b etween applications */
typedef struct {
    uint32_t    label_id;
    char        label[256];
} ipc_payload_t;

/* IPC Message structure */
/* Pointer to this structure will be shared through IPC Pipe */
typedef struct
{
    uint8_t         client_id; /* This must be a part of the IPC structure */
    uint16_t        intr_mask; /* This must be a part of the IPC structure */
    ipc_payload_t   payload;
} ipc_msg_t;

/*******************************************************************************
* Function prototypes
*******************************************************************************/
void cm33_ipc_communication_setup(void);
void cm33_ipc_pipe_isr(void);
void cm55_ipc_communication_setup(void);
void cm55_ipc_pipe_isr(void);

/* App functions for cm33 */
bool cm33_ipc_has_received_message(void);
void cm33_ipc_safe_copy_last_payload(ipc_payload_t* target);

/* A mechanism through which the application can get the last recorded/cached detection.
    Invoking this call will return true if there was a detection. 
    Invoking this call will also clear the last detection if there was one in the first place.
   */
bool cm33_ipc_safe_get_and_clear_cached_detection(ipc_payload_t* target);

/* App functions for cm55 */
ipc_payload_t* cm55_ipc_get_payload_ptr(void);
void cm55_ipc_send_to_cm33(void);

#endif /* SOURCE_IPC_COMMUNICATION_H */
