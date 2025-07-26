#include "error.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define MAX_ERROR_MSG_LENGTH 1024
static char sf_error_msg[MAX_ERROR_MSG_LENGTH];


SF_ErrorCode sf_set_error(SF_ErrorCode error, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(sf_error_msg, sizeof(sf_error_msg), fmt, ap);
    va_end(ap);

    return error;
}

const char* sf_last_error(void) {
    static char msg_buffer[MAX_ERROR_MSG_LENGTH];
    strncpy(msg_buffer, sf_error_msg, MAX_ERROR_MSG_LENGTH);
    msg_buffer[MAX_ERROR_MSG_LENGTH - 1] = '\0';

    sf_error_msg[0] = '\0';

    return msg_buffer;
}

