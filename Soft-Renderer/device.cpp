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
#include <climits>

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

static float lerp(float min, float max, float gradient)
{
    return min + (max - min) * glm::clamp<float>(gradient, 0.0f, 1.0f);
}

namespace SoftEngine
{

Device::Device(int width, int height)
    : m_width(width), m_height(height), m_back_buffer(new Color[width * height]), m_depthBuffer(new float[width * height])
{}

Device::~Device()
{
    delete [] m_back_buffer;
    delete [] m_depthBuffer;
}

void Device::clear(const Color color)
{
    for(int i = 0; i < m_width * m_height; ++i) {
        m_back_buffer[i] = color;
        m_depthBuffer[i] = std::numeric_limits<float>::max();
    }
}

void Device::putPixel(int x, int y, float z, const Color color)
{
    int index = (x + y * m_width);


    if(m_depthBuffer[index] < z)
        return;
    m_depthBuffer[index] = z;
    m_back_buffer[index] = color;
}

void Device::drawPoint(glm::vec3 point, Color color)
{
    if(point.x >= 0 && point.y >= 0 && point.x < m_width && point.y < m_height) {
        this->putPixel(static_cast<int>(point.x), static_cast<int>(point.y), point.z, color);
    }
}

void Device::drawLine(glm::vec3 start, glm::vec3 end, Color color)
{
    auto dist = glm::length(end - start);

    if(dist < 2 )
        return;
    auto middle = start + (end - start) / 2.0f;
    this->drawPoint(middle, color);

    this->drawLine(start, middle, color);
    this->drawLine(middle, end, color);
}

void Device::drawBLine(glm::vec3 start, glm::vec3 end, Color color)
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
        this->drawPoint(glm::vec3(x0, y0, start.z), color);
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

//void Device::fillTopTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, Color color)
//{
//    float invslope1 = v3.y == v1.y ? (v3.x - v1.x) / (v3.y - v1.y) : 1;
//    float invslope2 = v3.y == v2.y ? (v3.x - v2.x) / (v3.y - v2.y) : 1;

//    float curx1 = v3.x;
//    float curx2 = v3.x;

//    for(int y = v3.y; y > v1.y; --y) {
//        curx1 -= invslope1;
//        curx2 -= invslope2;
//        this->drawBLine(glm::vec3(curx1, y, v1.z), glm::vec3(curx2, y, v1.z), color);
//    }
//}

//void Device::fillBottomTriangle(glm::ivec2 v1, glm::ivec2 v2, glm::ivec2 v3, Color color)
//{
//    float invslope1 = v2.y != v1.y ? (v2.x - v1.x) / (v2.y - v1.y) : 1;
//    float invslope2 = v3.y != v1.y ? (v3.x - v1.x) / (v3.y - v1.y) : 1;

//    float curx1 = v1.x;
//    float curx2 = v1.x;

//    for(int y = v1.y; y <= v2.y; ++y) {
//        this->drawBLine(glm::ivec2(curx1, y), glm::ivec2(curx2, y), color);
//        curx1 += invslope1;
//        curx2 += invslope2;
//    }
//}

void Device::proccessScanLine(int y, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4, Color color)
{
    auto gradient1 = v1.y != v2.y ? (y - v1.y) / (v2.y - v1.y) : 1;
    auto gradient2 = v3.y != v4.y ? (y - v3.y) / (v4.y - v3.y) : 1;

    int sx = static_cast<int>(lerp(v1.x, v2.x, gradient1));
    int ex = static_cast<int>(lerp(v3.x, v4.x, gradient2));

    float z1 = lerp(v1.z, v2.z, gradient1);
    float z2 = lerp(v3.z, v4.z, gradient2);

    for(int x = sx; x < ex; ++x) {
        float gradient = (x - sx) / static_cast<float>(ex - sx);
        this->drawPoint(glm::vec3(x, y, lerp(z1, z2, gradient)), color);
    }
}

void Device::drawTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, Color color)
{
    if(v1.y > v2.y) std::swap(v1, v2);
    if(v2.y > v3.y) std::swap(v2, v3);
    if(v1.y > v2.y) std::swap(v1, v2);

//    if(v2.y == v3.y)
//        this->fillBottomTriangle(v1, v2, v3, color);
//    else if(v1.y == v2.y)
//        this->fillTopTriangle(v1, v2, v3, color);
//    else {
//        int x = v3.x != v1.x ? (v1.x + (v2.y - v1.y)) / ((v3.y - v1.y) * (v3.x - v1.x)) : v3.x;

//        glm::ivec2 v4( x, v2.y);
//        this->fillBottomTriangle(v1, v2, v4, color);
//        this->fillTopTriangle(v2, v4, v3, color);
//    }

    float dV1V2;
    float dV1V3;
    if(v2.y - v1.y > 0)
        dV1V2 = (v2.x - v1.x) / (v2.y - v1.y);
    else
        dV1V2 = 0;
    if(v3.y - v1.y > 0)
        dV1V3 = (v3.x - v1.x) / (v3.y - v1.y);
    else
        dV1V3 = 0;

    if(dV1V2 > dV1V3) {
        for(int y = v1.y; y <= v3.y; ++y) {
            if(y < v2.y)
                this->proccessScanLine(y, v1, v3, v1, v2, color);
            else
                this->proccessScanLine(y, v1, v3, v2, v3, color);
        }
    } else {
        for(int y = v1.y; y <= v3.y; ++y) {
            if(y < v2.y)
                this->proccessScanLine(y, v1, v2, v1, v3, color);
            else
                this->proccessScanLine(y, v2, v3, v1, v3, color);
        }
    }
}

