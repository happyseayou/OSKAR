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

#include "cuda/test/BeamPatternTest.h"
#include "cuda/beamPattern2dHorizontalGeometric.h"
#include "math/core/SphericalPositions.h"
#include "math/core/GridPositions.h"
#include "math/core/Matrix3.h"
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.1415926535
#endif

#define DEG2RAD (M_PI / 180.0)
#define RAD2DEG (180.0 / M_PI)
#define C_0 299792458.0

#define TIMER_ENABLE 1
#include "utility/timer.h"

// Register the test class.
CPPUNIT_TEST_SUITE_REGISTRATION(BeamPatternTest);

/**
 * @details
 * Sets up the context before running each test method.
 */
void BeamPatternTest::setUp()
{
}

/**
 * @details
 * Clean up routine called after each test is run.
 */
void BeamPatternTest::tearDown()
{
}

/**
 * @details
 * Tests beam pattern creation using CUDA.
 */
void BeamPatternTest::test_regular()
{
    // Generate square array of antenna positions.
    const int na = 100;
    const float sep = 0.15; // Antenna separation, metres.
    const float halfArraySize = (na - 1) * sep / 2.0;
    std::vector<float> ax(na * na), ay(na * na); // Antenna (x,y) positions.
    for (int x = 0; x < na; ++x) {
        for (int y = 0; y < na; ++y) {
            int i = y + x * na;
            ax[i] = x * sep - halfArraySize;
            ay[i] = y * sep - halfArraySize;
        }
    }

    // Generate test source positions.
    float beamAz = 0;  // Beam azimuth.
    float beamEl = 50; // Beam elevation.
    SphericalPositions<float> pos (
            beamAz * DEG2RAD, beamEl * DEG2RAD, // Centre.
            30 * DEG2RAD, 30 * DEG2RAD, // Half-widths.
            0.2 * DEG2RAD, 0.2 * DEG2RAD); // Spacings.
    unsigned ns = pos.generate(0, 0); // No. of sources.
    std::vector<float> slon(ns), slat(ns);
    pos.generate(&slon[0], &slat[0]);

    // Call CUDA beam pattern generator.
    float freq = 1e9; // Observing frequency, Hertz.
    std::vector<float> image(ns * 2); // Beam pattern real & imaginary values.
    TIMER_START
    beamPattern2dHorizontalGeometric(na*na, &ax[0], &ay[0], ns, &slon[0],
            &slat[0], beamAz * DEG2RAD, beamEl * DEG2RAD,
            2 * M_PI * (freq / C_0), &image[0]);
    TIMER_STOP("Finished beam pattern (%d element regular array, %d points)",
            na*na, ns);

    // Write image data to file.
    FILE* file = fopen("beamPattern2dHorizontalGeometricRegular.dat", "w");
    for (unsigned s = 0; s < ns; ++s) {
        fprintf(file, "%12.3f%12.3f%16.4e%16.4e\n",
                slon[s] * RAD2DEG, slat[s] * RAD2DEG, image[2*s], image[2*s+1]);
    }
    fclose(file);
}

/**
 * @details
 * Tests beam pattern creation using CUDA.
 */
