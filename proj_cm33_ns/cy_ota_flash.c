/*****************************************************************************
* File Name        : cy_ota_flash.c
*
* Description      : This file contains flash operations implementation
*
* Related Document : See README.md
*
*******************************************************************************
* (c) 2024-2025, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
* 
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/
/* Header file includes */
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "cy_pdl.h"
#include "cy_ota_flash.h"

#include "cycfg.h"
#include "cycfg_qspi_memslot.h"  /* Generated QSPI config with S25FS128S_SMIF0_SlaveSlot_1 */
#include "cy_smif_memslot.h"     /* PDL SMIF memslot API */

/* Direct PDL access - bypassing mtb_serial_memory due to TF-M SRF context issue */

/**********************************************************************************************************************************
 * local defines
 **********************************************************************************************************************************/
/* This defines if External Flash (SMIF) will be used for Upgrade Slots */




#define SECTOR_ADDR                 (0x40000U)  /* Offset to the start of external memory that belongs to the sector for which size is returned. */
#define SMIF_DATA_QUAD              (0x04U)
#define DATA_WIDTH_PINS             (0x04U)
#define CY_FLASH_ERASE_SIZE         (0x40000UL) /* Erase Size for External Flash and XIP area */
#define CY_FLASH_SIZEOF_ROW         (512UL)


/**********************************************************************************************************************************
 * local variables & data
 **********************************************************************************************************************************/
/* Simple SMIF context for direct PDL access.
 * Hardware is already initialized by bootloader, so we just need a minimal context.
 * We bypass mtb_serial_memory because its _nonsecure variant relies on SRF context
 * that TF-M never populates (TF-M uses its own ifx_driver_smif, not PDL Cy_SMIF_MemNumInit). */
static cy_stc_smif_context_t smif_context;

/**********************************************************************************************************************************
 * Internal Functions
 **********************************************************************************************************************************/
/**
 * @brief Initializes flash, QSPI flash, or any other external memory type
 *
 * @return  CY_RSLT_SUCCESS on success
 *          CY_RSLT_TYPE_ERROR on failure
 */
cy_rslt_t cy_ota_mem_init( void )
{
    /* Direct PDL initialization - bypassing mtb_serial_memory.
     * The SMIF hardware is already initialized by the bootloader.
     * TF-M doesn't populate the SRF context array that mtb_serial_memory_setup_nonsecure() needs,
     * so we use direct PDL Cy_SMIF_Mem* functions instead. */
    
    /* Initialize minimal context - hardware is already running from bootloader init */
    memset(&smif_context, 0, sizeof(smif_context));
    
    /* 0 means no timeout (infinite wait) */
    smif_context.timeout = 0UL;
    
    printf("External Memory initialized (direct PDL, bypassing mtb_serial_memory).\n");
    
    return CY_RSLT_SUCCESS;
}

/**
 * @brief Read from flash, QSPI flash, or any other external memory type
 *
 * @param[in]   mem_type   Memory type @ref cy_ota_mem_type_t
 * @param[in]   addr       Starting address to read from.
 * @param[out]  data       Pointer to the buffer to store the data read from the memory.
 * @param[in]   len        Number of data bytes to read.
 *
 * @return  CY_RSLT_SUCCESS on success
 *          CY_RSLT_TYPE_ERROR on failure
 */
cy_rslt_t cy_ota_mem_read( cy_ota_mem_type_t mem_type, uint32_t addr, void *data, size_t len )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if(mem_type == CY_OTA_MEM_TYPE_EXTERNAL_FLASH)
    {
        if(addr >= CY_XIP_PORT0_S_SBUS_BASE)
        {
            addr -= CY_XIP_PORT0_S_SBUS_BASE;
        }
        else if(addr >= CY_XIP_PORT0_NS_SBUS_BASE)
        {
            addr -= CY_XIP_PORT0_NS_SBUS_BASE;
        }
        else
        {
            //Nothing to do
        }

        /* Direct PDL call using generated memory config */
        cy_en_smif_status_t smif_status;
        smif_status = Cy_SMIF_MemRead(CYBSP_SMIF_CORE_0_XSPI_FLASH_HW, 
                                       &S25FS128S_SMIF0_SlaveSlot_1,
                                       addr, 
                                       (uint8_t*)data, 
                                       len, 
                                       &smif_context);
        
        if (smif_status != CY_SMIF_SUCCESS)
        {
            printf("Cy_SMIF_MemRead failed: status=%d, addr=0x%08lx, len=%u\n", 
                   (int)smif_status, (unsigned long)addr, (unsigned)len);
            result = CY_RSLT_TYPE_ERROR;
        }

        return result;
    }
    else
    {
        printf("%s() READ not supported for memory type %d\n", __func__, (int)mem_type);
        return CY_RSLT_TYPE_ERROR;
    }

}

