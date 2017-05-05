#pragma once

#include "ofMain.h"
#include "ofxFaceTracker.h"
#include "ofxTiming.h"

class FaceOverlay {
private:
    // Hack until ofPolyline supports 2d vectors better.
    static glm::vec3 toVec3(const glm::vec2& v)
    {
        return glm::vec3(v.x, v.y, 0);
    }

    template <typename T>
    static void drawCurve2D(const T& polyline) {
        if(polyline.size() == 0) {
            return;
        }
        ofBeginShape();
        const auto& vertices = polyline.getVertices();
        ofCurveVertex(vertices.front().x, vertices.front().y);
        for(std::size_t i = 0; i < vertices.size(); i++) {
            ofCurveVertex(vertices[i].x, vertices[i].y);
        }
        ofCurveVertex(vertices.back().x, vertices.back().y);
        ofEndShape();
    }

    static ofPolyline buildCircle(const glm::vec2& center, float radius, int resolution = 32) {
        ofPolyline polyline;
        glm::vec2 base(radius, 0);
        for(int i = 0; i < resolution; i++) {
            float angle = 360 * (float) i / resolution;
            glm::vec2 cur = center + glm::rotate(base, ofDegToRad(angle));
            polyline.addVertex({ cur.x, cur.y, 0 });
        }
        polyline.close();
        return polyline;
    }

    static float determinant(const glm::vec2& start,
                             const glm::vec2& end,
                             const glm::vec2& point) {
        return (end.x - start.x) * (point.y - start.y) - (end.y - start.y) * (point.x - start.x);
    }

    static void divide(const glm::vec3& a, const glm::vec3& b,
                       const glm::vec3& left, const glm::vec3& right,
                       ofMesh& top, ofMesh& bottom) {
        auto avg = glm::mix(a, b, 0.5);
        if(determinant(left.xy(), right.xy(), avg.xy()) < 0) {
            top.addVertex(a);
            top.addVertex(b);
        } else {
            bottom.addVertex(a);
            bottom.addVertex(b);
        }
    }

    static void divide(const ofPolyline& poly,
                       const glm::vec3& left, const glm::vec3& right,
                       ofMesh& top, ofMesh& bottom) {
        top.setMode(OF_PRIMITIVE_LINES);
        bottom.setMode(OF_PRIMITIVE_LINES);
        const auto& vertices = poly.getVertices();
        for(std::size_t i = 0; i + 1 < vertices.size(); i++) {
            divide(vertices[i], vertices[i + 1], left, right, top, bottom);
        }
        if(poly.isClosed()) {
            divide(vertices.back(), vertices.front(), left, right, top, bottom);
        }
    }

public:
    Hysteresis noseHysteresis, mouthHysteresis;
    float noseAngle = 15;
    float nostrilWidth = .06;
    float lipWidth = .7;

