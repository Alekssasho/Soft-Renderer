#ifndef TEXTURE_H
#define TEXTURE_H

#include "SDL2/SDL_image.h"
#include "color.h"

namespace SoftEngine
{
class Texture
{
private:
    int m_width = 0;
    int m_height = 0;
    SDL_Surface* m_surface = nullptr;
    void load(std::string filename);
public:
    Texture();
    Texture(std::string filename, int width, int height);
    Texture(const Texture& other) = delete;
    Texture& operator=(const Texture& other) = delete;
    ~Texture();

    Texture& operator=(Texture&& other);

    Color map(float tu, float tv) const;
};
}// end of namespace

#endif // TEXTURE_H
