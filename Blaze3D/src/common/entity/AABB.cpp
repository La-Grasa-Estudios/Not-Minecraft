#include "AABB.h"

AABB::AABB(float d, float e, float f, float g, float h, float i) {
    minX = d;
    minY = e;
    minZ = f;
    maxX = g;
    maxY = h;
    maxZ = i;
}

AABB AABB::expand(float n, float n2, float n3) {
    float x0 = this->minX;
    float y0 = this->minY;
    float z0 = this->minZ;
    float x2 = this->maxX;
    float y2 = this->maxY;
    float z2 = this->maxZ;
    if (n < 0.0f) {
        x0 += n;
    }
    if (n > 0.0f) {
        x2 += n;
    }
    if (n2 < 0.0f) {
        y0 += n2;
    }
    if (n2 > 0.0f) {
        y2 += n2;
    }
    if (n3 < 0.0f) {
        z0 += n3;
    }
    if (n3 > 0.0f) {
        z2 += n3;
    }
    return AABB(x0, y0, z0, x2, y2, z2);
}

AABB AABB::grow(float n, float n2, float n3) {
    float n4 = this->minX - n;
    float n5 = this->minY - n2;
    float n6 = this->minZ - n3;
    n += this->maxX;
    n2 += this->maxY;
    return AABB(n4, n5, n6, n, n2, this->maxZ + n3);
}

AABB AABB::cloneMove(float n, float n2, float n3) {
    return AABB(minX + n3, minY + n2, minZ + n3, maxX + n, maxY + n2, maxZ + n3);
}

float AABB::clipXCollide(AABB& aabb, float n) {
    float d = n;
    if (aabb.maxY <= minY || aabb.minY >= maxY)
    {
        return d;
    }
    if (aabb.maxZ <= minZ || aabb.minZ >= maxZ)
    {
        return d;
    }
    if (d > 0.0 && aabb.maxX <= minX)
    {
        float d1 = minX - aabb.maxX;
        if (d1 < d)
        {
            d = d1;
        }
    }
    if (d < 0.0 && aabb.minX >= maxX)
    {
        float d2 = maxX - aabb.minX;
        if (d2 > d)
        {
            d = d2;
        }
    }
    return d;
}

float AABB::clipYCollide(AABB& aabb, float n) {
    if (aabb.maxX <= minX || aabb.minX >= maxX) {
        return n;
    }
    if (aabb.maxZ <= minZ || aabb.minZ >= maxZ) {
        return n;
    }
    if (n > 0.0f && aabb.maxY <= minY) {
        float d1 = minY - aabb.maxY;
        if (d1 < n) {
            n = d1;
        }
    }
    if (n < 0.0f && aabb.minY >= maxY) {
        float d2 = maxY - aabb.minY;
        if (d2 > n) {
            n = d2;
        }
    }
    return n;
}

float AABB::clipZCollide(AABB& aabb, float n) {
    float d = n;
    if (aabb.maxX <= minX || aabb.minX >= maxX)
    {
        return d;
    }
    if (aabb.maxY <= minY || aabb.minY >= maxY)
    {
        return d;
    }
    if (d > 0.0 && aabb.maxZ <= minZ)
    {
        float d1 = minZ - aabb.maxZ;
        if (d1 < d)
        {
            d = d1;
        }
    }
    if (d < 0.0 && aabb.minZ >= maxZ)
    {
        float d2 = maxZ - aabb.minZ;
        if (d2 > d)
        {
            d = d2;
        }
    }
    return d;
}

bool AABB::intersects(AABB& axisalignedbb) {
    if (axisalignedbb.maxX <= minX || axisalignedbb.minX >= maxX)
    {
        return false;
    }
    if (axisalignedbb.maxY <= minY || axisalignedbb.minY >= maxY)
    {
        return false;
    }
    return axisalignedbb.maxZ > minZ && axisalignedbb.minZ < maxZ;
}

bool AABB::intersectsRay(const glm::vec3& org, const glm::vec3& dir)
{
	glm::vec3 invDir = 1.0f / dir;

	float t1 = (minX - org.x) * invDir.x;
	float t2 = (maxX - org.x) * invDir.x;
	float t3 = (minY - org.y) * invDir.y;
	float t4 = (maxY - org.y) * invDir.y;
	float t5 = (minZ - org.z) * invDir.z;
	float t6 = (maxZ - org.z) * invDir.z;
	float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
	float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

    if (tmax < 0.0f)
    {
        return false;
    }

    if (tmin > tmax)
    {
        return false;
    }

    return true;
}

void AABB::move(float n, float n2, float n3) {
    this->minX += n;
    this->minY += n2;
    this->minZ += n3;
    this->maxX += n;
    this->maxY += n2;
    this->maxZ += n3;
}

AABB AABB::copy() {
    return AABB(this->minX, this->minY, this->minZ, this->maxX, this->maxY, this->maxZ);
}

bool AABB::intersects(float px, float py, float pz) const
{
    return (px >= minX && px <= maxX) &&
        (py >= minY && py <= maxY) &&
        (pz >= minZ && pz <= maxZ);
}