glm::vec3 Device::project(glm::vec3 coord, glm::mat4 MVP)
{
    auto point = MVP * glm::vec4(coord, 1.0f);
    point /= point.w;

    float x = point.x * m_width + m_width / 2.0f;
    float y = -point.y * m_height + m_height / 2.0f;
    return glm::vec3(x, y, point.z);
}

#define PARALLEL

void Device::render(const Camera &camera, std::vector<Mesh> &meshes)
{
    auto viewMatrix = lookAtLH(camera.position(), camera.target(), glm::vec3(0.0f, 1.0f, 0.0f));
    auto projectionMatrix = perspectiveFovLH(0.78f, static_cast<float>(m_width) / m_height, 0.01f, 1.0f);

#ifdef PARALLEL
    std::vector<glm::vec3> quadrant1;
    std::vector<glm::vec3> quadrant2;
    std::vector<glm::vec3> quadrant3;
    std::vector<glm::vec3> quadrant4;
    std::vector<glm::vec3> quadrantCommon;
#endif

    for(Mesh& mesh : meshes) {
        auto modelMatrix = glm::translate(glm::mat4(1.0f), mesh.position()) *
                glm::yawPitchRoll(mesh.rotation().y, mesh.rotation().x, mesh.rotation().z);

        auto MVP = projectionMatrix * viewMatrix * modelMatrix;
#ifndef PARALLEL
        int faceIndex = 0;
#endif

        for(Face& face : mesh.faces()) {
            auto pointA = this->project(mesh.vertices()[face.A], MVP);
            auto pointB = this->project(mesh.vertices()[face.B], MVP);
            auto pointC = this->project(mesh.vertices()[face.C], MVP);
#ifdef PARALLEL
            if(pointA.x >= 0 && pointA.x < m_width / 2 && pointA.y >= 0 && pointA.y < m_height / 2) {
                if(pointB.x >= 0 && pointB.x < m_width / 2 && pointB.y >= 0 && pointB.y < m_height / 2) {
                    if(pointC.x >= 0 && pointC.x < m_width / 2 && pointC.y >= 0 && pointC.y < m_height / 2) {
                        quadrant2.push_back(pointA);
                        quadrant2.push_back(pointB);
                        quadrant2.push_back(pointC);
                        continue;
                    }
                }
                quadrantCommon.push_back(pointA);
                quadrantCommon.push_back(pointB);
                quadrantCommon.push_back(pointC);
            } else if(pointA.x >= m_width / 2 && pointA.x < m_width && pointA.y >= 0 && pointA.y < m_height / 2) {
                if(pointB.x >= m_width / 2 && pointB.x < m_width && pointB.y >= 0 && pointB.y < m_height / 2) {
                    if(pointC.x >= m_width / 2 && pointC.x < m_width && pointC.y >= 0 && pointC.y < m_height / 2) {
                        quadrant1.push_back(pointA);
                        quadrant1.push_back(pointB);
                        quadrant1.push_back(pointC);
                        continue;
                    }
                }
                quadrantCommon.push_back(pointA);
                quadrantCommon.push_back(pointB);
                quadrantCommon.push_back(pointC);
            } else if(pointA.x >= m_width / 2 && pointA.x < m_width && pointA.y >= m_height / 2 && pointA.y < m_height) {
                if(pointB.x >= m_width / 2 && pointB.x < m_width && pointB.y >= m_height / 2 && pointB.y < m_height) {
                    if(pointC.x >= m_width / 2 && pointC.x < m_width && pointC.y >= m_height / 2 && pointC.y < m_height) {
                        quadrant4.push_back(pointA);
                        quadrant4.push_back(pointB);
                        quadrant4.push_back(pointC);
                        continue;
                    }
                }
                quadrantCommon.push_back(pointA);
                quadrantCommon.push_back(pointB);
                quadrantCommon.push_back(pointC);
            } else {
                quadrant3.push_back(pointA);
                quadrant3.push_back(pointB);
                quadrant3.push_back(pointC);
            }

#else
            auto color = (0.25f + (faceIndex % mesh.faces().size()) * 0.75f / mesh.faces().size()) * 255;
            this->drawTriangle(pointA, pointB, pointC, Color(color, color, color, 255));
            ++faceIndex;
#endif

        }
    }
#ifdef PARALLEL
//std::cerr << quadrant1.size() << "  " << quadrant2.size() << " " << quadrant3.size() << " " << quadrant4.size() << " " << quadrantCommon.size() << std::endl;
#pragma omp parallel sections
    {
     #pragma omp section
        {
            int faceIndex = 0;
            for(auto itr = std::begin(quadrant1); itr != std::end(quadrant1); itr += 3) {
                auto color = (0.25f + (faceIndex % (quadrant1.size() / 3)) * 0.75f / (quadrant1.size() / 3)) * 255;
                this->drawTriangle(*itr, *(itr + 1), *(itr + 2), Color(color, color, color, 255));
                ++faceIndex;
            }
        }
    #pragma omp section
       {
           int faceIndex = 0;
           for(auto itr = std::begin(quadrant2); itr != std::end(quadrant2); itr += 3) {
               auto color = (0.25f + (faceIndex % (quadrant2.size() / 3)) * 0.75f / (quadrant2.size() / 3)) * 255;
               this->drawTriangle(*itr, *(itr + 1), *(itr + 2), Color(color, color, color, 255));
               ++faceIndex;
           }
       }
    #pragma omp section
       {
           int faceIndex = 0;
           for(auto itr = std::begin(quadrant3); itr != std::end(quadrant3); itr += 3) {
               auto color = (0.25f + (faceIndex % (quadrant3.size() / 3)) * 0.75f / (quadrant3.size() / 3)) * 255;
               this->drawTriangle(*itr, *(itr + 1), *(itr + 2), Color(color, color, color, 255));
               ++faceIndex;
           }
       }
    #pragma omp section
       {
           int faceIndex = 0;
           for(auto itr = std::begin(quadrant4); itr != std::end(quadrant4); itr += 3) {
               auto color = (0.25f + (faceIndex % (quadrant4.size() / 3)) * 0.75f / (quadrant4.size() / 3)) * 255;
               this->drawTriangle(*itr, *(itr + 1), *(itr + 2), Color(color, color, color, 255));
               ++faceIndex;
           }
       }
    }

    int faceIndex = 0;
    for(auto itr = std::begin(quadrantCommon); itr != std::end(quadrantCommon); itr += 3) {
        auto color = (0.25f + (faceIndex % (quadrantCommon.size() / 3)) * 0.75f / (quadrantCommon.size() / 3)) * 255;
        this->drawTriangle(*itr, *(itr + 1), *(itr + 2), Color(color, color, color, 255));
        ++faceIndex;
    }

#endif
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
