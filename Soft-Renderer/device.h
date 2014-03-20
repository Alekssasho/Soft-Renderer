#ifndef DEVICE_H
#define DEVICE_H

#include <vector>
#include "SDL2/SDL_stdinc.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "camera.h"
#include "mesh.h"
#include "color.h"

namespace SoftEngine
{
class Device
{
private:
    int m_width;
    int m_height;
    Color *m_back_buffer;
public:
    Device(int width, int height);
    ~Device();
    Device(const Device& other) = delete;
    Device& operator=(const Device& other) = delete;

    void loadJSONFile(std::string filename, std::vector<Mesh>& meshes);

    void clear(const Color color);

    Color* backBuffer() const { return m_back_buffer; }

    void putPixel(glm::ivec2 point, const Color color);

    glm::ivec2 project(glm::vec3 coord, glm::mat4 MVP);

    void drawPoint(glm::ivec2 point);
    void drawLine(glm::ivec2 start, glm::ivec2 end);
    void drawBLine(glm::ivec2 start, glm::ivec2 end);

    void render(const SoftEngine::Camera& camera, std::vector<Mesh>& meshes);
};
}//end of namespace

#endif // DEVICE_H
