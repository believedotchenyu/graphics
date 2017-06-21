#ifndef SCENE_H
#define SCENE_H

#include "const.hpp"
#include "geometry.hpp"
#include "light.hpp"
#include "object.hpp"
#include "camera.hpp"
#include <opencv2/opencv.hpp>

// 场景
class Scene
{
public:
    Scene(int maxdeep, int shade_quality);
    ~Scene();

    const RGB background = RGB(0.0, 0.0, 0.0);

    Object::ptr putObject(Object::ptr obj);
    Light::ptr putLight(Light::ptr light);
    Camera::ptr setCamera(Camera::ptr camera);

    RGB rayTracing(const Ray& ray, const RGB& weight, Object::ptr inner_obj, int remaindeep = -1) const; // 光线追踪
    cv::Mat renderRayTracing();

private:
    int objIndex(Object::ptr obj) const;

    const int maxdeep;
    const int shade_quality;
    vector<Object::ptr> objs;
    vector<Light::ptr> lights;
    Camera::ptr camera;
};

#endif // SCENE_H