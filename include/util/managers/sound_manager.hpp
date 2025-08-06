#ifndef SOUND_MANAGER
#define SOUND_MANAGER

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_oldnames.h>
#include <cstdio>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <atomic>
#include <cstdint>
#include <vector>
#include <mutex>

#include "../typedefs.hpp"

class sound_manager {
public: 
    sound_manager(const sound_manager&) = delete;

    static sound_manager& get_instance();

    bool init();
    bool load_wav(const std::string& p_path, const std::string& p_name);
    wav_audio* get_audio(const std::string& p_name);
    void play(const std::string& p_name);
    void quit();
    
private:
    sound_manager();
    ~sound_manager(); 

    SDL_AudioDeviceID audio_device = 0;
    
    std::unordered_map<std::string, wav_audio> audio_cache;
};

// SDL Audio capture

class audio_capture {
public:
    bool init();

    SDL_AppResult poll_event(SDL_Event* p_event);

    // callback to be called by SDL
    SDL_AppResult main_action();

    void record();
    void play();
    
    void quit();

    SDL_AudioStream* stream_o;
    SDL_AudioStream* stream_i;

    FILE* wav_file;
    u_int32_t wav_data;
    SDL_AudioSpec audio_spec;

private:
    void write_wav(FILE* p_file, const SDL_AudioSpec* p_spec, u_int32_t p_data);
};

#endif // !SOUND_MANAGER
