#include "device.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/ext.hpp"
#include "json/json.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <string>
#include <iostream>

static glm::mat4 lookAtLH(glm::vec3 eye, glm::vec3 target, glm::vec3 up)
{
    glm::vec3 xaxis, yaxis, zaxis;
    zaxis = glm::normalize(target - eye);
    xaxis = glm::normalize(glm::cross(up, zaxis));
    yaxis = glm::cross(zaxis, xaxis);

    glm::mat4 result(1.0f);
    result[0][0] = xaxis.x;
    result[1][0] = xaxis.y;
    result[2][0] = xaxis.z;
    result[0][1] = yaxis.x;
    result[1][1] = yaxis.y;
    result[2][1] = yaxis.z;
    result[0][2] = zaxis.x;
    result[1][2] = zaxis.y;
    result[2][2] = zaxis.z;

    result[3][0] = -glm::dot(xaxis, eye);
    result[3][1] = -glm::dot(yaxis, eye);
    result[3][2] = -glm::dot(zaxis, eye);

    return result;
}

static glm::mat4 perspectiveFovLH(float fovy, float aspect, float znear, float zfar)
{
    float h = glm::cot(fovy * 0.5f);
    float w = h / aspect;

    glm::mat4 result(0.0f);
    result[0][0] = w;
    result[1][1] = h;
    result[2][2] = zfar / (zfar - znear);
    result[2][3] = 1.0f;
    result[3][2] = -znear * zfar / (zfar - znear);

    return result;
}

namespace SoftEngine
{

Device::Device(int width, int height)
    : m_width(width), m_height(height), m_back_buffer(new Color[width * height])
{}

Device::~Device()
{
    delete [] m_back_buffer;
}

void Device::clear(const Color color)
{
    for(int i = 0; i < m_width * m_height; ++i) {
        m_back_buffer[i] = color;
    }
}

void Device::putPixel(glm::ivec2 point, const Color color)
{
    int index = (point.x + point.y * m_width);
    m_back_buffer[index] = color;
}

void Device::drawPoint(glm::ivec2 point)
{
    if(point.x >= 0 && point.y >= 0 && point.x < m_width && point.y < m_height) {
        this->putPixel(point, Color(255, 255, 0, 255));
    }
}

void Device::drawLine(glm::ivec2 start, glm::ivec2 end)
{
    auto dist = glm::length(glm::vec2(end) - glm::vec2(start));

    if(dist < 2 )
        return;
    auto middle = start + (end - start) / 2;
    this->drawPoint(middle);

    this->drawLine(start, middle);
    this->drawLine(middle, end);
}

void Device::drawBLine(glm::ivec2 start, glm::ivec2 end)
{
    int x0 = start.x;
    int y0 = start.y;
    int x1 = end.x;
    int y1 = end.y;

    auto dx = abs(x1 - x0);
    auto dy = abs(y1 - y0);
    auto sx = (x0 < x1) ? 1 : -1;
    auto sy = (y0 < y1) ? 1 : -1;
    auto err = dx - dy;
    while(true) {
        this->drawPoint(glm::ivec2(x0, y0));
        if((x0 == x1) && (y0 == y1))
            break;
        auto e2 = 2 * err;
        if(e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if(e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

glm::ivec2 Device::project(glm::vec3 coord, glm::mat4 MVP)
{
    auto point = MVP * glm::vec4(coord, 1.0f);
    point /= point.w;

    float x = point.x * m_width + m_width / 2.0f;
    float y = -point.y * m_height + m_height / 2.0f;
    return glm::ivec2(x, y);
}

void Device::render(const Camera &camera, std::vector<Mesh> &meshes)
{
    auto viewMatrix = lookAtLH(camera.position(), camera.target(), glm::vec3(0.0f, 1.0f, 0.0f));
    auto projectionMatrix = perspectiveFovLH(0.78f, static_cast<float>(m_width) / m_height, 0.01f, 1.0f);

    for(Mesh& mesh : meshes) {
        auto modelMatrix = glm::translate(glm::mat4(1.0f), mesh.position()) *
                glm::yawPitchRoll(mesh.rotation().y, mesh.rotation().x, mesh.rotation().z);

        auto MVP = projectionMatrix * viewMatrix * modelMatrix;

        for(Face& face : mesh.faces()) {
            auto pointA = this->project(mesh.vertices()[face.A], MVP);
            auto pointB = this->project(mesh.vertices()[face.B], MVP);
            auto pointC = this->project(mesh.vertices()[face.C], MVP);
            this->drawBLine(pointA, pointB);
            this->drawBLine(pointB, pointC);
            this->drawBLine(pointC, pointA);
        }
    }
}

void Device::loadJSONFile(std::string filename, std::vector<Mesh> &meshesVector)
{
    std::ifstream file(filename);
    std::string content;
    content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    json::Value data = json::Deserialize(content);
    json::Object obj = data.ToObject();
    json::Array meshes = obj["meshes"];
    for(auto& mesh : meshes) {
        auto verticesArray = mesh["vertices"];
        auto indicesArray = mesh["indices"];
        auto uvCount = mesh["uvCount"].ToInt();
        int verticesStep = 1;

        switch(uvCount) {
        case 0: verticesStep = 6; break;
        case 1: verticesStep = 8; break;
        case 2: verticesStep = 10; break;
        }
        int verticesCount = verticesArray.size() / verticesStep;
        int facesCount = indicesArray.size() / 3;
        meshesVector.push_back(Mesh(mesh["name"].ToString(), verticesCount, facesCount));
        Mesh& currentMesh = meshesVector.back();
        for(int i = 0; i < verticesCount; ++i) {
            float x = verticesArray[i * verticesStep].ToFloat();
            float y = verticesArray[i * verticesStep + 1].ToFloat();
            float z = verticesArray[i * verticesStep + 2].ToFloat();
            currentMesh.vertices()[i] = glm::vec3(x, y, z);
        }

        for(int i = 0; i < facesCount; ++i) {
            int a = indicesArray[i * 3].ToInt();
            int b = indicesArray[i * 3 + 1].ToInt();
            int c = indicesArray[i * 3 + 2].ToInt();
            currentMesh.faces()[i] = {a, b, c};
        }

        auto position = mesh["position"].ToArray();
        currentMesh.setPosition(glm::vec3(position[0].ToFloat(), position[1].ToFloat(), position[2].ToFloat()));
    }
}

}//end of namespace
