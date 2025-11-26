#include <stdio.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

// for TFM
#include "ifx_platform_api.h"


static void write_safe(const uint8_t *data, uint32_t len) {
    uint32_t offset = 0;
    while (offset < len) {
        uint32_t remain = len - offset;
        uint32_t req = remain > 32 ? 32 : remain;   /* <- fixed */
        uint32_t n = (uint32_t)ifx_platform_log_msg(data + offset, req);
        offset += n;
        if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
            vTaskDelay(pdMS_TO_TICKS(n / 10 + 1));
        }
    }
}

int _write(int fd, const void *buf, size_t count) {
    const char *input = (const char *)buf;
    size_t start = 0;

    #ifndef CY_RETARGET_IO_CONVERT_LF_TO_CRLF
    write_safe((const uint8_t *)buf, count);
    #else
    for (size_t i = 0; i < count; ++i) {
        if (input[i] == '\n' && (i == 0 || input[i - 1] != '\r')) {
            write_safe((const uint8_t *)&input[start], i - start);
            write_safe((const uint8_t *)"\r\n", 2);
            start = i + 1;
        }
    }
    if (start < count) {
        write_safe((const uint8_t *)&input[start], count - start);
    }
    #endif

    return (int)count;
}
