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

//#include <omp.h>

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

void Device::proccessScanLine(ScanLineData data, Vertex& va, Vertex& vb, Vertex& vc, Vertex& vd, Color color)
{
    glm::vec3& v1 = va.coordinates;
    glm::vec3& v2 = vb.coordinates;
    glm::vec3& v3 = vc.coordinates;
    glm::vec3& v4 = vd.coordinates;

    auto gradient1 = v1.y != v2.y ? (data.currentY - v1.y) / (v2.y - v1.y) : 1;
    auto gradient2 = v3.y != v4.y ? (data.currentY - v3.y) / (v4.y - v3.y) : 1;

    int sx = static_cast<int>(lerp(v1.x, v2.x, gradient1));
    int ex = static_cast<int>(lerp(v3.x, v4.x, gradient2));

    float z1 = lerp(v1.z, v2.z, gradient1);
    float z2 = lerp(v3.z, v4.z, gradient2);

    for(int x = sx; x < ex; ++x) {
        float gradient = (x - sx) / static_cast<float>(ex - sx);
        auto ndotl = data.ndotla;
        this->drawPoint(glm::vec3(x, data.currentY, lerp(z1, z2, gradient)), color * ndotl);
    }
}

float computeNDotL(glm::vec3 vertex, glm::vec3 normal, glm::vec3 light)
{
    auto lightDirection = light - vertex;
    glm::normalize(normal);
    glm::normalize(lightDirection);
    return glm::max(0.0f, glm::dot<float>(normal, lightDirection));
}

void Device::drawTriangle(Vertex vv1, Vertex vv2, Vertex vv3, Color color)
{
    if(vv1.coordinates.y > vv2.coordinates.y) std::swap(vv1, vv2);
    if(vv2.coordinates.y > vv3.coordinates.y) std::swap(vv2, vv3);
    if(vv1.coordinates.y > vv2.coordinates.y) std::swap(vv1, vv2);

    glm::vec3& v1 = vv1.coordinates;
    glm::vec3& v2 = vv2.coordinates;
    glm::vec3& v3 = vv3.coordinates;

    glm::vec3 vnFace = (vv1.normal + vv2.normal + vv3.normal) / 3.0f;
    glm::vec3 centerPoint = (vv1.worldCoordinates + vv2.worldCoordinates + vv3.worldCoordinates) / 3.0f;

    glm::vec3 lightPos(0, 10, -10);

    float ndotl = computeNDotL(centerPoint, vnFace, lightPos);
    ScanLineData data;
    data.ndotla = ndotl;

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
            data.currentY = y;
            if(y < v2.y)
                this->proccessScanLine(data, vv1, vv3, vv1, vv2, color);
            else
                this->proccessScanLine(data, vv1, vv3, vv2, vv3, color);
        }
    } else {
        for(int y = v1.y; y <= v3.y; ++y) {
            data.currentY = y;
            if(y < v2.y)
                this->proccessScanLine(data, vv1, vv2, vv1, vv3, color);
            else
                this->proccessScanLine(data, vv2, vv3, vv1, vv3, color);
        }
    }
}

Vertex Device::project(Vertex& vertex, glm::mat4& MVP, glm::mat4& modelMatrix)
{
    auto point = MVP * glm::vec4(vertex.coordinates, 1.0f);
    point /= point.w;

    auto worldPoint = modelMatrix * glm::vec4(vertex.coordinates, 1.0f);
    worldPoint /= worldPoint.w;
    auto worldNormal = modelMatrix * glm::vec4(vertex.normal, 0.0f);

    float x = point.x * m_width + m_width / 2.0f;
    float y = -point.y * m_height + m_height / 2.0f;
    Vertex result;
    result.coordinates = glm::vec3(x, y, point.z);
    result.normal = glm::vec3(worldNormal);
    result.worldCoordinates = glm::vec3(worldPoint);
    return result;
}

//#define PARALLEL

