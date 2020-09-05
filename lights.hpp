#ifndef LIGHTS_HPP
#define LIGHTS_HPP

#include <limits>
#include <glm/vec3.hpp>

class Light
{
protected:
    float intencity_;
    float min_depth_;
    float max_depth_;

public:
    Light(float intencity, float min_depth, float max_depth) :
        intencity_(intencity),
        min_depth_(min_depth),
        max_depth_(max_depth)
    {}

    float GetIntencity() const
    {
        return intencity_;
    }

    float GetMinDepth() const
    {
        return min_depth_;
    }

    float GetMaxDepth() const
    {
        return max_depth_;
    }

    void SetMinDepth(float depth)
    {
        min_depth_ = depth;
    }

    void SetMaxDepth(float depth)
    {
        max_depth_ = depth;
    }
};

class AmbientLight: public Light
{
public:
    AmbientLight(float intencity) :
        Light(intencity, 0.0f, 0.0f)
    {}
};

class PointLight: public Light
{
    glm::vec3 origin_;
public:
    PointLight(glm::vec3 origin, float intencity) :
        Light(intencity, 0.001f, 1.0f),
        origin_(origin)
    {}

    const glm::vec3& GetOrigin() const
    {
        return origin_;
    }

   glm::vec3 CalcL(glm::vec3 point) const
   {
       return origin_ - point;
   }
};

class DirectionalLight: public Light
{
    glm::vec3 direction_;
public:
    DirectionalLight(glm::vec3 direction, float intencity) :
        Light(intencity, 0.001f, std::numeric_limits<float>::max()),
        direction_(direction)
    {}

    const glm::vec3& GetDirection() const
    {
        return direction_;
    }

    glm::vec3 CalcL() const
    {
        return -direction_;
    }
};

#endif // LIGHTS_HPP
