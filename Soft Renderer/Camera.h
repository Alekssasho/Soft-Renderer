//
//  Camera.h
//  Soft Renderer
//
//  Created by Aleksandar Angelov on 3/13/14.
//  Copyright (c) 2014 Aleksandar Angelov. All rights reserved.
//

#ifndef __Soft_Renderer__Camera__
#define __Soft_Renderer__Camera__

#include "glm/glm.hpp"

namespace SoftEngine
{
class Camera
{
private:
    glm::vec3 m_position = glm::vec3(0.0f);
    glm::vec3 m_target   = glm::vec3(0.0f);
public:
    
    glm::vec3 position() const { return m_position; }
    glm::vec3 target() const   { return m_target; }
    
    void setPosition(const glm::vec3& position) { m_position = position; }
    void setTarget(const glm::vec3& target) { m_target = target; }
};
} // end of namespace

#endif /* defined(__Soft_Renderer__Camera__) */
