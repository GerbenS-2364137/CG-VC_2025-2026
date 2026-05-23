#include "FollowCam.h"

void FollowCamera::SetTarget(const glm::vec3& target) {
    m_TargetPosition = target;
}

void FollowCamera::SetDirection(const glm::vec3& direction) {
    m_Direction = glm::normalize(direction);
}

void FollowCamera::Update() {
    // Kleine offset in lokale ruimte van het voertuig voor cockpit view
    glm::vec3 localOffset = m_Direction * 0.5f + WorldUp * 0.2f;

    // Zet de camera net boven en achter het midden van het object
    Position = m_TargetPosition + localOffset;

    // Kijk vooruit in de rijrichting
    Front = glm::normalize(m_Direction);

    // Herbereken TBN-frame
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}