void BeamPatternTest::test_perturbed()
{
    // Generate array of antenna positions.
    int seed = 10;
    float radius = 15; // metres. (was 125)
    float xs = 1.4, ys = 1.4, xe = 0.3, ye = 0.3; // separations, errors.

    int na = GridPositions::circular(seed, radius, xs, ys, xe, ye);
    std::vector<float> ax(na), ay(na), az(na); // Antenna positions.
    GridPositions::circular(seed, radius, xs, ys, xe, ye, &ax[0], &ay[0]);

    // Rotate around z.
    float matrix[9];
    Matrix3::rotationZ(matrix, float(0 * DEG2RAD));
    Matrix3::transformPoints(matrix, na, &ax[0], &ay[0], &az[0]);

    // Write antenna positions to file.
    FILE* file = fopen("arrayRandom.dat", "w");
    for (int a = 0; a < na; ++a) {
        fprintf(file, "%12.3f%12.3f\n", ax[a], ay[a]);
    }
    fclose(file);

    // Set beam direction.
    float beamAz = 0;  // Beam azimuth.
    float beamEl = 90; // Beam elevation.

    // Generate test source positions for a square image.
//    SphericalPositions<float> pos (
//            beamAz * DEG2RAD, beamEl * DEG2RAD, // Centre.
//            30 * DEG2RAD, 30 * DEG2RAD, // Half-widths.
//            0.2 * DEG2RAD, 0.2 * DEG2RAD); // Spacings.

    // Generate test source positions for the hemisphere.
    SphericalPositions<float> pos (
            180 * DEG2RAD, 45 * DEG2RAD, // Centre.
            180 * DEG2RAD, 45 * DEG2RAD, // Half-widths.
            //0.03 * DEG2RAD, 0.03 * DEG2RAD, // Spacings.
            0.2 * DEG2RAD, 0.2 * DEG2RAD, // Spacings.
            0.0, true, false, true, true,
            SphericalPositions<float>::PROJECTION_NONE);

    int ns = 1 + pos.generate(0, 0); // No. of sources (add a point at zenith).
    std::vector<float> slon(ns), slat(ns);
    slon[0] = 0; slat[0] = 90 * DEG2RAD; // Add a point at zenith.
    pos.generate(&slon[1], &slat[1]); // Add a point at zenith.
    std::vector<float> image(ns * 2); // Beam pattern real & imaginary values.

    // Call CUDA beam pattern generator.
    int nf = 7; // Number of frequencies.
    float freq[] = {70, 115, 150, 200, 240, 300, 450}; // Frequencies in MHz.
    for (int f = 0; f < nf; ++f) {
        TIMER_START
        beamPattern2dHorizontalGeometric(na, &ax[0], &ay[0], ns, &slon[0],
                &slat[0], beamAz * DEG2RAD, beamEl * DEG2RAD,
                2 * M_PI * (freq[f] * 1e6 / C_0), &image[0]);
        TIMER_STOP("Finished beam pattern (%.0f MHz)", freq[f]);

        // Write image data to file.
        char fname[200];
        snprintf(fname, 200, "beamPattern_%.0f.dat", freq[f]);
        file = fopen(fname, "w");
        for (int s = 0; s < ns; ++s) {
            fprintf(file, "%12.3f%12.3f%16.4e%16.4e\n",
                    slon[s] * RAD2DEG, slat[s] * RAD2DEG, image[2*s], image[2*s+1]);
        }
        fclose(file);
    }
}

/**
 * @details
 * Tests beam pattern creation using CUDA.
 */
void BeamPatternTest::test_scattered()
{
    // Generate array of antenna positions.
    int seed = 10;
    float radius = 15; // metres. (was 125)
    float xs = 1.4, ys = 1.4, xe = 0.3, ye = 0.3; // separations, errors.

    int na = GridPositions::circular(seed, radius, xs, ys, xe, ye);
    std::vector<float> ax(na), ay(na), az(na); // Antenna positions.
    GridPositions::circular(seed, radius, xs, ys, xe, ye, &ax[0], &ay[0]);

    // Rotate around z.
    float matrix[9];
    Matrix3::rotationZ(matrix, float(0 * DEG2RAD));
    Matrix3::transformPoints(matrix, na, &ax[0], &ay[0], &az[0]);

    // Write antenna positions to file.
    FILE* file = fopen("arrayRandomScattered.dat", "w");
    for (int a = 0; a < na; ++a) {
        fprintf(file, "%12.3f%12.3f\n", ax[a], ay[a]);
    }
    fclose(file);

    // Set beam direction.
    float beamAz = 0;  // Beam azimuth.
    float beamEl = 90; // Beam elevation.

//    SphericalPositions<float> pos (
//            beamAz * DEG2RAD, beamEl * DEG2RAD, // Centre.
//            30 * DEG2RAD, 30 * DEG2RAD, // Half-widths.
//            0.5 * DEG2RAD, 0.5 * DEG2RAD); // Spacings.
//    int ns = pos.generate(0, 0); // No. of sources.
//    std::vector<float> slon(ns), slat(ns);
//    pos.generate(&slon[0], &slat[0]);

    float slat[] = {1, 0.75, 1.3, 0.8,
            1.5708, 1.5621, 1.5533, 1.5446, 1.5359,
            1.5272, 1.5184, 1.5097, 1.5010};
    float slon[] = {-2.8304, -0.0655, -1.9320, -2.3682,
            0, 0, 0, 0, 0,
            0, 0, 0, 0};
    int ns = sizeof(slon) / sizeof(float);
    std::vector<float> image(ns * 2); // Beam pattern real & imaginary values.

    // Call CUDA beam pattern generator.
    int nf = 7; // Number of frequencies.
    float freq[] = {70, 115, 150, 200, 240, 300, 450}; // Frequencies in MHz.
    for (int f = 0; f < nf; ++f) {
        TIMER_START
        beamPattern2dHorizontalGeometric(na, &ax[0], &ay[0], ns, &slon[0],
                &slat[0], beamAz * DEG2RAD, beamEl * DEG2RAD,
                2 * M_PI * (freq[f] * 1e6 / C_0), &image[0]);
        TIMER_STOP("Finished beam pattern (%.0f MHz)", freq[f]);

        // Write image data to file.
        char fname[200];
        snprintf(fname, 200, "beamPatternScattered_%.0f.dat", freq[f]);
        file = fopen(fname, "w");
        for (int s = 0; s < ns; ++s) {
            fprintf(file, "%12.3f%12.3f%16.4e%16.4e\n",
                    slon[s] * RAD2DEG, slat[s] * RAD2DEG, image[2*s], image[2*s+1]);
        }
        fclose(file);
    }
}

