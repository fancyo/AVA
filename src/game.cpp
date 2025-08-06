#include "game.hpp"
#include "imgui/imgui.h"
#include "util/tools.hpp"
#include "util/typedefs.hpp"
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_pixels.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <type_traits>

game::game() {
    splash_timer = 0;
    current_state = STATE_GAME;
    debug = false;
}

game::~game() { }

bool game::init(SDL_Renderer* p_renderer, SDL_Window* p_window) {
    if (!text_manager::get_instance().init() || !sound_manager::get_instance().init()) {
        return false;
    }

    if (!capture_system.init()) {
        return false;
    }

    return true;
}

void game::poll_events(SDL_Event* p_event) { 
    if (p_event->type == SDL_EVENT_KEY_DOWN) {
        switch (current_state) {
            case STATE_SPLASH: {

            } break;

            case STATE_MENU: {

            } break;

            case STATE_GAME: {
                if (p_event->key.key == SDLK_W && !window) {
                    window = true;                
                }
            } break;

            case STATE_PAUSE: {

            } break;
        }
    }
}

void game::update() {
    switch (current_state) {
        case STATE_SPLASH: {

        } break;

        case STATE_MENU: {
            
        }; break;

        case STATE_GAME: {
            capture_system.main_action();
        }; break;

        default: break;
    }
}

void game::render(SDL_Renderer* p_renderer) {
    // Clear the screen based on current state
    switch (current_state) {
        case STATE_SPLASH: {
                    
        } break;

        case STATE_MENU: {
            
        } break;

        case STATE_GAME: {
            if (window) {
                ImGui::Begin("SPEECH TO TEXT");
                
                if (ImGui::Button("START", {200, 100})) {
                    audio = true;
                    printf("TEST ON \n");
                    show_text = false;
                    // response.clear();
                    // clean_resp.clear();
                    capture_system.record();
                } 
                if (audio) {
                    if (ImGui::Button("STOP", {200, 100})) {
                        audio = false;
                        printf("TEST OFF \n");
                        capture_system.play();
                        text = run_command("./tools/whisper-cli -m tools/ggml-base.en.bin -f output.wav --no-prints --no-timestamps");
                        printf("transcribed %s\n", text.c_str());
                        response = query_gemini(text);
                        clean_resp = extract_text(response);
                        text.clear();

                        show_text = true;
                        printf("%s", clean_resp.c_str());
                    }
                }
                if (show_text) {
                    ImGui::Text("%s", clean_resp.c_str());
                }

                ImGui::End();
            }
        } break;

        default: break;
    } 
}

void game::reset() { 
    current_state = STATE_GAME;
    splash_timer = 0;
}

void game::quit() {

}

void game::show_debug() { }