/**
 * @brief Write to flash, QSPI flash, or any other external memory type
 *
 * @param[in]   mem_type   Memory type @ref cy_ota_mem_type_t
 * @param[in]   addr       Starting address to write to.
 * @param[in]   data        Pointer to the buffer to conaitning the write data
 * @param[in]   len        Number of data bytes to write.
 *
 * @return  CY_RSLT_SUCCESS on success
 *          CY_RSLT_TYPE_ERROR on failure
 */
static cy_rslt_t cy_ota_mem_write_row_size( cy_ota_mem_type_t mem_type, uint32_t addr, void *data, size_t len )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if(mem_type == CY_OTA_MEM_TYPE_EXTERNAL_FLASH)
    {
        if(addr >= CY_XIP_PORT0_S_SBUS_BASE)
        {
            addr -= CY_XIP_PORT0_S_SBUS_BASE;
        }
        else if(addr >= CY_XIP_PORT0_NS_SBUS_BASE)
        {
            addr -= CY_XIP_PORT0_NS_SBUS_BASE;
        }
        else
        {
            //Nothing to do
        }

        /* Direct PDL call using generated memory config */
        cy_en_smif_status_t smif_status;
        smif_status = Cy_SMIF_MemWrite(CYBSP_SMIF_CORE_0_XSPI_FLASH_HW, 
                                        &S25FS128S_SMIF0_SlaveSlot_1,
                                        addr, 
                                        (const uint8_t*)data, 
                                        len, 
                                        &smif_context);
        
        if (smif_status != CY_SMIF_SUCCESS)
        {
            printf("Cy_SMIF_MemWrite failed: status=%d, addr=0x%08lx, len=%u\n", 
                   (int)smif_status, (unsigned long)addr, (unsigned)len);
            result = CY_RSLT_TYPE_ERROR;
        }

        return result;
    }
    else
    {
        printf("%s() Write not supported for memory type %d\n", __func__, (int)mem_type);
        return CY_RSLT_TYPE_ERROR;
    }

}

/**
 * @brief Write to flash, QSPI flash, or any other external memory type
 *
 * @param[in]   mem_type   Memory type @ref cy_ota_mem_type_t
 * @param[in]   addr       Starting address to write to.
 * @param[in]   data       Pointer to the buffer containing the data to be written.
 * @param[in]   len        Number of bytes to write.
 *
 * @return  CY_RSLT_SUCCESS on success
 *          CY_RSLT_TYPE_ERROR on failure
 */
cy_rslt_t cy_ota_mem_write( cy_ota_mem_type_t mem_type, uint32_t addr, void *data, size_t len )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /**
     * This is used if a block is < Block size to satisfy requirements
     * of flash_area_write(). "static" so it is not on the stack.
     */
    static uint8_t block_buffer[CY_FLASH_SIZEOF_ROW];
    uint32_t chunk_size = 0;

    uint32_t bytes_to_write = len;
    uint32_t curr_addr = addr;
    uint8_t *curr_src = data;

    while(bytes_to_write > 0x0U)
    {
        chunk_size = bytes_to_write;
        if(chunk_size > CY_FLASH_SIZEOF_ROW)
        {
            chunk_size = CY_FLASH_SIZEOF_ROW;
        }

        /* Is the chunk_size smaller than a flash row? */
        if((chunk_size % CY_FLASH_SIZEOF_ROW) != 0x0U)
        {
            uint32_t row_offset = 0;
            uint32_t row_base = 0;

            row_base   = (curr_addr / CY_FLASH_SIZEOF_ROW) * CY_FLASH_SIZEOF_ROW;
            row_offset = curr_addr - row_base;

            if((row_offset + chunk_size) > CY_FLASH_SIZEOF_ROW)
            {
                chunk_size = (CY_FLASH_SIZEOF_ROW - row_offset);
            }

            /* we will read a CY_FLASH_SIZEOF_ROW byte block, write the new data into the block, then write the whole block */
            result = cy_ota_mem_read(mem_type, row_base, (void *)(&block_buffer[0]), sizeof(block_buffer));
            if(result != CY_RSLT_SUCCESS)
            {
                 return CY_RSLT_TYPE_ERROR;
            }
            memcpy (&block_buffer[row_offset], curr_src, chunk_size);

            result = cy_ota_mem_write_row_size(mem_type, row_base, (void *)(&block_buffer[0]), sizeof(block_buffer));
            if(result != CY_RSLT_SUCCESS)
            {
                return CY_RSLT_TYPE_ERROR;
            }
        }
        else
        {
            result = cy_ota_mem_write_row_size(mem_type, curr_addr, curr_src, chunk_size);
            if(result != CY_RSLT_SUCCESS)
            {
                return CY_RSLT_TYPE_ERROR;
            }
        }

        curr_addr += chunk_size;
        curr_src += chunk_size;
        bytes_to_write -= chunk_size;
    }

    return CY_RSLT_SUCCESS;
}

