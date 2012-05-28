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

#include "utility/oskar_log_file_data.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

char* oskar_log_file_data(oskar_Log* log, long* size)
{
    char* data = 0;
    if (size == NULL) return 0;

    /* If log exists, then write it out. */
    if (log)
    {
#ifdef _OPENMP
        /* Lock mutex. */
        omp_set_lock(&log->mutex);
#endif
        if (log->file)
        {
            /* Determine the current size of the file. */
            fflush(log->file);
            fseek(log->file, 0, SEEK_END);
            *size = ftell(log->file);

            /* Read the file into memory. */
            if (*size != 0)
            {
                size_t bytes_read = 0;
                data = (char*) malloc(*size * sizeof(char));
                if (data != 0)
                {
                    rewind(log->file);
                    bytes_read = fread(data, 1, (size_t)(*size), log->file);
                    if (bytes_read != (size_t)(*size))
                    {
                        free(data);
                        data = 0;
                    }
                }
            }
        }
#ifdef _OPENMP
        /* Unlock mutex. */
        omp_unset_lock(&log->mutex);
#endif
    }

    return data;
}

#ifdef __cplusplus
}
#endif
