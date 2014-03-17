#ifndef DEVICE_H
#define DEVICE_H

#include <vector>
#include "SDL2/SDL_stdinc.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "camera.h"
#include "mesh.h"

namespace SoftEngine
{
class Device
{
private:
    int m_width;
    int m_height;
    Uint8 *m_back_buffer;
public:
    Device(int width, int height);
    ~Device();
    Device(const Device& other) = delete;
    Device& operator=(const Device& other) = delete;

    void clear(glm::vec4 color);
    Uint8* backBuffer() const { return m_back_buffer; }
    void putPixel(glm::vec2 point, glm::vec4 color);
    glm::vec2 project(glm::vec3 coord, glm::mat4 transformationMatrix, glm::mat4 proj);
    void drawPoint(glm::vec2 point);
    void render(const SoftEngine::Camera& camera, std::vector<Mesh>& meshes);

};
}//end of namespace

#endif // DEVICE_H
