
#pragma once

#include <cmath>

#ifdef _WIN32
#include <intrin.h>
#endif

#define PI32 3.14159265358979323846264338327950288f
#define PI64 3.14159265358979323846264338327950288

#define _MAX(a,b) ((a) > (b) ? (a) : (b))
#define _MIN(a,b) ((a) < (b) ? (a) : (b))

#define Radians(v) (v * (PI32 / 180.0f)) 
#define Degrees(v) (v * (180.0f / PI32)) 

#define KB(x) (1024 * (x))
#define MB(x) (1024 * KB(x))
#define GB(x) (1024 * MB(x))

inline unsigned int prev_pow2(unsigned int val) {

	unsigned int pos = 0;
#ifdef _MSC_VER
	_BitScanReverse((unsigned long*)&pos, val);
#else
    for(unsigned int bit = 31; bit >= 0; bit--) {
        if(val & (1 << bit)) {
            pos = bit;
            break;
        }
    }
#endif
	return 1 << pos;
}

inline unsigned int ceil_pow2(unsigned int x) {
	unsigned int prev = prev_pow2(x);
	return x == prev ? x : prev << 1;
}

inline float lerp(float min, float max, float t) {
    return min + (max - min) * t;
}

inline float clamp(float x, float min, float max) {
    return _MIN(_MAX(x,min),max);
}

inline float frac(float x) {
    return x - (long long)x;
}

inline float smoothstep(float e0, float e1, float x) {
	float t = clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

