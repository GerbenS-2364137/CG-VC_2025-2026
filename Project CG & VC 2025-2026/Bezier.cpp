#include "Bezier.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include <cmath>
#include <random>

namespace Bezier {

    std::vector<BezierCurve> generateClosedBezierPath(int numPoints, float radius, float hoogteFactor) {
        std::vector<glm::vec3> catmullPoints;
        std::default_random_engine generator;
        std::uniform_real_distribution<float> heightDist(-hoogteFactor, hoogteFactor);

        // Plaats punten in een cirkelvorm
        for (int i = 0; i < numPoints; ++i) {
            float angle = i * 2.0f * glm::pi<float>() / numPoints;
            float x = radius * std::cos(angle);
            float z = radius * std::sin(angle);
            float y = heightDist(generator);
            catmullPoints.emplace_back(x, y, z);
        }

        // Zorg voor gesloten Catmull-Rom: extra punten toevoegen aan begin en einde
        catmullPoints.insert(catmullPoints.begin(), catmullPoints[numPoints - 1]);
        catmullPoints.push_back(catmullPoints[1]);
        catmullPoints.push_back(catmullPoints[2]);

        std::vector<BezierCurve> curves;

        for (int i = 0; i < numPoints; ++i) {
            glm::vec3 p0 = catmullPoints[i];
            glm::vec3 p1 = catmullPoints[i + 1];
            glm::vec3 p2 = catmullPoints[i + 2];
            glm::vec3 p3 = catmullPoints[i + 3];

            glm::vec3 b0 = p1;
            glm::vec3 b1 = p1 + (p2 - p0) / 6.0f;
            glm::vec3 b2 = p2 - (p3 - p1) / 6.0f;
            glm::vec3 b3 = p2;

            curves.push_back(BezierCurve({ b0, b1, b2, b3 }));
        }

        return curves;
    }


    // De Casteljau-algoritme voor evaluatie van B�zier curve
    glm::vec3 BezierCurve::evaluate(float t) const {
        // Zorg ervoor dat t tussen 0 en 1 ligt
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;

        float u = 1.0f - t;

        glm::vec3 p = (u * u * u) * controlPoints[0];   // (1-t)^3 * p0
        p += 3 * (u * u) * t * controlPoints[1];        // 3(1-t)^2 * t * p1
        p += 3 * u * (t * t) * controlPoints[2];        // 3(1-t) * t^2 * p2
        p += (t * t * t) * controlPoints[3];            // t^3 * p3

        return p;
    }

    // Genereer een lookup table voor afstand-interpolatie
    std::vector<LookupEntry> BezierCurve::generateLookupTable(const BezierCurve& curve, int samplesPerSegment) {
        std::vector<LookupEntry> table;
        glm::vec3 previous = curve.evaluate(0.0f);
        float accumulatedDistance = 0.0f;

        table.push_back({ 0.0f, 0.0f, previous });

        for (int i = 1; i <= samplesPerSegment; ++i) {
            float t = static_cast<float>(i) / samplesPerSegment;
            glm::vec3 current = curve.evaluate(t);
            float segmentDistance = glm::length(current - previous);
            accumulatedDistance += segmentDistance;

            table.push_back({ t, accumulatedDistance, current });
            previous = current;
        }

        return table;
    }

    // Zoek het bijbehorende t voor een gegeven afstand d
    float BezierCurve::findTforDistance(const std::vector<LookupEntry>& lut, float distance) {
        if (lut.empty()) return 0.0f;
        if (distance <= 0.0f) return lut.front().t;
        if (distance >= lut.back().distance) return lut.back().t;

        for (size_t i = 1; i < lut.size(); ++i) {
            if (distance < lut[i].distance) {
                const auto& prev = lut[i - 1];
                const auto& curr = lut[i];
                float delta = curr.distance - prev.distance;
                float alpha = (distance - prev.distance) / delta;
                return glm::mix(prev.t, curr.t, alpha);
            }
        }

        return lut.back().t;
    }

    // Bereken de afgeleide/tangent van de curve op t
    // https://pomax.github.io/bezierinfo/#derivatives
    glm::vec3 Bezier::BezierCurve::tangent(float t) const {
        float u = 1.0f - t;

        glm::vec3 p =
            -3.0f * u * u * controlPoints[0] +
            (3.0f * u * u - 6.0f * u * t) * controlPoints[1] +
            (6.0f * u * t - 3.0f * t * t) * controlPoints[2] +
            3.0f * t * t * controlPoints[3];

        return glm::normalize(p);
    }

    // Deformeer mesh langs B�zier curve
    std::vector<glm::vec3> deformMeshAlongBezier(
        const Bezier::BezierCurve& curve,
        const std::vector<glm::vec3>& originalMesh,
        int numSegments
    ) {
        std::vector<glm::vec3> deformedVertices;

        for (int i = 0; i < numSegments; ++i) {
            float t = static_cast<float>(i) / (numSegments - 1);

            glm::vec3 position = curve.evaluate(t);
            glm::vec3 tangent = curve.tangent(t);

            // Vermijd degenerate up vector
            glm::vec3 up = glm::vec3(0, 1, 0);
            if (glm::abs(glm::dot(tangent, up)) > 0.99f) {
                up = glm::vec3(1, 0, 0);
            }

            glm::vec3 binormal = glm::normalize(glm::cross(up, tangent));
            glm::vec3 normal = glm::normalize(glm::cross(tangent, binormal));

            glm::mat3 rotation(binormal, normal, tangent); // TNB basis https://en.wikipedia.org/wiki/Frenet-Serret_formulas

            for (const auto& v : originalMesh) {
                glm::vec3 transformed = position + rotation * v;
                deformedVertices.push_back(transformed);
            }
        }

        return deformedVertices;
    }

}
