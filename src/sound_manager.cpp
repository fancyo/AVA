#include "util/managers/sound_manager.hpp"
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <cstdint>
#include <sys/types.h>

// SOUND MANAGER

sound_manager& sound_manager::get_instance() {
    static sound_manager mgr;
    return mgr;
}

sound_manager::sound_manager() { }

sound_manager::~sound_manager() {
    quit(); 
}

bool sound_manager::init() {
    if (audio_device != 0) {
        return true;
    }

    audio_device =
        SDL_OpenAudioDevice(
            SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, 
            NULL
        );

    if (audio_device == 0) {
        return false;
    }

    return true;
}

bool sound_manager::load_wav(const std::string& p_path, const std::string& p_name) {
    if (audio_device == 0) {
        SDL_Log("Audio device not initialized");
        return false;
    }

    // Check if already loaded
    auto it = audio_cache.find(p_name);
    if (it != audio_cache.end()) {
        return true;
    }

    static wav_audio audio = {nullptr, 0, nullptr, false};

    audio_cache[p_name] = audio;

    // Dynamically allocate the full path using SDL_asSDL_Log
    char *wav_path = nullptr;
    if (SDL_asprintf(&wav_path, "%s%s", SDL_GetBasePath(), p_path.c_str()) < 0) {
        SDL_Log("Failed to allocate memory for WAV path: %s", SDL_GetError());
        audio_cache.erase(p_name);
        return false;
    }

    // Load the WAV file
    SDL_AudioSpec spec;
    if (!SDL_LoadWAV(
        wav_path, 
        &spec, 
        &audio_cache[p_name].data, 
        &audio_cache[p_name].data_len)
    ) {
        SDL_Log("Failed to load WAV file: %s", SDL_GetError());
        audio_cache.erase(p_name);
        return false;
    }

    audio_cache[p_name].stream = SDL_CreateAudioStream(&spec, nullptr);
    if (!audio_cache[p_name].stream) {
        SDL_Log("Failed to create audio stream: %s", SDL_GetError());
        SDL_free(audio_cache[p_name].data);
        audio_cache.erase(p_name);
        return false;
    }

    if (!SDL_BindAudioStream(audio_device, audio_cache[p_name].stream)) {
        SDL_Log("Failed to bind audio stream: %s", SDL_GetError());
        SDL_DestroyAudioStream(audio_cache[p_name].stream);
        SDL_free(audio_cache[p_name].data);
        audio_cache.erase(p_name);
        return false;
    }

    SDL_free(wav_path);

    audio_cache[p_name].loaded = true;
    
    return true;
}

wav_audio* sound_manager::get_audio(const std::string& p_name) {
    auto it = audio_cache.find(p_name);
    return (it != audio_cache.end()) ? &it->second : nullptr;
}

void sound_manager::play(const std::string& p_name) {
    wav_audio *audio = (audio_device != 0) ? get_audio(p_name) : nullptr;

    // valid_audio = true if audio exists and is loaded
    u_int8_t valid_audio = ((bool)audio + (u_int8_t)audio->loaded) >> 1;

    // stream_available is only queried if audio is valid
    u_int8_t available = (valid_audio + (u_int8_t)SDL_GetAudioStreamAvailable(audio->stream)) >> 1;

    // only attempt to push data if all conditions are good
    (valid_audio && (available < static_cast<int>(audio->data_len))) ?
        SDL_PutAudioStreamData(audio->stream, 
                               audio->data, static_cast<int>(audio->data_len)) : 0;
}

