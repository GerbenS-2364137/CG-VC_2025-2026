#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace Bezier {

    // Struct om lengte over de curve bij te houden
    struct LookupEntry {
        float t;           // de genormaliseerde parameter op de curve (0-1)
        float distance;    // afstand vanaf start tot punt op t
        glm::vec3 point;   // het corresponderende punt op de curve
    };

    class BezierCurve {
    public:
        std::vector<glm::vec3> controlPoints;

        BezierCurve(std::initializer_list<glm::vec3> points)
            : controlPoints(points) {
        }

        glm::vec3 evaluate(float t) const;
        glm::vec3 tangent(float t) const;

        static std::vector<LookupEntry> generateLookupTable(const BezierCurve& curve, int samplesPerSegment = 100);
        static float findTforDistance(const std::vector<LookupEntry>& lut, float distance);
    };

    std::vector<BezierCurve> generateClosedBezierPath(int numCurves, float radius, float hoogteFactor);

    std::vector<glm::vec3> deformMeshAlongBezier(
        const BezierCurve& curve,
        const std::vector<glm::vec3>& originalMesh,
        int numSegments
    );

}


