#ifndef STRUCTS
#define STRUCTS 

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_surface.h>
#include <cstddef>
#include <cstdint>

#include <SDL3/SDL_audio.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <sys/types.h>

/*-------------------------------*/
  /*STRUCTS - STRUCTS - STRUCTS*/
/*-------------------------------*/

typedef struct vector_2f {
    float x;
    float y;
} vector_2f;

typedef struct vector_2i {
    int x;
    int y;
} vector_2i;

// For Sound Manager
typedef struct wav_audio {
    uint8_t *data;
    uint32_t data_len;

    SDL_AudioStream *stream;

    bool loaded;
} wav_audio;

// For Text Manager
typedef struct ttf_font { 
    const char* path;
    TTF_Font *font;
    int ptsize;
} ttf_font;

typedef struct text_render_request {
    std::string font_name;
    std::string text;
    float ptsize;
    SDL_Color color;
    vector_2f pos;
    float angle;
    const SDL_FPoint* center;
    SDL_FlipMode flip;
} text_render_request;

typedef struct text_label {
    bool show;
    std::string text;
    float ptsize;
    SDL_Color color;
    vector_2i pos;
    float angle;
    const SDL_Point* center;
    SDL_FlipMode flip;
} text_label;

/*-------------------------*/
  /*ENUMS - ENUMS - ENUMS*/
/*-------------------------*/

typedef enum game_state {
    STATE_SPLASH,
    STATE_MENU,
    STATE_GAME,
    STATE_PAUSE
} game_state;

/* --------------------- */
/*  CLINIC PROGRAM DATA  */

typedef enum gender {
    MALE = 1,
    FEMALE = 0
} gender;

typedef struct patient {
    char name[256];
    char date_of_birth[64];
    char email[128];
    char address[1024];
    const char* gender;
    u_int8_t age;
    u_int64_t id;
    u_int64_t number;
} patient;

#endif // !UTIL
