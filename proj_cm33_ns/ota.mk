################################################################################
# Basic OTA configuration
################################################################################
# Set to 1 to add OTA defines, sources, and libraries (must be used with MCUBoot)
OTA_SUPPORT?=1

ifeq ($(OTA_SUPPORT),1)
    DEFINES+=IOTC_OTA_SUPPORT=1
endif

CY_BOOTLOADER?=IFX_MCUBOOT
    
# Add Boot loader support
COMPONENTS+=$(CY_BOOTLOADER)

 # OTA HTTP support
OTA_HTTP_SUPPORT=1

    
DEFINES+= OTA_SUPPORT=1 \
          APP_VERSION_MAJOR=$(APP_VERSION_MAJOR) \
          APP_VERSION_MINOR=$(APP_VERSION_MINOR) \
          APP_VERSION_BUILD=$(APP_VERSION_BUILD) \
          APP_1_NAME=$(APP_1_NAME) \
          APP_2_NAME=$(APP_2_NAME) \
          APP_3_NAME=$(APP_3_NAME)
    
    
ifneq ($(MAKECMDGOALS),get_app_info)      
    OTA_UPDATE_MK := $(wildcard $(CY_GETLIBS_SHARED_PATH)/$(CY_GETLIBS_SHARED_NAME)/ota-update/*/makefiles/ota_update.mk)
    include $(OTA_UPDATE_MK)
endif
    
OTA_PLATFORM?=PSE84
XIP_MODE?=XIP
	
ifneq ($(MAKECMDGOALS),get_app_info)      
    # Path to the OTA linker script to use.
    OTA_LINKER_FILE?=$(MTB_TOOLS__TARGET_DIR)/COMPONENT_$(CORE)/TOOLCHAIN_$(TOOLCHAIN)/pse84_ns_cm33.$(BSP_LINKER_SCRIPT_EXT)
endif 

$(info OTA Linker file path = $(OTA_LINKER_FILE))
# Set Python path
ifeq ($(CY_PYTHON_PATH),)
    ifeq ($(OS),Windows_NT)
        CY_PYTHON_PATH=python
    else
        CY_PYTHON_PATH=python3
    endif
endif # checking for Python path
    
ifneq ($(MAKECMDGOALS),get_app_info)
    ifneq ($(MAKECMDGOALS),eclipse) 
        OTA_BOOTLOADER_ABSTRACTION_DIRS := $(wildcard $(CY_GETLIBS_SHARED_PATH)/$(CY_GETLIBS_SHARED_NAME)/ota-bootloader-abstraction/*/makefiles/mcuboot/ifx_mcuboot_support.mk)
        include $(OTA_BOOTLOADER_ABSTRACTION_DIRS)
    endif
endif

AWS_IOT_DEVICE_SDK_DIRS := $(wildcard $(CY_GETLIBS_SHARED_PATH)/$(CY_GETLIBS_SHARED_NAME)/aws-iot-device-sdk-embedded-C/*/libraries/standard/coreMQTT/)
COMMAND_CONSOLE_IPERF_DIR := $(wildcard $(CY_GETLIBS_SHARED_PATH)/$(CY_GETLIBS_SHARED_NAME)/command-console/release*/source/iperf)
COMMAND_CONSOLE_IPERF_UTIL_DIR := $(wildcard $(CY_GETLIBS_SHARED_NAME)/command-console/release*/source/iperf_utility)
COMPONENT_HCI-UART_DIR := $(wildcard $(CY_GETLIBS_SHARED_PATH)/$(CY_GETLIBS_SHARED_NAME)/btstack-integration/release*/COMPONENT_HCI-UART/)

#CY_IGNORE+= $(AWS_IOT_DEVICE_SDK_DIRS)
CY_IGNORE+=$(COMMAND_CONSOLE_IPERF_DIR)
CY_IGNORE+=$(COMMAND_CONSOLE_IPERF_UTIL_DIR)
CY_IGNORE+=$(COMPONENT_HCI-UART_DIR)
CY_IGNORE+=$(SEARCH_btstack-integration)
CY_IGNORE+=$(SEARCH_btstack)
