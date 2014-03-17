//
//  Mesh.h
//  Soft Renderer
//
//  Created by Aleksandar Angelov on 3/13/14.
//  Copyright (c) 2014 Aleksandar Angelov. All rights reserved.
//

#ifndef __Soft_Renderer__Mesh__
#define __Soft_Renderer__Mesh__

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
    const std::vector<glm::vec3>& vertices() const { return m_vertices; }
    glm::vec3 position() const { return m_position; }
    glm::vec3 rotation() const { return m_rotation; }
    
    void setName(std::string name) { m_name = std::move(name); };
    void setPosition(const glm::vec3& position ) { m_position = position; }
    void setRotation(const glm::vec3& rotation) { m_rotation = rotation; }
    
};
    
} // end of namespace

#endif /* defined(__Soft_Renderer__Mesh__) */