void sound_manager::quit() {
    printf("RUNNING sound_manager::quit()\n");
    printf("CHECKING AUDIO DEVICE...\n");
    if (audio_device == 0) {
        printf("AUDIO DEVICE IS ALREADY CLOSED\nEXITING sound_manager::quit()\n");
        return;
    }
    printf("AUDIO DEVICE IS INITIALIZED...\n");

    printf("CLEANING DATA...\n");
    for (auto& [name, audio] : audio_cache) { 
        printf("FREEING DATA...\n");
        if (audio.data) {
            SDL_free(audio.data);
            audio.data = nullptr;
            printf("FREED DATA OF OBJ: %s\n", name.c_str());
        }
        printf("DATA FREED...\n");

        printf("DESTROYING STREAMS...\n");
        if (audio.stream) { 
            audio.stream = nullptr;
            SDL_DestroyAudioStream(audio.stream);
            printf("DESTROYED STREAM OF OBJ: %s\n", name.c_str());
        }
        printf("STREAM(S) DESTROYED...\n");
    }

    printf("CLEARING AUDIO CACHE...\n");
    audio_cache.clear();
    printf("CLEARED AUDIO CACHE...\n");
    printf("CLOSING AUDIO DEVICE..\n");
    SDL_CloseAudioDevice(audio_device);
    audio_device = 0;
    printf("AUDIO DEVICE CLOSED...\n");

    printf("RAN sound_manager::quit() SUCCESSFULLY.\n");
}

bool audio_capture::init() {
    SDL_AudioDeviceID *devices;
    SDL_AudioSpec outspec;
    SDL_AudioSpec inspec;
    SDL_AudioSpec desired_spec = {
        .format = SDL_AUDIO_S16,
        .channels = 1,
        .freq = 44100,
    };
    SDL_AudioDeviceID device;
    SDL_AudioDeviceID want_device;
    const char *devname = NULL;
    int i;

    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "15");

    SDL_Log("Using audio driver: %s", SDL_GetCurrentAudioDriver());

    devices = SDL_GetAudioRecordingDevices(NULL);
    if (!devices || !devices[0]) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No recording devices found!");
        return SDL_APP_FAILURE;
    }

    SDL_Log("Available recording devices:");
    for (i = 0; devices[i] != 0; i++) {
        SDL_Log(" [%d] %s", i, SDL_GetAudioDeviceName(devices[i]));
    }

    int choice = -1;
    while (choice < 0 || choice >= i) {
        printf("Select a recording device (0-%d): ", i - 1);
        fflush(stdout);
        if (scanf("%d", &choice) != 1 || choice < 0 || choice >= i) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid selection.");
            choice = -1;
            while (getchar() != '\n'); // clear buffer
        }
    }

    want_device = devices[choice];
    devname = SDL_GetAudioDeviceName(want_device);
    SDL_Log("You selected recording device: '%s'", devname);

    SDL_Log("Opening default playback device...");
    device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired_spec);
    if (!device) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, 
            "Couldn't open an audio device for playback: %s!", 
            SDL_GetError()
        );
        SDL_free(devices);
        return SDL_APP_FAILURE;
    }
    SDL_PauseAudioDevice(device);
    SDL_GetAudioDeviceFormat(device, &outspec, NULL);
    audio_spec = outspec;
    stream_o = SDL_CreateAudioStream(&outspec, &outspec);
    if (!stream_o || !SDL_BindAudioStream(device, stream_o)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't bind playback stream: %s", SDL_GetError());
        SDL_free(devices);
        return SDL_APP_FAILURE;
    }

    SDL_Log("Opening recording device '%s'...", devname);
    device = SDL_OpenAudioDevice(want_device, &desired_spec);
    if (!device) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION, 
            "Couldn't open audio device for recording: %s!", 
            SDL_GetError()
        );
        SDL_free(devices);
        return SDL_APP_FAILURE;
    }
    SDL_free(devices);
    SDL_PauseAudioDevice(device);
    SDL_GetAudioDeviceFormat(device, &inspec, NULL);
    stream_i = SDL_CreateAudioStream(&inspec, &outspec);
    if (!stream_i || !SDL_BindAudioStream(device, stream_i)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't bind input stream: %s!", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Create WAV file
    wav_file = fopen("output.wav", "wb");
    if (!wav_file) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open output.wav for writing");
        return SDL_APP_FAILURE;
    }
    Uint8 header[44] = {0};
    fwrite(header, 1, sizeof(header), wav_file);

    SDL_Log("Ready! Hold mouse button to record, release to play back and save to output.wav.");

    return true;
}

