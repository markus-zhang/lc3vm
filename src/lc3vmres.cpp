#include "lc3vmres.hpp"
#include <iostream>


LC3VMRes::LC3VMRes(SDL_Renderer* renderer)
{
    if (renderer == nullptr)
    {
        std::cerr << "Function " << __FUNCTION__ << " error: renderer passed as null pointer" << std::endl;
    }
    renderer = renderer;
}

LC3VMRes::~LC3VMRes()
{
    for (auto iter = res.begin(); iter != res.end(); iter++)
    {
        TTF_CloseFont(iter->second);
    }
}

void LC3VMRes::load_font(std::string path, int size, std::string id)
{
    if (path.length() == 0)
    {
        std::cerr << "Function " << __FUNCTION__ << " error: path length 0" << std::endl;
        exit(1);
    }

    if (size <= 0)
    {
        std::cerr << "Function " << __FUNCTION__ << " error: font size <= 0" << std::endl;
        exit(1);
    }

    if (id.length() <= 0)
    {
        std::cerr << "Function " << __FUNCTION__ << " error: font id empty" << std::endl;
        exit(1);
    }

    TTF_Font* font = TTF_OpenFont(path.c_str(), size);

    if (font == nullptr)
    {
        std::cerr << "Function " << __FUNCTION__ << " error: failed to load from path " << path << std::endl;
        exit(1);
    }

    res.insert({id, font});
}

TTF_Font* LC3VMRes::get_font(std::string id)
{
    auto search = res.find(id);
    if (search != res.end())
    {
        return search->second;
    }
    return nullptr;
}