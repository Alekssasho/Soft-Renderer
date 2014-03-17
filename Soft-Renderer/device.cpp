#include "device.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/ext.hpp"
#include <algorithm>

#include <iostream>
#include <stdio.h>

SoftEngine::Device::Device(int width, int height)
    : m_width(width), m_height(height), m_back_buffer(new Uint8[width * height * 4])
{}

SoftEngine::Device::~Device()
{
    delete [] m_back_buffer;
}

void SoftEngine::Device::clear(glm::vec4 color)
{
    for(int i = 0; i < m_width * m_height * 4; i += 4) {
        m_back_buffer[i] = color.r;
        m_back_buffer[i + 1] = color.g;
        m_back_buffer[i + 2] = color.b;
        m_back_buffer[i + 3] = color.a;
    }
}

void SoftEngine::Device::putPixel(glm::vec2 point, glm::vec4 color)
{
    int index = (point.x + point.y * m_width) * 4;
    m_back_buffer[index] = color.r * 255;
    m_back_buffer[index + 1] = color.g * 255;
    m_back_buffer[index + 2] = color.b * 255;
    m_back_buffer[index + 3] = color.a * 255;
}

glm::vec2 SoftEngine::Device::project(glm::vec3 coord, glm::mat4 transformationMatrix, glm::mat4 proj)
{

//     std::cout << glm::to_string(glm::vec4(coord, 1.0f)) << std::endl;
//    auto point = glm::vec4(coord, 1.0f) * transformationMatrix;

//    float x = point.x * m_width + m_width / 2.0f;
//    float y = -point.y * m_height + m_height / 2.0f;
//    std::cout << x << "    " << y << std::endl;
//    return glm::vec2(x, y);

    auto point = glm::project(coord, transformationMatrix, proj, glm::vec4(0, 0, m_width, m_height));
//    std::cout << glm::to_string(point) << std::endl;
    return glm::vec2(point);
}

void SoftEngine::Device::drawPoint(glm::vec2 point)
{
    if(point.x >= 0 && point.y >= 0 && point.x < m_width && point.y < m_height) {
        this->putPixel(point, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    }
}

void SoftEngine::Device::render(const SoftEngine::Camera &camera, std::vector<Mesh> &meshes)
{
    auto viewMatrix = glm::lookAt(camera.position(), camera.target(), glm::vec3(0.0f, 1.0f, 0.0f));
    auto projectionMatrix = glm::perspective(0.78f, static_cast<float>(m_width) / m_height, 0.01f, 1.0f);

    for(Mesh& mesh : meshes) {
//        auto worldMatrix = glm::translate(glm::mat4(1.0f), mesh.position()) *
//                glm::yawPitchRoll(mesh.rotation().y, mesh.rotation().x, mesh.rotation().z);
        auto worldMatrix = glm::translate(glm::yawPitchRoll(mesh.rotation().y, mesh.rotation().x, mesh.rotation().z), mesh.position());

        auto WVP = projectionMatrix * viewMatrix * worldMatrix;
//        auto WVP = worldMatrix * viewMatrix * projectionMatrix;

        for(glm::vec3& vertex : mesh.vertices()) {
            auto point = this->project(vertex, viewMatrix * worldMatrix, projectionMatrix);
            this->drawPoint(point);
        }
    }
}
