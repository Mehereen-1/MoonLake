#ifndef TRAIN_STATION_H
#define TRAIN_STATION_H

#include <glm/glm.hpp>
#include "Shader.h"
#include "primitives/Cube.h"
#include "primitives/Cylinder.h"
#include "primitives/Sphere.h"

// ---------------------------------------------------------------------------
// TrainStation
// A rustic, picturesque lakeside meadow boarding station:
// - Raised wooden platform with timber deck and curb matching train door level.
// - Boarding steps and ramp connecting smoothly to the walking path from the dock.
// - Open-air timber canopy shelter with heavy cedar posts, knee-braces, and pitched roof.
// - Station amenities: "MEADOW STATION" carved hanging sign, slatted waiting benches,
//   warm platform lantern posts, timetable board, and railings with flower planter boxes.
// ---------------------------------------------------------------------------
class TrainStation
{
public:
    TrainStation(const glm::vec3& pos = glm::vec3(8.5f, -0.66f, 29.2f), float rotY = -8.0f, float scale = 1.0f);

    void setPosition(const glm::vec3& pos) { position = pos; }
    void setRotation(float rotY)           { rotationY = rotY; }
    void setScale(float s)                 { scaleFactor = s; }

    const glm::vec3& getPosition() const   { return position; }
    float getRotation() const              { return rotationY; }
    float getScale() const                 { return scaleFactor; }

    void draw(const Shader& shader,
              const Cube& unitCube,
              const Cylinder& unitCylinder,
              const Sphere& unitSphere,
              float currentTime) const;

private:
    glm::vec3 position;
    float rotationY;
    float scaleFactor;
};

#endif // TRAIN_STATION_H