/**
 * @details
 * Tests beam pattern creation using CUDA.
 */
void BeamPatternTest::test_random()
{
    // Set antenna positions.
    const int na = 1000;
    float ax[] = {13.108,17.494,11.439,0.045236,3.1559,8.5675,-19.412,3.0974,
            4.3002,-3.9595,4.0163,-14.938,-12.636,-17.871,12.924,17.174,
            -18.455,-20.459,-18.943,-18.994,5.1591,-18.886,2.7326,-0.68578,
            -5.2299,-13.875,-6.3902,-1.0571,-8.1681,-4.5398,23.639,10.079,
            -18.196,19.846,1.5362,13.309,-19.414,-8.6324,10.004,24.363,1.3876,
            10.889,12.182,-1.8697,29.08,7.3266,13.549,-0.29534,11.199,-29.109,
            -6.8505,-12.703,25.731,-4.1342,21.234,12.624,-1.6345,-5.4595,
            12.306,-30.126,-11.942,9.6346,-12.827,-1.0421,-17.158,-23.917,
            -3.7213,-11.152,22.581,20.217,20.707,27.17,0.69184,-4.8173,21.74,
            14.945,-17.868,-16.103,1.5318,-1.5637,-23.191,-23.075,-28.319,
            29.756,4.0069,-16.533,2.2065,13.389,-14.586,-20.334,-18.976,
            6.2746,-19.493,12.675,-2.787,4.7093,-5.7182,14.512,-2.385,-5.2762,
            9.831,-4.0804,1.6057,-24.437,-16.814,-10.653,20.262,-24.681,10.636,
            -19.972,10.522,18.033,9.3174,-9.2694,-0.34759,-9.1782,-18.452,
            -8.6579,25.563,-19.501,-25.289,21.926,-6.9708,-2.4275,11.484,
            1.8635,-7.7361,9.807,-10.641,6.8508,0.36013,25.176,2.9751,18.897,
            -16.264,-9.5607,-16.362,16.99,2.9189,2.5297,1.2975,-12.685,16.505,
            -10.681,-15.251,30.122,9.3571,1.6246,-23.115,18.648,-7.9549,
            -3.5373,-1.6087,-17.785,7.915,-4.9557,4.6452,-6.7423,8.8011,
            -16.076,-16.776,15.903,-13.266,11.66,-5.0133,15.179,-10.575,1.0244,
            -21.857,11.139,6.3417,-7.2734,-14.761,-22.259,-12.499,3.0496,
            -24.48,7.1322,-17.949,-20.207,8.8793,-22.624,9.2697,27.861,-24.964,
            -11.645,-5.0482,17.106,22.697,14.615,1.5834,5.4982,24.016,18.679,
            22.23,-16.375,7.0268,-26.716,8.325,6.1947,16.112,-25.442,16.39,
            20.551,3.6707,-0.44698,19.625,13.572,-10.457,21.724,0.93623,
            -7.6223,-8.8549,11.388,12.822,-13.844,27.444,-5.7453,-30.236,
            6.5793,29.878,-0.93347,1.6933,7.0512,-29.401,17.274,-1.7302,
            -8.9968,6.5303,5.9513,10.255,4.6376,-8.6336,-29.72,-8.6845,3.8079,
            -26.416,-12.957,24.044,-14.389,4.8435,21.97,-0.22124,-16.337,
            18.765,-19.508,-12.926,16.263,3.4985,-19.068,-21.819,-3.6772,
            11.334,-2.265,-13.082,5.5519,22.724,20.681,13.036,-3.3874,9.0086,
            -22.187,11.247,-22.066,18.218,24.919,-0.79931,-7.4455,6.7908,
            26.726,-7.0493,12.235,10.447,-18.128,-0.39062,10.162,6.138,24.067,
            -17.349,13.633,-6.3685,21.714,9.0837,0.13083,18.369,26.647,6.4701,
            4.3847,28.409,-24.586,-15.353,-11.231,-12.127,-23.083,-25.603,
            20.529,18.022,-1.1877,28.951,-10.236,-4.0037,9.8001,-6.1027,
            5.2548,18.946,12.977,-6.7395,8.2169,-4.0338,-23.576,3.6372,-18.012,
            3.5259,11.37,17.376,-23.429,8.1147,-21.751,7.9547,-15.423,-15.318,
            18.016,2.3078,-2.5025,-24.26,-15.724,-23.273,-2.0643,-14.096,
            -16.465,-13.965,27.327,5.5578,-6.5539,-9.4858,20.371,22.624,
            -18.987,20.133,1.9588,0.70325,-8.214,12.691,6.6837,-0.54709,4.0904,
            4.5086,1.7492,12.889,13.013,7.2469,-27.605,28.553,18.398,-15.181,
            -7.1849,-27.407,15.2,0.18912,6.1706,13.165,6.2674,-26.339,28.545,
            19.436,-30.364,8.3022,-10.307,-4.9257,4.4101,-8.7508,-12.781,
            -1.8134,-17.765,-1.8925,-10.049,-5.7675,15.321,-21.626,11.778,
            -3.8186,-11.472,24.06,-15.177,-12.445,15.614,-10.142,-6.2095,
            28.528,-10.372,13.695,-0.48431,-1.5441,-15.974,-22.043,20.84,
            24.867,28.678,15.1,26.193,-20.166,-2.9035,2.5353,-27.342,-0.78547,
            -3.0536,-19.269,-10.88,17.002,14.014,-22.793,-1.4858,20.363,
            -16.764,6.4684,-16.857,-11.508,-21.437,3.5769,2.4965,-14.962,
            -7.5148,-15.049,27.524,-4.2154,-5.0419,19.051,12.51,-5.6214,
            -17.468,-10.178,22.589,29.566,-9.1384,-11.17,-10.977,-16.151,
            -12.354,0.35914,15.282,0.53614,-6.4482,5.0545,-18.512,1.8056,
            -3.4921,-15.746,-20.133,-10.779,2.8723,22.767,-9.3555,-3.6453,
            17.765,26.039,1.8974,-13.097,17.378,-25.373,-17.086,11.397,9.5251,
            28.271,30.397,-8.3233,17.203,-4.4884,17.297,-9.2103,8.011,4.8758,
            -7.3195,10.781,-4.9999,-14.373,-9.5479,-12.682,16.987,2.377,
            11.654,4.1822,-16.679,-6.4059,-21.131,24.106,-2.9503,-7.391,-3.079,
            -27.008,21.149,-27.539,-8.8649,-29.573,-20.748,4.9365,10.82,16.915,
            1.9984,-7.8024,-14.226,27.949,-11.325,-24.697,-22.441,23.795,
            -2.5735,-17.242,25.742,23.096,24.24,-0.89987,-29.241,-11.079,
            -28.795,4.4364,-5.6378,-23.965,-11.863,7.4577,8.3188,-28.173,
            12.326,0.94009,1.0734,15.91,-21.915,16.037,1.9617,-23.626,23.405,
            7.3621,16.92,22.664,-22.809,-11.267,-13.133,22.464,-14.098,-18.611,
            -20.038,-17.695,0.95318,13.704,-27.378,-21.834,14.449,-2.2727,
            4.9857,-9.8942,-18.098,19.334,19.462,-17.64,15.572,-5.4358,17.261,
            1.7816,-10.562,-20.326,25.333,19.572,-11.549,11.275,-27.386,20.02,
            14.154,26.229,-27.938,14.564,3.5777,19.007,3.3197,8.5543,14.905,
            -26.16,-2.6075,-5.2749,-7.3014,-4.8659,-3.2928,-8.3597,2.1128,
            -7.4388,-13.173,-26.843,-0.35411,-14.5,9.4291,16.989,-23.792,
            -21.109,-5.5338,5.4035,-14.71,-19.622,14.528,-1.8836,18.822,
            -20.205,24.381,21.498,-25.82,7.1083,3.8021,21.696,-14.927,8.6443,
            28.531,18.007,-16.72,7.3163,26.605,-13.103,1.7875,15.294,-24.37,
            -27.542,9.0517,-6.8166,-10.117,15.018,-1.6329,-25.379,-14.138,
            5.0343,-6.5733,-14.688,25.706,13.476,-0.96419,0.63346,-6.8077,
            -25.168,8.6056,14.19,6.5561,21.955,9.7443,0.023613,11.081,-25.882,
            10.086,-23.957,0.0062583,-14.655,-1.7246,24.9,23.247,-6.3182,
            19.596,0.69035,3.1592,-5.5326,26.507,6.9482,-20.457,-28.721,4.1633,
            -10.4,-13.158,5.7674,18.274,-5.4832,14.777,20.075,-2.1944,-12.229,
            -11.233,-8.9535,9.1794,14.132,-14.083,-2.4669,11.575,13.135,
            -15.085,-22.387,-10.176,0.087708,-21.844,-13.203,-5.4296,12.964,
            14.937,6.207,-2.3167,15.657,-0.1873,8.5845,7.7324,-23.942,-7.2328,
            20.857,4.5767,-19.574,-17.115,-20.575,19.99,-9.71,9.911,16.497,
            -0.2271,-21.98,3.5615,-3.6499,20.21,-3.4435,-14.104,16.506,10.336,
            22.079,-12.608,3.6169,11.589,-8.5259,6.0106,6.9391,2.444,23.825,
            6.7434,21.578,-3.6147,-10.352,1.0055,-7.2824,26.801,-14.107,-16.69,
            1.5715,-12.328,8.5688,-21.75,11.348,8.8118,8.4808,-13.218,-12.667,
            0.46249,-4.6954,7.5084,-14.168,25.663,-9.39,-19.013,-11.327,8.2377,
            24.447,-5.0358,2.8146,-0.073031,0.69014,15.834,-1.2638,-5.7243,
            3.1316,4.8138,-3.4639,-14.502,-16.472,-16.036,-4.2474,-8.1484,
            10.148,-21.17,-17.787,4.3649,0.13287,5.4449,9.3467,-1.2546,18.638,
            -10.367,4.8638,-19.12,9.3093,13.603,20.482,10.502,10.483,2.0457,
            23.583,-8.0246,13.181,-1.8788,2.7924,2.6808,10.423,-26.943,-3.4983,
            -26.021,-30.155,-26.233,8.4594,-25.906,-19.991,15.437,-23.433,
            -11.527,-3.3603,24.618,26.583,-12.295,16.752,-8.3683,14.862,20.692,
            -1.4169,24.366,12.411,-4.0553,4.4033,5.741,25.04,18.488,15.997,
            -1.9267,9.8203,-8.6444,-11.104,-21.942,11.38,-4.0408,14.186,
            -16.002,24.865,-4.861,-23.577,-20.169,13.908,17.126,9.3817,6.6203,
            -1.2634,19.954,-20.469,-17.688,27.316,-21.767,3.7934,21.901,
            -13.312, -18.973,-6.6497,-21.384,-20.397,14.868,-24.795,11.594,
            -4.1199,11.011,17.973,-27.846,-20.547,-16.199,20.844,-16.199,
            -21.882,-9.6521,3.3977,-25.816,-24.981,19.726,-2.1152,8.6703,
            -14.935,6.5794,18.26,23.382,15.172,17.139,11.61,14.337,-26.155,
            -5.9017,-14.993,-14.245,9.6406,17.334,12.17,22.054,-2.7985,
            23.493,21.971,-9.8148,8.7599,-19.605,-19.042,-4.2326,-13.298,
            22.258,-8.3434,25.8,2.0918,-18.062,-2.5575,15.423,-3.7156,27.155,
            -8.6706,-11.775,5.273,-15.791,7.471,26.636,-12.217,-28.86,12.319,
            8.3502,18.218,21.306,21.688,16.715,10.195,11.868,-7.2163,23.127,
            -17.455,3.4575,-2.3981,-7.7589,-9.2027,3.1589,-28.355,-0.35433,
            4.9682,2.7841,3.1387,25.251,0.017842,18.954,14.987,-14.418,-6.3241,
            22.929,-28.634,-15.263,12.689,8.1253,-15.676,15.854,-11.592,
            -22.895,-21.849,1.0291,-26.035,-23.749,-2.8952,-15.626,8.1002,
            7.1529,24.904,-8.9311,-14.288,20.122,18.929,7.3783,24.451,6.0265,
            1.3711,27.671,16.127,5.7223,-3.5682,-12.19,13.114,-8.0129,23.553,
            -6.5719,-13.284,13.873,-7.8314,1.3801,-20.876,17.986,-20.765,
            -26.588,-28.059,-27.316,-9.254,10.492,-1.0466,-6.3671,20.879,
            26.419,-23.215,11.844,20.889,-18.655,-8.6346,-8.8656,0.22355,
            -20.803,-4.9385,16.276,-4.0271,27.906,0.7501,-10.497,27.227,
            29.139};
    float ay[] = {10.024,-13.319,13.565,22.288,27.986,22.775,4.5243,29.567,
            10.459,-4.9821,-24.313,-22.38,-20.057,11.117,2.1907,18.026,-22.008,
            -3.9055,16.634,19.061,-6.5986,-11.243,-13.33,-25.388,-16.09,13.462,
            -20.543,7.1935,-14.022,21.517,7.6922,23.143,23.061,-17.743,-10.374,
            15.071,21.691,-6.2786,-6.8376,1.9431,12.929,-18.872,11.384,2.973,
            3.1144,15.71,-12.163,17.65,-23.175,2.2573,-7.7683,26.439,-4.5596,
            11.619,-6.8529,-5.3059,19.697,-0.6073,-2.2266,-4.2685,-26.932,
            -0.0080816,-22.871,12.48,-4.1949,-7.9407,2.2863,24.968,-0.67086,
            -4.8362,-10.39,0.50135,4.3139,-30.044,10.431,22.43,4.3754,14.188,
            -22.786,-20.592,15.111,1.645,-5.0845,-3.9736,1.1873,-9.8614,
            0.14716,5.2498,-10.452,-1.3404,1.5482,-15.816,-2.7583,0.74123,
            21.999,-1.786,19.105,-10.809,13.802,5.217,-21.306,-6.6987,-11.967,
            -15.345,-1.6553,-21.24,21.547,8.2627,-13.763,-5.2649,-25.269,
            10.466,18.211,-12.064,0.47821,-15.728,20.498,-4.4533,-7.7114,
            -16.717,-10.544,2.8867,-0.84908,-24.073,27.285,16.568,-9.0916,
            6.0672,-8.0827,12.888,25.623,5.0696,2.4308,14.372,18.307,12.06,
            25.647,0.43562,-18.426,20.837,-26.212,21.226,-20.856,2.9221,6.8708,
            -0.91286,9.5908,-1.7907,9.9982,5.8286,2.1714,19.476,23.301,-20.012,
            3.1625,-19.697,27.548,26.464,11.8,-11.625,16.932,-1.5498,-18.559,
            -27.777,-2.7716,14.274,0.97908,-3.1482,-3.0731,4.8699,5.8067,
            18.109,-1.7923,-5.9309,14.674,24.874,2.5911,8.0711,-16.779,-7.7852,
            -26.197,12.125,16.093,-5.0506,-13.245,-24.231,25.667,-10.724,
            -17.068,9.993,-16.932,-25.735,14.396,-11.813,13.627,1.9039,19.114,
            -0.25199,5.9283,-5.5305,8.4402,4.4781,11.659,15.705,8.7857,-9.3251,
            9.039,-4.0953,15.498,-11.563,-8.8557,-3.2157,6.1454,7.9998,7.3066,
            -13.557,3.7942,-6.0587,-0.33991,0.94585,4.4338,-6.2791,-14.922,
            -1.3273,7.7663,-15.842,10.783,20.038,-22.334,23.929,24.789,23.349,
            -26.008,-6.2158,-21.161,-22.301,2.947,2.7421,-4.5833,-24.072,
            -20.966,-2.4464,-28.309,21.451,16.535,-14.886,24.382,26.067,5.491,
            8.2021,5.7026,4.008,22.149,-0.052007,-5.1445,15.692,4.1142,-14.593,
            26.219,17.034,-5.5304,-18.74,-20.484,-14.881,-17.476,-1.8943,
            -26.923,15.764,-11.668,-9.3862,-28.343,-14.548,-3.0931,-14.029,
            15.375,1.5931,25.897,16.381,-8.6631,-22.212,-25.2,8.8607,26.491,
            -15.148,1.151,7.9167,11.121,16.604,-8.6179,9.7801,16.491,-18.681,
            -14.604,-10.734,6.5109,7.8614,12.213,25.305,-6.475,-23.145,-9.8921,
            13.989,14.331,3.4093,-22.198,23.296,11.84,-27.982,-23.024,4.5984,
            -0.33415,-23.705,-26.111,16.456,-22.999,16.607,-23.615,-13.079,
            -7.129,9.9315,-4.2838,-24.399,-19.799,6.7601,13.207,-13.756,
            -4.7243,-8.5999,-21.052,23.93,-27.34,-1.9434,-9.2398,-4.6459,
            -19.925,12.765,-14.261,6.6067,0.084278,18.984,11.522,14.096,21.266,
            -27.491,30.111,-10.31,7.3169,1.6316,17.016,19.003,28.313,-2.1816,
            1.4684,-7.4547,20.131,28.637,11.996,-7.7731,-19.445,20.796,-8.9832,
            -13.38,-5.435,-10.212,-19.264,-2.6431,-9.1843,9.6124,-7.9035,
            13.862,-28.569,16.496,26.922,13.44,-11.581,-10.433,-9.9126,5.6339,
            -10.547,-11.025,27.336,-6.8576,10.875,22.285,1.3556,1.4793,26.485,
            3.2455,8.1411,22.628,-24.359,-21.754,1.374,-19.448,-8.2941,20.131,
            -14.825,10.574,-26.392,12.702,0.27793,-29.433,-8.0335,-11.862,
            21.058,29.818,12.678,-15.502,21.872,-15.248,8.2477,-2.2059,-8.8022,
            -17.703,-17.345,20.03,8.5922,-16.938,-29.927,14.877,12.38,20.057,
            23.722,-6.722,0.20162,7.6367,18.118,-7.5651,-14.395,8.9545,-27.432,
            16.647,-2.5057,-24.177,21.275,13.045,4.4562,-12.21,9.0715,24.275,
            -29.543,-12.638,19.343,-7.2799,26.994,-21.199,-7.7721,-9.9908,
            18.583,-2.59,-10.55,17.394,-12.579,-4.7582,6.4241,-28.092,-9.528,
            13.504,-8.6568,-12.806,-15.973,3.1841,5.6213,1.4328,22.977,20.241,
            24.32,-3.0911,-1.9473,17.122,29.135,-19.009,12.089,-24.154,1.1616,
            -17.697,9.7753,24.208,-25.242,-0.42112,-4.0221,-6.5596,22.846,
            11.935,-7.6772,-15.923,9.8321,-26.563,8.2205,-19.71,6.6901,21.619,
            3.7256,1.626,4.9296,-8.5432,-18.508,8.7732,-16.999,8.1004,-3.5956,
            11.173,-18.012,19.571,-13.156,-17.5,6.8553,0.43965,-8.8975,17.799,
            4.8089,-1.6505,16.938,-9.0949,25.365,16.7,-12.121,-3.5057,4.7904,
            25.137,-7.5237,-25.427,-5.3325,21.211,-9.5996,4.1505,-24.863,
            5.6575,18.127,12.636,-2.9062,5.3072,-18.621,-16.192,-9.3862,
            -6.6123,-3.9696,18.996,-4.3561,-12.169,18.235,30.057,12.701,5.1959,
            14.623,-17.941,-5.2564,-29.506,4.4358,-0.49957,-13.457,-20.846,
            15.118,-19.535,29.324,3.8369,-6.6426,-25.861,17.719,9.7166,-15.825,
            -16.83,17.918,-9.8313,3.1957,0.98036,-0.98287,0.70216,7.8956,
            -16.54,20.093,12.911,-14.986,20.034,-14.653,25.006,27.177,-21.694,
            13.582,8.1302,26.478,23.709,5.663,5.8312,14.794,13.898,14.97,
            -10.121,-6.6003,6.013,8.2368,-18.339,-18.662,5.2969,-19.62,17.238,
            15.478,-10.152,15.487,-10.109,5.8366,-12.01,-25.281,3.6273,18.983,
            -15.37,21.111,-0.99786,7.9058,-22.391,26.752,-12.111,4.1888,
            -30.387,-22.682,-3.3932,10.149,-18.274,7.3964,7.1987,4.1832,
            -15.091,15.333,21.121,-12.068,-23.375,2.9649,2.6743,-19.804,
            -30.627,7.1309,-26.926,-0.66803,-13.248,-13.566,-19.829,-5.476,
            -1.6874,27.86,19.437,-3.8308,28.176,-1.7692,-17.817,-6.4342,
            -18.928,-6.4313,5.6871,21.068,23.152,-20.868,22.316,1.529,-3.1189,
            -9.7828,19.757,-3.3163,-13.609,-13.021,12.221,8.5269,-1.3021,
            -28.55,26.256,10.713,18.005,7.2783,5.9566,8.2438,-24.667,-0.90497,
            17.284,-10.18,10.068,-26.882,26.401,-1.0104,-0.68792,-1.8938,
            21.222,-2.9566,10.59,-16.433,-3.6399,17.751,-7.0334,-5.4748,-11.51,
            1.3788,9.7265,-6.2381,24.191,-1.1471,-15.354,-21.077,-15.529,6.833,
            -2.389,24.797,-28.997,14.969,-3.9497,16.21,19.058,-27.903,4.9045,
            -2.8589,-16.708,-12.028,-12.323,-15.735,22.864,26.543,2.9804,
            -0.62492,-23.974,22.145,-5.2138,0.10052,-7.9122,1.0919,9.8017,
            -5.4611,-13.508,-10.721,10.822,-7.9158,0.5093,28.539,-0.42838,
            -16.599,0.27411,6.3579,8.1633,-20.232,-24.956,19.361,24.069,
            -11.521,-4.6778,11.049,14.072,-8.9783,10.154,-28.28,14.208,3.7111,
            -12.924,11,2.5683,-24.398,18.792,-23.189,-22.071,-11.402,-27.942,
            23.26,-11.958,-25.46,-20.882,15.02,11.181,21.254,-19.887,-2.787,
            -8.0356,19.138,-0.24142,19.74,9.1877,21.711,-3.188,11.987,-18.176,
            4.5977,24.874,17.933,-5.0162,-17.443,3.5407,-2.7676,0.7261,3.6276,
            -28.233,7.0493,-23.707,-26.851,-8.3115,-1.3815,-1.9987,1.1451,
            -6.8611,-11.613,10.842,-22.508,-16.168,0.10161,-10.834,-19.558,
            -16.686,-13.808,-8.0441,6.842,4.3066,-20.932,-12.63,-13.533,
            -11.774,-18.506,-14.834,21.365,-4.1477,8.3104,24.435,-14.271,
            28.736,-15.879,-22.74,27.567,-21.456,-6.3613,-18.028,-6.567,
            -2.9128,12.108,-26.721,-13.877,2.9614,-2.3988,16.341,-8.1513,
            14.339,-16.618,1.5688,13.98,2.8933,9.4021,17.812,-19.924,12.042,
            -1.4895,-8.7974,-15.402,9.7143,-13.719,15.678,-4.8922,23.774,
            6.5342,14.931,-20.272,3.1765,-18.454,11.544,14.253,7.9815,2.7149,
            14.221,-28.381,1.0757,11.963,-6.2794,-3.5964,27.984,-25.651,2.5737,
            2.7866,-6.349,12.593,-8.5213,-4.0849,2.6263,12.972,9.0169,-18.176,
            -0.36609,-23.022,9.2025,-12.867,7.3933,12.364,9.1274,15.339,
            -14.401,-3.6347,23.482,14.366,18.2,-15.444,-20.712,-7.7896,-10.723,
            -21.462,-5.755,5.0594,-0.16112,-24.794,2.1666,28.285,-1.9626,
            1.9125,-23.684,-18.384,5.1228,17.922,6.0071,-21.523,-0.65968,
            -14.668,-17.535,-8.2583,2.4984,10.765,25.387,-6.0251,19.079,
            -11.154,-6.5084,-22.217,-24.623,3.0799,17.368,8.7562,10.332,
            -17.105,-9.5518,-14.76,-13.032,-7.632,-3.8802,-12.249,-19.515,
            -17.049,1.7959,-11.078,-9.157,-23.31,-29.602,-0.63276,-17.549,
            4.2241,13.658,-4.5302,15.146,9.3136,-9.4543,-13.91,-16.772,-21.747,
            -14.5,-3.3249,15.969,25.131,6.334,4.1464,23.844,6.4916,-2.3736,
            -18.532,6.9178,10.135,-10.661,-8.4597,-21.516,-10.418,12.621,
            -15.498,0.5225,-11.066,11.273,-12.281,10.145,-6.4771,22.995,22.21,
            -13.218,-0.768,-3.6878,1.5302,26.23,-0.83762,-2.2906,-3.5513,
            -5.8143,-17.589,-9.5392,-21.139,-12.661,18.646,9.8076,5.7163,
            -15.577,22.901,-4.1755,28.766,11.846,-0.49244,20.005,-10.785,
            6.8169};

    // Generate test source positions.
    float beamAz = 0;  // Beam azimuth.
    float beamEl = 50; // Beam elevation.
    SphericalPositions<float> pos (
            beamAz * DEG2RAD, beamEl * DEG2RAD, // Centre.
            30 * DEG2RAD, 30 * DEG2RAD, // Half-widths.
            0.1 * DEG2RAD, 0.1 * DEG2RAD); // Spacings.
    unsigned ns = pos.generate(0, 0); // No. of sources.
    std::vector<float> slon(ns), slat(ns);
    pos.generate(&slon[0], &slat[0]);

    // Call CUDA beam pattern generator.
    float freq = 1e9; // Observing frequency, Hertz.
    std::vector<float> image(ns * 2); // Beam pattern real & imaginary values.
    TIMER_START
    beamPattern2dHorizontalGeometric(na, ax, ay, ns, &slon[0],
            &slat[0], beamAz * DEG2RAD, beamEl * DEG2RAD,
            2 * M_PI * (freq / C_0), &image[0]);
    TIMER_STOP("Finished beam pattern (1000 element random array, %d points)",
            ns);

    // Write image data to file.
    FILE* file = fopen("beamPattern2dHorizontalGeometricRandom.dat", "w");
    for (unsigned s = 0; s < ns; ++s) {
        fprintf(file, "%12.3f%12.3f%16.4e%16.4e\n",
                slon[s] * RAD2DEG, slat[s] * RAD2DEG, image[2*s], image[2*s+1]);
    }
    fclose(file);
}
