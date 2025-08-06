#ifndef GAME
#define GAME

#include "global.hpp"

#include "util/debug.hpp"
#include "util/typedefs.hpp"
#include "util/tools.hpp"

#include "imgui/imgui.h"

#include "util/managers/sound_manager.hpp"
#include "util/managers/text_manager.hpp"

#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <string>
#include <sys/types.h>
#include <vector>

class game {
public:
    game();
    ~game();

    // Initializer function (!= constructor)
    bool init(SDL_Renderer* p_renderer, SDL_Window* p_window);

    // Read events
    void poll_events(SDL_Event* p_event);
    
    // Core functions
    void update();
    void render(SDL_Renderer* p_renderer);

    void reset(); // DEBUGGING

    bool audio = false;
    bool show_text = false;
    audio_capture capture_system;

    // Deinitializer function
    void quit();

    std::string text;
    std::string response;
    std::string clean_resp;

private:
    int splash_timer;
    game_state current_state;

    std::vector<patient> record;

    bool window = true;
    ImVec4 my_color;
    char buffer[256];
    u_int8_t age;
    const char* choices[2] = {"MALE", "FEMALE"};

    bool debug;
    void show_debug();
};

#endif // !GAME