    FaceOverlay() {
        noseHysteresis.setDelay(1000);
        mouthHysteresis.setDelay(1000);
    }
    void draw(ofxFaceTracker& tracker) {
        if(noseHysteresis.set(abs(tracker.getOrientation().y) > ofDegToRad(noseAngle))) {
            tracker.getImageFeature(ofxFaceTracker::NOSE_BRIDGE).draw();
        }

        ofPolyline eyeLeft = tracker.getImageFeature(ofxFaceTracker::LEFT_EYE);
        ofPolyline eyeRight = tracker.getImageFeature(ofxFaceTracker::RIGHT_EYE);
        glm::vec2 eyeLeftLeft = tracker.getImagePoint(36);
        glm::vec2 eyeLeftRight = tracker.getImagePoint(39);
        glm::vec2 eyeRightLeft = tracker.getImagePoint(42);
        glm::vec2 eyeRightRight = tracker.getImagePoint(45);
        float eyeRadius = tracker.getScale() * 3;

        float lidHeight = .5;
        glm::vec2 eyeLeftLidLeft = glm::mix(tracker.getImagePoint(36), tracker.getImagePoint(37), lidHeight);
        glm::vec2 eyeLeftLidRight = glm::mix(tracker.getImagePoint(38), tracker.getImagePoint(39), lidHeight);
        glm::vec2 eyeRightLidLeft = glm::mix(tracker.getImagePoint(42), tracker.getImagePoint(43), lidHeight);
        glm::vec2 eyeRightLidRight = glm::mix(tracker.getImagePoint(44), tracker.getImagePoint(45), lidHeight);

        glm::vec2 eyeCenterLeft = glm::mix(eyeLeftLidLeft, eyeLeftLidRight, .5);
        glm::vec2 eyeCenterRight = glm::mix(eyeRightLidLeft, eyeRightLidRight, .5);

        float irisSize = .5;
        ofMesh leftTop, leftBottom;
        ofPolyline leftCircle = buildCircle(eyeCenterLeft, eyeRadius * irisSize);
        divide(leftCircle, toVec3(eyeLeftLidLeft), toVec3(eyeLeftLidRight), leftTop, leftBottom);
        ofMesh rightTop, rightBottom;
        ofPolyline rightCircle = buildCircle(eyeCenterRight, eyeRadius * irisSize);
        divide(rightCircle, toVec3(eyeLeftLidLeft), toVec3(eyeLeftLidRight), rightTop, rightBottom);

        leftBottom.draw();
        rightBottom.draw();

        ofPolyline lowerLip;
        lowerLip.addVertex(toVec3(glm::mix(tracker.getImagePoint(59), tracker.getImagePoint(48), lipWidth)));
        lowerLip.addVertex(toVec3(tracker.getImagePoint(59)));
        lowerLip.addVertex(toVec3(tracker.getImagePoint(58)));
        lowerLip.addVertex(toVec3(tracker.getImagePoint(57)));
        lowerLip.addVertex(toVec3(tracker.getImagePoint(56)));
        lowerLip.addVertex(toVec3(tracker.getImagePoint(55)));
        lowerLip.addVertex(toVec3(glm::mix(tracker.getImagePoint(55), tracker.getImagePoint(54), lipWidth)));
        drawCurve2D(lowerLip);

        ofPolyline innerLip;
        innerLip.addVertex(toVec3(glm::mix(tracker.getImagePoint(60), tracker.getImagePoint(48), lipWidth)));
        innerLip.addVertex(toVec3(tracker.getImagePoint(60)));
        innerLip.addVertex(toVec3(tracker.getImagePoint(61)));
        innerLip.addVertex(toVec3(tracker.getImagePoint(62)));
        innerLip.addVertex(toVec3(glm::mix(tracker.getImagePoint(62), tracker.getImagePoint(54), lipWidth)));
        drawCurve2D(innerLip);

        // if mouth is open than some amount, draw this line
        if(mouthHysteresis.set(tracker.getGesture(ofxFaceTracker::MOUTH_HEIGHT) > 2)) {
            ofPolyline innerLowerLip;
            innerLowerLip.addVertex(toVec3(glm::mix(tracker.getImagePoint(65), tracker.getImagePoint(48), lipWidth)));
            innerLowerLip.addVertex(toVec3(tracker.getImagePoint(65)));
            innerLowerLip.addVertex(toVec3(tracker.getImagePoint(64)));
            innerLowerLip.addVertex(toVec3(tracker.getImagePoint(63)));
            innerLowerLip.addVertex(toVec3(glm::mix(tracker.getImagePoint(63), tracker.getImagePoint(54), lipWidth)));
            drawCurve2D(innerLowerLip);
        }

        // nose
        ofPolyline nose;
        nose.addVertex(toVec3(glm::mix(tracker.getImagePoint(31), tracker.getImagePoint(4), nostrilWidth)));
        nose.addVertex(toVec3(tracker.getImagePoint(31)));
        nose.addVertex(toVec3(tracker.getImagePoint(32)));
        nose.addVertex(toVec3(tracker.getImagePoint(33)));
        nose.addVertex(toVec3(tracker.getImagePoint(34)));
        nose.addVertex(toVec3(tracker.getImagePoint(35)));
        nose.addVertex(toVec3(glm::mix(tracker.getImagePoint(35), tracker.getImagePoint(12), nostrilWidth)));
        drawCurve2D(nose);

        ofPolyline upperLip;
        upperLip.addVertex(toVec3(glm::mix(tracker.getImagePoint(49), tracker.getImagePoint(48), lipWidth)));
        upperLip.addVertex(toVec3(tracker.getImagePoint(49)));
        upperLip.addVertex(toVec3(tracker.getImagePoint(50)));
        upperLip.addVertex(toVec3(tracker.getImagePoint(51)));
        upperLip.addVertex(toVec3(tracker.getImagePoint(52)));
        upperLip.addVertex(toVec3(tracker.getImagePoint(53)));
        upperLip.addVertex(toVec3(glm::mix(tracker.getImagePoint(53), tracker.getImagePoint(54), lipWidth)));
        drawCurve2D(upperLip);

        drawCurve2D(tracker.getImageFeature(ofxFaceTracker::LEFT_EYE_TOP));
        drawCurve2D(tracker.getImageFeature(ofxFaceTracker::RIGHT_EYE_TOP));
        drawCurve2D(tracker.getImageFeature(ofxFaceTracker::LEFT_EYEBROW));
        drawCurve2D(tracker.getImageFeature(ofxFaceTracker::RIGHT_EYEBROW));
        drawCurve2D(tracker.getImageFeature(ofxFaceTracker::JAW));
    }
};
