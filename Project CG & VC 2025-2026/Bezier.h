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

        // Brute-force evaluatie via de Casteljau formule
        glm::vec3 evaluate(float t) const;

        // Afgeleide/tangent op parameter t
        glm::vec3 tangent(float t) const;

        // Forward differencing genereert (n+1) punten langs de curve
        // Efficienter dan n keer evaluate() aanroepen => enkel optelling per stap
        std::vector<glm::vec3> forwardDifferencing(int n) const;

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


