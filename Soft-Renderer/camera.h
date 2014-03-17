#ifndef CAMERA_H
#define CAMERA_H

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

#endif // CAMERA_H
