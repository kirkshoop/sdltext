#pragma once

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2_ttf/SDL_ttf.h>

#include <assert.h>

#include <cmath>
#include <chrono>
namespace t=std::chrono;

#include <rxcpp/rx.hpp>
namespace rx=rxcpp;
namespace rxu=rxcpp::util;

/**
* SDL Boilerplate
*/


/**
* Log an SDL error with some error message to the output stream of our choice
* @param os The output stream to write the message to
* @param msg The error message to write, format will be msg error: SDL_GetError()
*/
void logSDLError(std::ostream &os, const std::string &msg){
    os << msg << " error: " << SDL_GetError() << std::endl;
}

/**
* Create an SDL texture and draw the text on it. SDL error with some error message to the output stream of our choice
* @param renderer Used to create the texture.
* @param fontPath The text will be drawn with this font. This will load and parse the font.
* @param fontSize The point size to draw the text at.
* @param text The text to draw.
* @param color The color to draw the text with.
* @return texture
*/
SDL_Texture * draw_text(SDL_Renderer* renderer, const char* fontPath, int fontSize, const char* text, SDL_Color color) {

    //Open the font
    TTF_Font *font = TTF_OpenFont(fontPath, fontSize);
    if (font == nullptr){
        logSDLError(std::cout, "TTF_OpenFont");
        throw std::runtime_error("TTF_OpenFont");
    }

    //First render to a surface as that's what TTF_RenderText
    //returns
    SDL_Surface *surf = TTF_RenderText_Blended(font, text, color);
    TTF_CloseFont(font);
    if (surf == nullptr){
        logSDLError(std::cout, "TTF_RenderText");
        throw std::runtime_error("TTF_RenderText");
    }

    //Load that surface into a texture
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (texture == nullptr){
        logSDLError(std::cout, "CreateTexture");
        throw std::runtime_error("CreateTexture");
    }
    return texture;
}

/**
* filter the SDL event stream to just one event type
* @param events The source of all events
* @param type The type value of the desired events
* @return observable<CastTo*>
*/
template<class CastTo>
rx::observable<CastTo*> make_event_filter(rx::observable<SDL_Event*> events, uint16_t type) {
    return events.
        filter([=](SDL_Event* e){return e->type == type;}).
        map([](SDL_Event* e){
            return reinterpret_cast<CastTo*>(e);
        });
}

/**
* simple function to map a value in one range into the same position in another range
*/
float float_map(
  float value,
  float minValue,
  float maxValue,
  float minResult,
  float maxResult) {
    return minResult + (maxResult - minResult) * ((value - minValue) / (maxValue - minValue));
}

/**
* make SDL_Point enough of a value type to compile.
*/
SDL_Point operator+(const SDL_Point& lhs, const SDL_Point& rhs){
  return SDL_Point{lhs.x + rhs.x, lhs.y + rhs.y};
}
bool operator==(const SDL_Point& lhs, const SDL_Point& rhs){
  return lhs.x == rhs.x && lhs.y == rhs.y;
}
bool operator!=(const SDL_Point& lhs, const SDL_Point& rhs){
  return !(lhs == rhs);
}
template<class OStream>
OStream& operator<<(OStream& os, const SDL_Point& rhs){
  os << "SDLPoint: { x: " << rhs.x << ", y: " << rhs.y << "}";
  return os;
}


rx::subjects::subject<SDL_Event *> events;
auto event = events.get_subscriber();

rx::subjects::subject<t::milliseconds> updates;
auto update = updates.get_subscriber();

rx::subjects::subject<SDL_Renderer *> renders;
auto render = renders.get_subscriber();

