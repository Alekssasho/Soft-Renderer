#ifndef MESH_H
#define MESH_H

#include "glm/glm.hpp"
#include <string>
#include <vector>
#include "texture.h"

namespace SoftEngine
{

struct Face
{
    int A;
    int B;
    int C;

    glm::vec3 normal;
};

struct Vertex
{
    glm::vec3 normal;
    glm::vec3 coordinates;
    glm::vec3 worldCoordinates;
    glm::vec2 textureCoordinates;
};

class Mesh
{
private:
    std::string m_name;
    std::vector<Vertex> m_vertices;
    std::vector<Face> m_faces;
    glm::vec3 m_position = glm::vec3(0.0f);
    glm::vec3 m_rotation = glm::vec3(0.0f);
    Texture *m_texture = nullptr;

public:
    Mesh(std::string name, int verticesCount, int facesCount)
    : m_name(name), m_vertices(verticesCount), m_faces(facesCount)
    {}

    ~Mesh()
    {
        delete m_texture;
    }

    const std::string& name() const { return m_name; }
    std::vector<Vertex>& vertices() { return m_vertices; }
    std::vector<Face>& faces() { return m_faces; }
    glm::vec3 position() const { return m_position; }
    glm::vec3 rotation() const { return m_rotation; }
    const Texture& texture() const { return *m_texture; }

    void setName(const std::string& name) { m_name = name; }
    void setPosition(const glm::vec3& position ) { m_position = position; }
    void setRotation(const glm::vec3& rotation) { m_rotation = rotation; }
    void setTexture(Texture* texture) { delete m_texture; m_texture = texture; }

    void computeFaceNormal() {
        for(Face& face : m_faces) {
            auto &vertexA  = m_vertices[face.A];
            auto &vertexB  = m_vertices[face.B];
            auto &vertexC  = m_vertices[face.C];

            face.normal = glm::normalize((vertexA.normal + vertexB.normal + vertexC.normal) / 3.0f);
        }
    }

};

} // end of namespace


#endif // MESH_H