/**
 * @brief Erase flash, QSPI flash, or any other external memory type
 *
 * @param[in]   mem_type   Memory type @ref cy_ota_mem_type_t
 * @param[in]   addr       Starting address to begin erasing.
 * @param[in]   len        Number of bytes to erase.
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_TYPE_ERROR
 */
cy_rslt_t cy_ota_mem_erase( cy_ota_mem_type_t mem_type, uint32_t addr, size_t len )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if(mem_type == CY_OTA_MEM_TYPE_EXTERNAL_FLASH)
    {
        uint32_t offset=0;

        if(addr >= CY_XIP_PORT0_S_SBUS_BASE)
        {
            offset= addr - CY_XIP_PORT0_S_SBUS_BASE;
        }
        else if(addr >= CY_XIP_PORT0_NS_SBUS_BASE)
        {
            offset= addr - CY_XIP_PORT0_NS_SBUS_BASE;
        }
        else
        {
            //Nothing to do
        }
        
        /* Direct PDL call using generated memory config */
        cy_en_smif_status_t smif_status;
        smif_status = Cy_SMIF_MemEraseSector(CYBSP_SMIF_CORE_0_XSPI_FLASH_HW, 
                                              &S25FS128S_SMIF0_SlaveSlot_1,
                                              offset, 
                                              len, 
                                              &smif_context);
        
        if (smif_status != CY_SMIF_SUCCESS)
        {
            printf("Cy_SMIF_MemEraseSector failed: status=%d, addr=0x%08lx, len=%u\n", 
                   (int)smif_status, (unsigned long)offset, (unsigned)len);
            result = CY_RSLT_TYPE_ERROR;
        }

        return result;
    }
    else
    {
        printf("%s() Erase not supported for memory type %d\n", __func__, (int)mem_type);
        result = CY_RSLT_TYPE_ERROR;
        return result;
    }

}

/**
 * @brief To get page size for programming flash, QSPI flash, or any other external memory type
 *
 * @param[in]   mem_type   Memory type @ref cy_ota_mem_type_t
 * @param[in]   addr       Address that belongs to the sector for which programming page size needs to be returned.
 *
 * @return    Page size in bytes.
 */
size_t cy_ota_mem_get_prog_size ( cy_ota_mem_type_t mem_type, uint32_t addr )
{
    (void)addr; /* unused - same prog size across entire flash */
    if(mem_type == CY_OTA_MEM_TYPE_EXTERNAL_FLASH)
    {
        /* Return program page size from generated device config */
        return deviceCfg_S25FS128S_SMIF0_SlaveSlot_1.programSize;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief To get sector size of flash, QSPI flash, or any other external memory type
 *
 * @param[in]   mem_type   Memory type @ref cy_ota_mem_type_t
 * @param[in]   addr       Address that belongs to the sector for which sector erase size needs to be returned.
 *
 * @return    Sector size in bytes.
 */
size_t cy_ota_mem_get_erase_size ( cy_ota_mem_type_t mem_type, uint32_t addr )
{
    (void)addr; /* unused - same erase size across entire flash (or use hybrid region lookup if needed) */
    if(mem_type == CY_OTA_MEM_TYPE_EXTERNAL_FLASH)
    {
        /* Return erase sector size from generated device config */
        return deviceCfg_S25FS128S_SMIF0_SlaveSlot_1.eraseSize;
    }
    else
    {
        return 0;
    }
}