void Device::render(const Camera &camera, std::vector<Mesh> &meshes)
{
    auto viewMatrix = lookAtLH(camera.position(), camera.target(), glm::vec3(0.0f, 1.0f, 0.0f));
    auto projectionMatrix = perspectiveFovLH(0.78f, static_cast<float>(m_width) / m_height, 0.01f, 1.0f);

#ifdef PARALLEL
    struct vector {
        int index;
        std::vector<Vertex> m_backingVector;

        vector(int size)
            :index(0), m_backingVector(size)
        {}

        void push_back(Vertex& point)
        {
            if(m_backingVector.size() == index)
                m_backingVector.resize(index * 2);
            m_backingVector[index++] = point;
        }
    };

    vector quadrant1(500);
    vector quadrant2(500);
    vector quadrant3(500);
    vector quadrant4(500);
    vector quadrantCommon(500);

    int halfWidth = m_width / 2;
    int halfHeight = m_height / 2;
#endif

    for(Mesh& mesh : meshes) {
        auto modelMatrix = glm::translate(glm::mat4(1.0f), mesh.position()) *
                glm::yawPitchRoll(mesh.rotation().y, mesh.rotation().x, mesh.rotation().z);

        auto MVP = projectionMatrix * viewMatrix * modelMatrix;
#ifndef PARALLEL
        int faceIndex = 0;
#else
        int result;
#endif
        for(Face& face : mesh.faces()) {
            auto pointA = this->project(mesh.vertices()[face.A], MVP, modelMatrix);
            auto pointB = this->project(mesh.vertices()[face.B], MVP, modelMatrix);
            auto pointC = this->project(mesh.vertices()[face.C], MVP, modelMatrix);
#ifdef PARALLEL
            result = 0;
            result += pointA.coordinates.x >= halfWidth ? 1 : 0;
            result += pointB.coordinates.x >= halfWidth ? 1 : 0;
            result += pointC.coordinates.x >= halfWidth ? 1 : 0;
            result += pointA.coordinates.y >= halfHeight ? 4 : 0;
            result += pointB.coordinates.y >= halfHeight ? 4 : 0;
            result += pointC.coordinates.y >= halfHeight ? 4 : 0;

            vector *p_quadrant = &quadrantCommon;
            switch(result) {
            case 0:  p_quadrant = &quadrant2; break;
            case 3:  p_quadrant = &quadrant1; break;
            case 12: p_quadrant = &quadrant3; break;
            case 15: p_quadrant = &quadrant4; break;
            }

            p_quadrant->push_back(pointA);
            p_quadrant->push_back(pointB);
            p_quadrant->push_back(pointC);
#else
            auto color = (0.25f + (faceIndex % mesh.faces().size()) * 0.75f / mesh.faces().size()) * 255;
            this->drawTriangle(pointA, pointB, pointC, Color(color, color, color, 255));
            ++faceIndex;
#endif

        }
    }
#ifdef PARALLEL
//std::cerr << quadrant1.size() << "  " << quadrant2.size() << " " << quadrant3.size() << " " << quadrant4.size() << " " << quadrantCommon.size() << std::endl;

auto drawTask = [](Device* dev, vector &arr, Color colour)
{
    int faceIndex = 0;
    for(auto i = 0; i < arr.index; i += 3) {
        auto color = (0.25f + (faceIndex % (arr.m_backingVector.size() / 3)) * 0.75f / (arr.m_backingVector.size() / 3)) * 255;
        dev->drawTriangle(arr.m_backingVector[i], arr.m_backingVector[i + 1], arr.m_backingVector[i + 2], Color(color, color, color, 255));
        ++faceIndex;
    }
};

#pragma omp parallel sections
    {
     #pragma omp section
        {
//            std::cout << omp_get_num_thread();
            drawTask(this, quadrant1, Color::Red);
        }
    #pragma omp section
       {
        drawTask(this, quadrant2, Color::Green);
       }
    #pragma omp section
       {
        drawTask(this, quadrant3, Color::Blue);
       }
    #pragma omp section
       {
        drawTask(this, quadrant4, Color::White);
       }
    }

    drawTask(this, quadrantCommon, Color(125, 125, 125, 255));
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
            //Normals
            float nx = verticesArray[i * verticesStep + 3].ToFloat();
            float ny = verticesArray[i * verticesStep + 4].ToFloat();
            float nz = verticesArray[i * verticesStep + 5].ToFloat();
            currentMesh.vertices()[i].coordinates = glm::vec3(x, y, z);
            currentMesh.vertices()[i].normal = glm::vec3(nx, ny, nz);
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
