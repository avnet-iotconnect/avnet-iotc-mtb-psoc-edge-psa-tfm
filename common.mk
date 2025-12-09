################################################################################
# \file common.mk
# \version 1.0
#
# \brief
# Settings shared across all projects.
#
################################################################################
# \copyright
# (c) 2025, Infineon Technologies AG, or an affiliate of Infineon
# Technologies AG.  SPDX-License-Identifier: Apache-2.0
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

MTB_TYPE=PROJECT

# Target board/hardware (BSP).
# To change the target, it is recommended to use the Library manager
# ('make library-manager' from command line), which will also update 
# Eclipse IDE launch configurations.
TARGET=APP_KIT_PSE84_EVAL_EPC2

# Name of toolchain to use. Options include:
#
# GCC_ARM 	-- GCC is available as part of ModusToolbox Setup program
# ARM     	-- ARM Compiler (must be installed separately)
# LLVM_ARM	-- LLVM Embedded Toolchain (must be installed separately)
#
# See also: CY_COMPILER_PATH below
TOOLCHAIN=GCC_ARM

# Toolchains supported by this code example. See README.md file.
# This is used by automated build systems to identify the supported toolchains. 
MTB_SUPPORTED_TOOLCHAINS=GCC_ARM ARM LLVM_ARM

# Default build configuration. Options include:
#
# Debug -- build with minimal optimizations, focus on debugging.
# Release -- build with full optimizations
# Custom -- build with custom configuration, set the optimization flag in CFLAGS
# 
# If CONFIG is manually edited, ensure to update or regenerate 
# launch configurations for your IDE.
CONFIG=Debug

ifeq ($(TOOLCHAIN),GCC_ARM)
    ifeq ($(CONFIG),Release)
$(error GCC_ARM Release mode is not supported in this version of the code example)
    endif
endif

# Building ifx-mcuboot with ARM compiler requries some
# specific symbols. However, linking when ifx-mcuboot is
# added as a library to an existing project,
# those symbols are not required. Excluding those symbols,
#which are not needed for this application build
ifeq ($(TOOLCHAIN),ARM)
  DEFINES+=MCUBOOT_SKIP_CLEANUP_RAM=1
endif

# model selection

# For KIT_PSE84_EVAL_EPC2, KIT_PSE84_EVAL_EPC4 and KIT_PSE84_AI below models are supported.
####### WARNING AUDIO TASKS (COUGH, ALARM, BABYCRY, DOA) TASK ARE NOT SUPPORTED AS OF NOW. ##########
# COUGH_MODEL
# ALARM_MODEL
# BABYCRY_MODEL
# DIRECTIONOFARRIVAL_MODEL
# FALLDETECTION_MODEL
# GESTURE_MODEL (Reserved for future boards)
# MOTION_SENSOR (No AI, just ure IMU)
# IDLE (For testing only - will send dummy IPC messages to CM33)

MODEL_SELECTION = MOTION_SENSOR

#Config file for postbuild sign and merge operations.
#NOTE:Check the JSON file for the command parameters
COMBINE_SIGN_JSON?=./bsps/TARGET_$(TARGET)/config/GeneratedSource/boot_with_bldr.json

include ../common_app.mk
