/*
 * Copyright (c) 2011, The University of Oxford
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


#include "station/oskar_StationModel.h"
#include "math/cudak/oskar_cudak_vec_scale_rr.h"
#include "station/oskar_station_model_copy.h"
#include "station/oskar_station_model_free.h"
#include "station/oskar_station_model_init.h"
#include "station/oskar_station_model_load.h"
#include "station/oskar_station_model_location.h"
#include "station/oskar_station_model_resize.h"
#include "station/oskar_station_model_check_mem.h"
#include "station/oskar_station_model_scale_coords.h"
#include "station/oskar_station_model_type.h"
#include <cuda_runtime_api.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


oskar_StationModel::oskar_StationModel(int type, int location, int n_elements)
: n_elements(n_elements),
  child(NULL),
  parent(NULL),
  element_pattern(NULL)
{
    if (oskar_station_model_init(this, type, location, n_elements))
        throw "Error in oskar_station_model_init";
}

oskar_StationModel::oskar_StationModel(const oskar_StationModel* other,
        int location)
: n_elements(other->n_elements),
  child(NULL),
  parent(NULL),
  element_pattern(NULL)
{
    if (oskar_station_model_init(this, other->x.type(), location,
            other->n_elements))
        throw "error in oskar_station_model_init";
    if (oskar_station_model_copy(this, other))
        throw "error in oskar_station_model_copy";
}

oskar_StationModel::~oskar_StationModel()
{
    oskar_station_model_free(this);
}

int oskar_StationModel::copy_to(oskar_StationModel* other)
{
    return oskar_station_model_copy(other, this); // Copy this to other.
}

int oskar_StationModel::load(const char* filename)
{
    return oskar_station_model_load(this, filename);
}

int oskar_StationModel::location() const
{
    return oskar_station_model_location(this);
}

int oskar_StationModel::resize(int n_elements)
{
    return oskar_station_model_resize(this, n_elements);
}

int oskar_StationModel::scale_coords_to_wavenumbers(const double frequency_hz)
{
    if (coord_units != OSKAR_METRES)
        return OSKAR_ERR_BAD_UNITS;
    coord_units = OSKAR_WAVENUMBERS;
    const double metres_to_wavenumbers = 2.0 * M_PI * frequency_hz / 299792458.0;
    return oskar_station_model_scale_coords(this, metres_to_wavenumbers);
}

int oskar_StationModel::type() const
{
    return oskar_station_model_type(this);
}

int oskar_StationModel::coord_type() const
{
    return oskar_station_model_coord_type(this);
}

int oskar_StationModel::coord_location() const
{
    return oskar_station_model_coord_location(this);
}




















// DEPRECATED

#ifdef __cplusplus
extern "C" {
#endif

void oskar_station_model_copy_to_device_d(const oskar_StationModel_d* h_stations,
        const unsigned num_stations, oskar_StationModel_d* hd_stations)
{
    // Allocate and copy memory for each station.
    for (unsigned i = 0; i < num_stations; ++i)
    {
        hd_stations[i].num_antennas = h_stations[i].num_antennas;
        size_t mem_size = hd_stations[i].num_antennas * sizeof(double);
        cudaMalloc((void**)&(hd_stations[i].antenna_x), mem_size);
        cudaMalloc((void**)&(hd_stations[i].antenna_y), mem_size);
        cudaMemcpy(hd_stations[i].antenna_x, h_stations[i].antenna_x, mem_size,
                cudaMemcpyHostToDevice);
        cudaMemcpy(hd_stations[i].antenna_y, h_stations[i].antenna_y, mem_size,
                cudaMemcpyHostToDevice);
    }
}


void oskar_station_model_copy_to_device_f(const oskar_StationModel_f* h_stations,
        const unsigned num_stations, oskar_StationModel_f* hd_stations)
{
    // Allocate and copy memory for each station.
    for (unsigned i = 0; i < num_stations; ++i)
    {
        hd_stations[i].num_antennas = h_stations[i].num_antennas;
        size_t mem_size = hd_stations[i].num_antennas * sizeof(float);
        cudaMalloc((void**)&(hd_stations[i].antenna_x), mem_size);
        cudaMalloc((void**)&(hd_stations[i].antenna_y), mem_size);
        cudaMemcpy(hd_stations[i].antenna_x, h_stations[i].antenna_x, mem_size,
                cudaMemcpyHostToDevice);
        cudaMemcpy(hd_stations[i].antenna_y, h_stations[i].antenna_y, mem_size,
                cudaMemcpyHostToDevice);
    }
}

void oskar_station_model_scale_coords_d(const unsigned num_stations,
        oskar_StationModel_d* hd_stations, const double value)
{
    int num_threads = 256;
    for (unsigned i = 0; i < num_stations; ++i)
    {
        int num_antennas = hd_stations[i].num_antennas;
        int num_blocks  = (int)ceil((double) num_antennas / num_threads);
        oskar_cudak_vec_scale_rr_d <<< num_blocks, num_threads >>>
                (num_antennas, value, hd_stations[i].antenna_x);
        oskar_cudak_vec_scale_rr_d <<< num_blocks, num_threads >>>
                (num_antennas, value, hd_stations[i].antenna_y);
    }
}


void oskar_station_model_scale_coords_f(const unsigned num_stations,
        oskar_StationModel_f* hd_stations, const float value)
{
    int num_threads = 256;
    for (unsigned i = 0; i < num_stations; ++i)
    {
        int num_antennas = hd_stations[i].num_antennas;
        int num_blocks  = (int)ceil((float) num_antennas / num_threads);
        oskar_cudak_vec_scale_rr_f <<< num_blocks, num_threads >>>
                (num_antennas, value, hd_stations[i].antenna_x);
        oskar_cudak_vec_scale_rr_f <<< num_blocks, num_threads >>>
                (num_antennas, value, hd_stations[i].antenna_y);
    }
}


#ifdef __cplusplus
}
#endif
