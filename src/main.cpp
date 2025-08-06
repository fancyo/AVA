#include <SDL3/SDL_video.h>
#define SDL_MAIN_USE_CALLBACKS 1  // use the callbacks instead of main()

#include "global.hpp"
#include "game.hpp"

static SDL_Window* window;
static SDL_Renderer* renderer;

game game;

// This function runs once at startup
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_SetAppMetadata(DESCRIPTION, VERSION, NULL); // Set game metadata (all vals defined in global.hpp)
    
    SDL_SetHint(SDL_HINT_APP_NAME, TITLE); // title
    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, FPS_HINT_VALUE); // to limit FPS to a set value 

    // Check if SDL inits properly
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL INIT. FAILURE: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    // Create window and renderer
    if (!SDL_CreateWindowAndRenderer(
            TITLE,
            (float) WIDTH*scale,
            (float) HEIGHT*scale,
            SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE, 
            &window, 
            &renderer
          )) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (window) {
        SDL_SetWindowPosition(
            window, 
            SDL_WINDOWPOS_CENTERED, 
            SDL_WINDOWPOS_CENTERED
        );

        SDL_ShowWindow(window);
    }
    
    if (!game.init(renderer, window)) {
        return SDL_APP_FAILURE;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    return SDL_APP_CONTINUE;
}

// This function runs when a new event (mouse input, keypresses, etc) occurs
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) { 
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  // End the program, reporting success to the OS
    }

    ImGui_ImplSDL3_ProcessEvent(event);
    game.poll_events(event);

    return SDL_APP_CONTINUE;
}

// This function runs once per frame, and is the heart of the program
SDL_AppResult SDL_AppIterate(void *appstate) {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    game.update(); 

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    game.render(renderer);

    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer); 

    // Present final frame after all rendering is complete
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

// This function runs once at shutdown
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    game.quit();

    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }

    SDL_Quit();
}
