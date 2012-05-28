/*
 * Copyright (c) 2012, The University of Oxford
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Oxford nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OSKAR_LOG_WRITE_H_
#define OSKAR_LOG_WRITE_H_

/**
 * @file oskar_log_write.h
 */

#include "oskar_global.h"
#include "utility/oskar_Log.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 * Writes a generic message to a log.
 *
 * @details
 * This function writes a message to the log.
 *
 * @param[in,out] log    Pointer to a log structure.
 * @param[in]     code   Code (group) of log message.
 * @param[in]     depth  Level of nesting of log message.
 * @param[in]     width  Minimum width of key description.
 * @param[in]     prefix Description of key (set blank or NULL if not required).
 * @param[in]     format Format string for printf().
 *
 * @return An error code.
 */
OSKAR_EXPORT
int oskar_log_write(oskar_Log* log, char code, int depth, int width,
        const char* prefix, const char* format, ...);

/**
 * @brief
 * Writes a generic message to a log.
 *
 * @details
 * This function writes a message to the log.
 * It is called by other log functions, such as oskar_log_message(),
 * oskar_log_warning(), and oskar_log_error().
 *
 * @param[in,out] log    Pointer to a log structure.
 * @param[in]     code   Code (group) of log message.
 * @param[in]     depth  Level of nesting of log message.
 * @param[in]     width  Minimum width of key description.
 * @param[in]     prefix Description of key (set blank or NULL if not required).
 * @param[in]     args   Variable argument list for printf().
 *
 * @return An error code.
 */
OSKAR_EXPORT
int oskar_log_writev(oskar_Log* log, char code, int depth, int width,
        const char* prefix, const char* format, va_list args);

/**
 * @brief
 * Writes a generic message to standard error channel.
 *
 * @details
 * This function writes a message to the standard error channel.
 * It is called by oskar_log_error().
 *
 * @param[in,out] log    Pointer to a log structure.
 * @param[in]     code   Code (group) of log message.
 * @param[in]     depth  Level of nesting of log message.
 * @param[in]     width  Minimum width of key description.
 * @param[in]     prefix Description of key (set blank or NULL if not required).
 * @param[in]     args   Variable argument list for printf().
 *
 * @return An error code.
 */
int oskar_log_writev_stderr(char code, int depth, int width,
        const char* prefix, const char* format, va_list args);

/**
 * @brief
 * Writes a generic message to standard output channel.
 *
 * @details
 * This function writes a message to the standard output channel.
 * It is called by other log functions, such as oskar_log_message(),
 * oskar_log_warning().
 *
 * @param[in,out] log    Pointer to a log structure.
 * @param[in]     code   Code (group) of log message.
 * @param[in]     depth  Level of nesting of log message.
 * @param[in]     width  Minimum width of key description.
 * @param[in]     prefix Description of key (set blank or NULL if not required).
 * @param[in]     args   Variable argument list for printf().
 *
 * @return An error code.
 */
int oskar_log_writev_stdout(char code, int depth, int width,
        const char* prefix, const char* format, va_list args);

#ifdef __cplusplus
}
#endif

#endif /* OSKAR_LOG_WRITE_H_ */
