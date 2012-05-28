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

#include "apps/lib/oskar_settings_load.h"
#include "apps/lib/oskar_settings_load_beam_pattern.h"
#include "apps/lib/oskar_settings_load_image.h"
#include "apps/lib/oskar_settings_load_observation.h"
#include "apps/lib/oskar_settings_load_simulator.h"
#include "apps/lib/oskar_settings_load_sky.h"
#include "apps/lib/oskar_settings_load_telescope.h"
#include "utility/oskar_Log.h"
#include "utility/oskar_mem_init.h"
#include "utility/oskar_mem_append_raw.h"
#include "utility/oskar_settings_init.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

int oskar_settings_load(oskar_Settings* settings, oskar_Log* log,
        const char* filename)
{
    int error;

    /* Initialise the settings array. */
    error = oskar_settings_init(settings);
    if (error) return error;

    /* Load observation settings first. */
    error = oskar_settings_load_observation(&settings->obs, log, filename);
    if (error) return error;

    error = oskar_settings_load_simulator(&settings->sim, filename);
    if (error) return error;
    error = oskar_settings_load_sky(&settings->sky, filename);
    if (error) return error;
    error = oskar_settings_load_telescope(&settings->telescope, filename);
    if (error) return error;
    error = oskar_settings_load_image(&settings->image, filename);
    if (error) return error;
    error = oskar_settings_load_beam_pattern(&settings->beam_pattern, filename);
    if (error) return error;

    /* Save the path to the settings file. */
    error = oskar_mem_append_raw(&settings->settings_path, filename,
            OSKAR_CHAR, OSKAR_LOCATION_CPU, 1 + strlen(filename));
    if (error) return error;

    return OSKAR_SUCCESS;
}

#ifdef __cplusplus
}
#endif
