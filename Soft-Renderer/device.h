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

namespace // annonymous namespace
{
struct ScanLineData
{
    int currentY;
    float ndotla;
    float ndotlb;
    float ndotlc;
    float ndotld;
};
}

class Device
{
private:
    int m_width;
    int m_height;
    Color *m_back_buffer;
    float *m_depthBuffer;

    void putPixel(int x, int y, float z, const Color color);
    Vertex project(Vertex& coord, glm::mat4& MVP, glm::mat4& modelMatrix);
    void proccessScanLine(ScanLineData y, Vertex& v1, Vertex& v2, Vertex& v3,Vertex& v4, Color color);
public:
    Device(int width, int height);
    ~Device();
    Device(const Device& other) = delete;
    Device& operator=(const Device& other) = delete;

    void loadJSONFile(std::string filename, std::vector<Mesh>& meshes);

    void clear(const Color color);

    Color* backBuffer() const { return m_back_buffer; }

    void render(const SoftEngine::Camera& camera, std::vector<Mesh>& meshes);

    void drawPoint(glm::vec3 point, Color color);
    void drawLine(glm::vec3 start, glm::vec3 end, Color color);
    void drawBLine(glm::vec3 start, glm::vec3 end, Color color);
    void drawTriangle(Vertex v1, Vertex v2, Vertex v3, Color color);
};
}//end of namespace

#endif // DEVICE_H
