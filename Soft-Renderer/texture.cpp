#include "texture.h"
#include <iostream>
#include <cmath>

namespace SoftEngine
{
Texture::Texture(std::string filename, int width, int height)
    : m_width(width), m_height(height)
{
    this->load(filename);
}

Texture::~Texture()
{
    if(m_surface)
        SDL_FreeSurface(m_surface);
}

Texture& Texture::operator=(Texture&& other)
{
    m_width = other.m_width;
    m_height = other.m_height;
    std::swap(m_surface, other.m_surface);

    return *this;
}

void Texture::load(std::string filename)
{
    m_surface = IMG_Load(("../" + filename).c_str());
    if(!m_surface) {
        std::cerr << "Cannot load image named " << filename << std::endl;
        std::cerr << "With error : " << IMG_GetError() << std::endl;
        return;
    }
}

Color Texture::map(float tu, float tv) const
{
    if(!m_surface)
        return Color::White;

    int u = std::abs(static_cast<int>(tu * m_width) % m_width);
    int v = std::abs(static_cast<int>(tv * m_height) % m_height);
    int pos = (u + v * m_width);

    Uint32 *pixels = static_cast<Uint32 *>(m_surface->pixels);
    Uint8 r, g, b, a;
    SDL_GetRGBA(pixels[pos], m_surface->format, &r, &g, &b, &a);

    return Color(r, g, b, a);
}

}//end of namespace
