#ifndef COLOR_H
#define COLOR_H

#include "SDL2/SDL_stdinc.h"
#include <iostream>
#include "glm/glm.hpp"

namespace SoftEngine
{
class Color
{
private:
    Uint32 m_color;
public:
    Color()
        : m_color(0)
    {}

    Color(Uint32 red, Uint32 green, Uint32 blue, Uint32 alpha)
    {

        m_color = (glm::clamp<Uint32>(red, 0, 255)  << 24)
                | (glm::clamp<Uint32>(green, 0, 255)  << 16)
                | (glm::clamp<Uint32>(blue, 0, 255)  << 8)
                | glm::clamp<Uint32>(alpha, 0, 255);
    }

    unsigned int r() const
    {
        return (m_color & 0xff000000) >> 24;
    }

    unsigned int g() const
    {
        return (m_color & 0x00ff0000) >> 16;
    }

    unsigned int b() const
    {
        return (m_color & 0x0000ff00) >> 8;
    }

    unsigned int a() const
    {
        return (m_color & 0x000000ff);
    }

    void setRed(Uint32 red)
    {

        m_color &= 0x00ffffff;
        m_color |= (glm::clamp<Uint32>(red, 0, 255) << 24);
    }

    void setGreen(Uint32 green)
    {
        m_color &= 0xff00ffff;
        m_color |= (glm::clamp<Uint32>(green, 0, 255)  << 16);
    }

    void setBlue(Uint32 blue)
    {
        m_color &= 0xffff00ff;
        m_color |= (glm::clamp<Uint32>(blue, 0, 255) << 8);
    }

    void setAlpha(Uint32 alpha)
    {
        m_color &= 0xffffff00;
        m_color |= glm::clamp<Uint32>(alpha, 0, 255);
    }

    Uint32 color() const
    {
        return m_color;
    }

    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color White;
    static const Color Black;
};

Color operator*(Color& color, float scalar);

std::ostream& operator<<(std::ostream& out, const Color& color);

}//end of namespace



#endif // COLOR_H
