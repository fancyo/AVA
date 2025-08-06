#include "util/managers/text_manager.hpp"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>
#include <SDL3_ttf/SDL_ttf.h>

text_manager::text_manager() { }

text_manager::~text_manager() {
    quit();
}

text_manager& text_manager::get_instance() {
    static text_manager mgr;
    return mgr;
} 

bool text_manager::init() { 
    if (!TTF_Init()) {
        SDL_Log("ERROR: TTF SUBSYSTEM FAILED TO INITIALIZE %s", SDL_GetError());
        return false;
    }

    return true;
}

bool text_manager::load_font(
    const std::string& p_path, 
    const std::string& p_name, 
    int p_def_ptsize) { 
    if (font_cache.find(p_name) != font_cache.end()) {
        SDL_Log("Font %s already loaded", p_name.c_str());
        return false;
    }

    ttf_font font = {p_path.c_str(), NULL, p_def_ptsize};
    font.font = TTF_OpenFont(font.path, p_def_ptsize);
    if (!font.font) {
        SDL_Log("COULDN'T LOAD FONT: %s", SDL_GetError());
        return false;
    }

    font_cache[p_name] = font;
    return true;
}

ttf_font* text_manager::get_font(const std::string& p_name) {
    auto it = font_cache.find(p_name);
    return (it != font_cache.end()) ? &it->second : nullptr;
}

std::string text_manager::generate_cache_key(const std::string& p_font_name,
                                            const char* p_text,
                                            float p_ptsize,
                                            const SDL_Color p_color) const {
    return p_font_name + "|" + p_text + "|" + 
        std::to_string(static_cast<int>(p_ptsize)) + "|" +
        std::to_string(p_color.r) + "," +
        std::to_string(p_color.g) + "," +
        std::to_string(p_color.b) + "," +
        std::to_string(p_color.a);
}

void text_manager::render_text(SDL_Renderer* p_renderer,
                               const std::string& p_font_name,
                               const std::string& p_text,
                               float ptsize,
                               const SDL_Color p_color,
                               vector_2f p_pos,
                               double p_angle,
                               const SDL_FPoint* p_center,
                               SDL_FlipMode p_flip,
                               bool p_shadow_outline) {
    ttf_font* font = get_font(p_font_name);
    if (!font) return;

    TTF_SetFontSize(font->font, static_cast<int>(ptsize));

    // Render passes: shadow (if any), then normal
    for (int pass = 0; pass < (p_shadow_outline ? 2 : 1); pass++) {
        // Calculate masks
        int is_shadow_pass = p_shadow_outline && pass == 0;

        // Use masks to conditionally adjust color and position
        Uint8 r = static_cast<Uint8>(p_color.r - ((p_color.r * is_shadow_pass) >> 2 * is_shadow_pass));
        Uint8 g = static_cast<Uint8>(p_color.g - ((p_color.g * is_shadow_pass) >> 2 * is_shadow_pass));
        Uint8 b = static_cast<Uint8>(p_color.b - ((p_color.b * is_shadow_pass) >> 2 * is_shadow_pass));

        SDL_Color color = {
            static_cast<Uint8>((p_color.r >> 2) * is_shadow_pass + p_color.r * (!is_shadow_pass)),
            static_cast<Uint8>((p_color.g >> 2) * is_shadow_pass + p_color.g * (!is_shadow_pass)),
            static_cast<Uint8>((p_color.b >> 2) * is_shadow_pass + p_color.b * (!is_shadow_pass)),
            p_color.a
        };

        vector_2f pos = {
            p_pos.x + 4.0f * is_shadow_pass,
            p_pos.y + 4.0f * is_shadow_pass
        };

        // Create font cache key
        const std::string cache_key = 
            generate_cache_key(
                p_font_name, p_text.c_str(), 
                ptsize, 
                color
            );
        
        // Check if text label is in cache
        auto cached = text_texture_cache.find(cache_key);

        // Create texture 
        SDL_Texture* texture = nullptr;
        if (cached != text_texture_cache.end()) {
            texture = cached->second;
        } else {
            SDL_Surface* surface = 
                TTF_RenderText_Solid(
                    font->font, 
                    p_text.c_str(), 
                    p_text.length(), 
                    color
                );
            if (!surface) {
                SDL_Log("Failed to render text: %s", SDL_GetError());
                continue;
            }

            texture = SDL_CreateTextureFromSurface(p_renderer, surface);
            SDL_DestroySurface(surface);
            if (!texture) {
                SDL_Log("Failed to create texture: %s", SDL_GetError());
                continue;
            }

            text_texture_cache[cache_key] = texture;
        }

        SDL_FRect dst{pos.x, pos.y, 0, 0};
        SDL_GetTextureSize(texture, &dst.w, &dst.h);

        SDL_RenderTextureRotated(p_renderer,
                                 texture,
                                 nullptr,
                                 &dst,
                                 p_angle,
                                 p_center,
                                 p_flip);
    }
}

void text_manager::queue_text(const std::string& p_font_name,
                            const std::string& p_text,
                            float p_ptsize,
                            const SDL_Color p_color,
                            vector_2f p_pos,
                            float p_angle,
                            const SDL_FPoint* p_center,
                            SDL_FlipMode p_flip) {
    render_queue.push_back({
        p_font_name, 
        p_text, 
        p_ptsize, 
        p_color, 
        p_pos, 
        static_cast<float>(p_angle), 
        p_center, 
        p_flip
    });
}

void text_manager::render_queued_text(SDL_Renderer* p_renderer) {
    // Sort by font/size/color to minimize state changes
    std::sort(render_queue.begin(), render_queue.end(),
        [](const text_render_request& a, const text_render_request& b) {
            return std::tie(a.font_name, a.ptsize, a.color.r, a.color.g, a.color.b, a.color.a) <
                   std::tie(b.font_name, b.ptsize, b.color.r, b.color.g, b.color.b, b.color.a);
        });

    // Render in batches
    for (const auto& request : render_queue) {
        render_text(p_renderer, 
                    request.font_name, 
                    request.text.c_str(),
                    request.ptsize, 
                    request.color, 
                    request.pos, 
                    request.angle, 
                    request.center, 
                    request.flip,
                    false);
    }
    render_queue.clear();
}

SDL_Rect text_manager::get_text_size(const std::string& p_font_name,
                                    const std::string& p_text,
                                    float p_ptsize,
                                    int p_max_width,
                                    bool shadow) {
    ttf_font* font = get_font(p_font_name);
    if (!font) return {0, 0, 0, 0};

    int w;
    float h = p_ptsize;

    size_t p_length = p_text.length();

    TTF_SetFontSize(font->font, static_cast<int>(p_ptsize));
    TTF_MeasureString(font->font, p_text.c_str(), p_length, p_max_width, &w, NULL);
    
    if (shadow) {
        return {0, 0, w+4, static_cast<int>(h)+4};
    }

    return {0, 0, w, static_cast<int>(h)};
}

void text_manager::quit() {
    // Clear texture cache first
    for (auto& [key, texture] : text_texture_cache) {
        SDL_DestroyTexture(texture);
    }
    text_texture_cache.clear();
    
    // Then clear fonts
    for (auto& [name, font] : font_cache) {
        if (font.font) {
            TTF_CloseFont(font.font);
            font.font = nullptr;
        }
    }
    font_cache.clear();
    
    TTF_Quit();
}
