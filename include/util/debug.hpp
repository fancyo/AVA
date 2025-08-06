#ifndef DEBUG
#define DEBUG

#include <cstdint>

inline float get_fps(uint64_t previous_time, uint64_t current_time) {
    uint64_t delta_time = current_time - previous_time;
    if (delta_time == 0) return 0.0f; // Avoid division by zero
    return 1000.0f / delta_time; // Milliseconds to seconds
}

#endif // !DEBUG
