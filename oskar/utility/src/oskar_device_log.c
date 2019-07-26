/*
 * Copyright (c) 2018-2019, The University of Oxford
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

#include <stdlib.h>
#include <string.h>

#include "log/oskar_log.h"
#include "mem/oskar_mem.h"
#include "utility/private_device.h"
#include "utility/oskar_device.h"
#include "utility/oskar_device_log.h"

#ifdef __cplusplus
extern "C" {
#endif

void oskar_device_log_details(const oskar_Device* device, oskar_Log* log)
{
    const char p = 'M';
    oskar_log_message(log, p, 0, "Device %d (%s):",
            device->index, device->name);
    oskar_log_value(log, p, 1, "Vendor", "%s", device->vendor);
    oskar_log_value(log, p, 1, "Compute platform", "%s",
            device->platform_type == 'C' ? "CUDA" : "OpenCL");
    if (device->platform_type == 'C')
    {
        oskar_log_value(log, p, 1, "CUDA runtime version", "%d.%d",
                (device->cuda_runtime_version / 1000),
                (device->cuda_runtime_version % 100) / 10);
        oskar_log_value(log, p, 1, "CUDA driver version", "%d.%d",
                (device->cuda_driver_version / 1000),
                (device->cuda_driver_version % 100) / 10);
    }
    else if (device->platform_type == 'O')
    {
        oskar_log_value(log, p, 1, "OpenCL version", "%s", device->cl_version);
        oskar_log_value(log, p, 1, "OpenCL driver version", "%s",
                device->cl_driver_version);
    }
    if (device->compute_capability[0] > 0)
        oskar_log_value(log, p, 1, "CUDA compute capability", "%d.%d",
                device->compute_capability[0], device->compute_capability[1]);
    oskar_log_value(log, p, 1, "Supports double precision", "%s",
            device->supports_double ? "true" : "false");
    if (device->platform_type == 'O')
    {
        oskar_log_value(log, p, 1, "Supports 32-bit atomics", "%s",
                device->supports_atomic32 ? "true" : "false");
        oskar_log_value(log, p, 1, "Supports 64-bit atomics", "%s",
                device->supports_atomic64 ? "true" : "false");
    }
    if (device->global_mem_free_size > 0)
        oskar_log_value(log, p, 1, "Free global memory (MiB)", "%.1f",
                device->global_mem_free_size / (1024. * 1024.));
    oskar_log_value(log, p, 1, "Global memory size (MiB)", "%.0f",
            device->global_mem_size / (1024. * 1024.));
    if (device->max_mem_alloc_size > 0)
        oskar_log_value(log, p, 1, "Max allocation size (MiB)", "%.0f",
                device->max_mem_alloc_size / (1024. * 1024.));
    if (device->global_mem_cache_size > 0)
        oskar_log_value(log, p, 1, "Global memory cache size (kiB)", "%.0f",
                device->global_mem_cache_size / 1024.);
    oskar_log_value(log, p, 1, "Local/shared memory size (kiB)", "%.0f",
            device->local_mem_size / 1024.);
    oskar_log_value(log, p, 1, "Number of compute units", "%d",
            device->max_compute_units);
    if (device->num_cores > 0)
        oskar_log_value(log, p, 1, "Number of cores", "%d",
                device->num_cores);
    oskar_log_value(log, p, 1, "Clock speed (MHz)", "%.0f",
            device->max_clock_freq_kHz / 1000.);
    if (device->memory_clock_freq_kHz > 0)
        oskar_log_value(log, p, 1, "Memory clock speed (MHz)", "%.0f",
                device->memory_clock_freq_kHz / 1000.);
    if (device->memory_bus_width > 0)
        oskar_log_value(log, p, 1, "Memory bus width", "%d-bit",
                device->memory_bus_width);
    if (device->num_registers > 0)
        oskar_log_value(log, p, 1, "Registers per block", "%d",
                device->num_registers);
    if (device->warp_size > 0)
        oskar_log_value(log, p, 1, "Warp size", "%d", device->warp_size);
    oskar_log_value(log, p, 1, "Max work group size", "%d",
            device->max_work_group_size);
    oskar_log_value(log, p, 1, "Max work item sizes",
            "(%d x %d x %d)",
            device->max_local_size[0],
            device->max_local_size[1],
            device->max_local_size[2]);
}

void oskar_device_log_mem(int location, int depth, int id, oskar_Log* log)
{
    const double megabyte = 1024. * 1024.;
    if (location == OSKAR_GPU)
    {
        oskar_Device* device = oskar_device_create();
        device->index = id;
        oskar_device_get_info_cuda(device);
        const double mem_free  = (double) device->global_mem_free_size;
        const double mem_total = (double) device->global_mem_size;
        oskar_log_message(log, 'M', depth,
                "CUDA device %d [%s] memory is %.1f%% (%.1f/%.1f MiB) used.",
                id, device->name, 100. * (1. - (mem_free / mem_total)),
                (mem_total - mem_free) / megabyte, mem_total / megabyte);
        oskar_device_free(device);
    }
    else if (location & OSKAR_CL)
    {
        const oskar_Device* device = oskar_device_cl(id);
        if (device)
            oskar_log_message(log, 'M', depth,
                    "OpenCL device %d [%s] has %.1f MiB (max alloc. %.1f MiB).",
                    id, device->name, device->global_mem_size / megabyte,
                    device->max_mem_alloc_size / megabyte);
    }
}

#ifdef __cplusplus
}
#endif
