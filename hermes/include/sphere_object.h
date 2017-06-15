#ifndef SPHERE_OBJECT_H
#define SPHERE_OBJECT_H

#include "object.h"
#include "material.h"

// 球形
class SphereObject: public Object
{
public:
    SphereObject(const Point& center, double radius, Material::ptr material);

    // 渲染，返回RGB
    virtual RGB render(const Ray& ray, double t, const Scene* scene, int deep, const vector<Light::ptr>& possible_ligts) const;

    // 光线碰撞，返回t使得P=ray.s+t*rat.d在图形上
    virtual double intersect(const Ray& ray) const;

private:
    Point center;
    double radius;
    Material::ptr material;
};

#endif // SPHERE_OBJECT_H