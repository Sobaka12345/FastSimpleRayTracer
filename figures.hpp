#ifndef FIGURES_HPP
#define FIGURES_HPP

#include <glm/vec3.hpp>
#include <glm/mat2x2.hpp>
#include <glm/geometric.hpp>

#include <optional>
#include <vector>

struct Color: public glm::vec4
{
    Color(glm::vec3 color) :
        glm::vec4(color.r, color.g, color.b, 255)
    {}

    Color(glm::vec4 color) :
        glm::vec4(color)
    {}

    Color(float r, float g, float b, float a = 255.0f) :
        glm::vec4(r, g, b, a)
    {}
};

class Figure
{
protected:
    Color color_;
    float reflect_;
    std::optional<uint16_t> spec_;

    Figure(Color color, float reflect, std::optional<uint16_t> spec = std::nullopt) :
        color_(color),
        reflect_(reflect),
        spec_(spec)
    {}


public:
    virtual ~Figure() = default;
    virtual std::vector<float> GetIntersectionCoef(glm::vec3 orig, glm::vec3 dir) const = 0;
    virtual glm::vec3 GetN(glm::vec3 vec) const = 0;

    Color GetColor() const
    {
        return color_;
    }

    float GetReflectionCoef() const
    {
        return reflect_;
    }

    std::optional<uint16_t> GetSpec() const
    {
        return spec_;
    }

    glm::vec3 ReflectedRay(glm::vec3 vec, glm::vec3 dir) const
    {
        glm::vec3 N = GetN(vec);
        glm::vec3 R = -dir;
        return (2.0f * glm::dot(N, R) * N) - R;
    }
};

class Sphere: public Figure
{
    glm::vec3 center_;
    float radius_;

public:
    Sphere(Color center, float radius, glm::vec3 color, float reflect,
           std::optional<uint16_t> spec = std::nullopt) :
        Figure(std::move(color), reflect, spec),
        center_(center),
        radius_(radius)
    {}

    glm::vec3 GetN(glm::vec3 sphere_point) const override
    {
        return (sphere_point - center_) / glm::length(sphere_point - center_);
    }

    std::vector<float> GetIntersectionCoef(glm::vec3 orig, glm::vec3 dir) const override
    {
        glm::vec3  viewToCenter = orig - center_;

        float A = glm::dot(dir, dir);
        float B = glm::dot(viewToCenter, dir) * 2.0f;
        float C = glm::dot(viewToCenter, viewToCenter) - radius_ * radius_;

        float D = B * B - 4 * A * C;

        if (D < 0.0f)
            return std::vector<float>();

        float x1 = (-B - sqrt(D)) / (2.0f * A);
        float x2 = (-B + sqrt(D)) / (2.0f * A);

        return std::vector<float>{ x1, x2 };
    }

};

class Plane: public Figure
{
    glm::vec3 perpendicular_;
    float d_;

public:
    Plane(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, Color color, float reflect,
          std::optional<uint16_t> spec = std::nullopt) :
        Figure(std::move(color), reflect, spec)
    {
        glm::mat2x2 M1(
            glm::vec2(p2.y - p1.y, p2.z - p1.z),
            glm::vec2(p3.y - p1.y, p3.z - p1.z)
        );
        glm::mat2x2 M2(
            glm::vec2(p2.x - p1.x, p2.z - p1.z),
            glm::vec2(p3.x - p1.x, p3.z - p1.z)
        );
        glm::mat2x2 M3(
            glm::vec2(p2.x - p1.x, p2.y - p1.y),
            glm::vec2(p3.x - p1.x, p3.y - p1.y)
        );

        float A = glm::determinant(M1);
        float B = -glm::determinant(M2);
        float C = glm::determinant(M3);
        float d = p1.y * (-B) - p1.x * A - p1.z * C;

        perpendicular_ = glm::vec3(A, B, C);
        d_ = d;
    }

    glm::vec3 GetN(glm::vec3) const override
    {
        float sum = perpendicular_.x + perpendicular_.y + perpendicular_.z;
        return -glm::vec3(perpendicular_.x, perpendicular_.y, perpendicular_.z) * (1.0f / sum);
    }

    std::vector<float> GetIntersectionCoef(glm::vec3 orig, glm::vec3 dir) const override
    {
        float val = glm::dot(perpendicular_, dir);
        if (val == 0)
            return std::vector<float>();
        float x = ((-1) * (glm::dot(perpendicular_, orig) + d_)) / val;


        return std::vector<float>{ (float)x };
    }
};


#endif // FIGURES_HPP
