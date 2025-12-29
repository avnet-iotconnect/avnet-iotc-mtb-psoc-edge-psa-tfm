/* SPDX-License-Identifier: MIT
 * Copyright (C) 2025 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com>, Shu Liu <shu.liu@avnet.com> et al.
 */

#ifndef APP_IO_INTERFACE_H
#define APP_IO_INTERFACE_H

#include <stddef.h>
#include <stdbool.h>

/*
*******************************************************************************
This file contains the function prototypes for application IO handling
(primarily for App Settings Configurator over USB CDC).
While the only current implementation is for USB CDC, 
the intent is to abstract the underlying IO mechanism (e.g. main IO, USB CDC, etc.)
so that the application code does not need to be changed.
******************************************************************************
*/


int app_io_init(void);

// Several ways to write data:
void app_io_write_data(const char* data, size_t data_len);
void app_io_write_data_crlf(const char* data, size_t data_len);
void app_io_write_str(const char* data);
void app_io_write_str_crlf(const char* data);

// Blocking read lines into buffer until newline or (until_eod=true)EOD character (CTRL-D) is encountered.
// Returns number of bytes read. (no error condition defined as of now)
int app_io_read_lines(char * buffer, size_t buffer_len, bool until_eod);

void app_io_start_password_masking(void);
void app_io_stop_password_masking(void);

#endif