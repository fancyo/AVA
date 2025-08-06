#ifndef GLOBAL
#define GLOBAL

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_sdlrenderer3.h"

static constexpr unsigned int WIDTH = 980;
static constexpr unsigned int HEIGHT = 540;

const uint8_t BG_R = 0x18;
const uint8_t BG_G = 0x18;
const uint8_t BG_B = 0x18;
const uint8_t BG_A = SDL_ALPHA_OPAQUE;

#define TITLE "Versatile Clinic Control Program (VCCP)"
#define DESCRIPTION "Program used for general clinic control and management."
#define VERSION "0.1.0"
#define FPS_HINT_VALUE "60"

#endif // !GLOBAL
