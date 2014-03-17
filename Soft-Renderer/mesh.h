#ifndef MESH_H
#define MESH_H

#include "glm/glm.hpp"
#include <string>
#include <vector>

namespace SoftEngine
{

class Mesh
{
private:
    std::string m_name;
    std::vector<glm::vec3> m_vertices;
    glm::vec3 m_position = glm::vec3(0.0f);
    glm::vec3 m_rotation = glm::vec3(0.0f);

public:
    Mesh(std::string name, int verticesCount)
    : m_name(name), m_vertices(verticesCount)
    {}

    const std::string& name() const { return m_name; }
    std::vector<glm::vec3>& vertices() { return m_vertices; }
    glm::vec3 position() const { return m_position; }
    glm::vec3 rotation() const { return m_rotation; }

    void setName(const std::string& name) { m_name = name; }
    void setPosition(const glm::vec3& position ) { m_position = position; }
    void setRotation(const glm::vec3& rotation) { m_rotation = rotation; }

};

} // end of namespace


#endif // MESH_H
