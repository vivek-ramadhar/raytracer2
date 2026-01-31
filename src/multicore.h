//
// Created by vivek on 1/10/2026.
//

#ifdef WIN32
#pragma once
#include <pmmintrin.h>
#include <windows.h>
#include <xmmintrin.h>
#ifndef RAYTRACER2_MULTICORE_H
#define RAYTRACER2_MULTICORE_H

static inline void PinThisThreadToCore(int core, bool highprio) {
    if (highprio) {
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    }
    DWORD_PTR mask = 1ull << core;
    SetThreadAffinityMask(GetCurrentThread(), mask);
}
#endif

static inline void EnableFTZ_DAZ() {
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
}


#endif //RAYTRACER2_MULTICORE_H