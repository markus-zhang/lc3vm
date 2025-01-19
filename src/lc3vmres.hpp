#ifndef _LC3VMRES_HPP_
#define _LC3VMRES_HPP_

/*
    The resource class that stores font
*/

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include <unordered_map>
#include <string>

class LC3VMRes {
public:
    /* Member variables */
    std::unordered_map<std::string, TTF_Font*> res;
    std::unordered_map<char, SDL_Texture*> charset;
    SDL_Renderer* renderer;

    /* Member functions */
    LC3VMRes() : renderer(nullptr) {}
    LC3VMRes(SDL_Renderer* renderer);
    ~LC3VMRes();

    void load_font(std::string path, int size, std::string id);
    TTF_Font* get_font(std::string id);
    /* Generate SDL_Texture* for 0-9, a-z, A-Z, /, ?, ., ^, +, | */
    void generate_charset();
};

#endif