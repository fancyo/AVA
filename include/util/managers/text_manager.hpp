#ifndef TEXT_MANAGER
#define TEXT_MANAGER

#include <cstddef>
#include <string>
#include <unordered_map>
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <algorithm>

#include "../typedefs.hpp"

class text_manager {
public:
    // Singleton class stuff
    text_manager(const text_manager&) = delete;
    text_manager& operator = (const text_manager&) = delete;

    static text_manager& get_instance(); // Singleton function for getting an instance of the text manager class 

    bool init(); // Init SDL_TTF
    
    bool load_font(const std::string& p_path, 
                   const std::string& p_name, 
                   int p_def_ptsize = 14); // Load a font from a path, then give it a name
    ttf_font* get_font(const std::string& p_name);

    // A very complex but very usable text rendering function
    void render_text(SDL_Renderer* p_renderer,
                    const std::string& p_font_name,
                    const std::string& p_text, 
                    float ptsize,
                    const SDL_Color p_color,
                    vector_2f p_pos,
                    double p_angle,
                    const SDL_FPoint* p_center,
                    SDL_FlipMode p_flip,
                    bool p_shadow_outline); 

    // Batch rendering support
    void queue_text(const std::string& p_font_name,
                    const std::string& p_text,
                    float p_ptsize,
                    const SDL_Color p_color,
                    vector_2f p_pos,
                    float p_angle,
                    const SDL_FPoint* p_center,
                    SDL_FlipMode p_flip);

    void render_queued_text(SDL_Renderer* p_renderer); // Also batch rednering support

    // Util for getting text rect size (might move to tools.hpp)
    SDL_Rect get_text_size(const std::string& p_font_name,
                        const std::string& p_text,
                        float p_ptsize,
                        int p_max_width,
                        bool shadow);

    void quit(); // Erase all loaded fonts and quit SDL_TTF

private:
    // Private constructor / destructor for singleton
    text_manager();
    ~text_manager();

    // Caches for different parts
    std::unordered_map<std::string, ttf_font> font_cache; // To store fonts
    std::unordered_map<std::string, SDL_Texture*> text_texture_cache; // To store text textures (switching to text labeling system instead)
    std::vector<text_render_request> render_queue; // Text batching system

    // Helper functions
    std::string generate_cache_key(const std::string& p_font_name,
                                 const char* p_text,
                                 float p_ptsize,
                                 const SDL_Color p_color) const; // Creates key for bacthed text request
};

#endif // !TEXT_MANAGER
