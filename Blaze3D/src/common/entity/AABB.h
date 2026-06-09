#pragma once

#include "thirdparty/glm/ext.hpp"

struct AABB
{
    float minX;
    float minY;
    float minZ;
    float maxX;
    float maxY;
    float maxZ;
    AABB() = default;
    AABB(float x0, float y0, float z0, float x1, float y1, float z1);
    AABB expand(float n, float n2, float n3);
    AABB grow(float n, float n2, float n3);
    AABB cloneMove(float n, float n2, float n3);
    float clipXCollide(AABB& aabb, float n);
    float clipYCollide(AABB& aabb, float n);
    float clipZCollide(AABB& aabb, float n);
    bool intersects(AABB& axisalignedbb);
    bool intersectsRay(const glm::vec3& org, const glm::vec3& dir);
    void move(float n, float n2, float n3);
    AABB copy();
    bool intersects(float px, float py, float pz) const;
};