SDL_AppResult audio_capture::poll_event(SDL_Event* p_event) {
    if (p_event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    } else if (p_event->type == SDL_EVENT_KEY_DOWN) {
        if (p_event->key.key == SDLK_ESCAPE) {
            return SDL_APP_SUCCESS;
        }
    } else if (p_event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (p_event->button.button == 1) {
            record();
        }
    } else if (p_event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (p_event->button.button == 1) {
            play();
            system("./tools/whisper-cli -m tools/ggml-base.en.bin -f output.wav");
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult audio_capture::main_action() {
    while (SDL_GetAudioStreamAvailable(stream_i) > 0) {
        Uint8 buf[4096];
        const int br = SDL_GetAudioStreamData(stream_i, buf, sizeof(buf));
        if (br < 0) {
            SDL_LogError(
                SDL_LOG_CATEGORY_APPLICATION, 
                "Failed to read from input audio stream: %s", 
                SDL_GetError()
            );
            return SDL_APP_FAILURE;
        } else if (!SDL_PutAudioStreamData(stream_o, buf, br)) {
            SDL_LogError(
                SDL_LOG_CATEGORY_APPLICATION, 
                "Failed to write to output audio stream: %s", 
                SDL_GetError()
            );
            return SDL_APP_FAILURE;
        }

        if (wav_file && br > 0) {
            fwrite(buf, 1, br, wav_file);
            wav_data += br;
        }
    }

    return SDL_APP_CONTINUE;
}

void audio_capture::record() {
    SDL_PauseAudioStreamDevice(stream_o);
    SDL_FlushAudioStream(stream_o);
    SDL_ResumeAudioStreamDevice(stream_i);

    wav_data = 0;  // reset recorded data size

    // Reopen file for new recording session
    if (!wav_file) {
        wav_file = fopen("output.wav", "wb");
        if (!wav_file) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open output.wav for writing");
            return;
        }
        Uint8 header[44] = {0};  // Placeholder for WAV header
        fwrite(header, 1, sizeof(header), wav_file);
    }
}

void audio_capture::play() {
    SDL_PauseAudioStreamDevice(stream_i);
    SDL_FlushAudioStream(stream_i);

    if (wav_file) {
        write_wav(wav_file, &audio_spec, wav_data);
        fclose(wav_file);
        wav_file = nullptr;  // prevent use-after-close
        SDL_Log("WAV file written to output.wav (%u bytes)", wav_data);
    }
}

void audio_capture::write_wav(
    FILE* p_file, 
    const SDL_AudioSpec* p_spec, 
    u_int32_t p_data) {
    Uint16 audio_format = 1; // PCM
    Uint16 num_channels = p_spec->channels;
    Uint32 sample_rate = p_spec->freq;
    Uint16 bits_per_sample = SDL_AUDIO_BITSIZE(p_spec->format);
    Uint32 byte_rate = sample_rate * num_channels * bits_per_sample / 8;
    Uint16 block_align = num_channels * bits_per_sample / 8;
    Uint32 chunk_size = 36 + p_data;

    fseek(p_file, 0, SEEK_SET);
    fwrite("RIFF", 1, 4, p_file);
    fwrite(&chunk_size, 4, 1, p_file);
    fwrite("WAVE", 1, 4, p_file);

    fwrite("fmt ", 1, 4, p_file);
    Uint32 subchunk1_size = 16;
    fwrite(&subchunk1_size, 4, 1, p_file);
    fwrite(&audio_format, 2, 1, p_file);
    fwrite(&num_channels, 2, 1, p_file);
    fwrite(&sample_rate, 4, 1, p_file);
    fwrite(&byte_rate, 4, 1, p_file);
    fwrite(&block_align, 2, 1, p_file);
    fwrite(&bits_per_sample, 2, 1, p_file);

    fwrite("data", 1, 4, p_file);
    fwrite(&p_data, 4, 1, p_file);
}
