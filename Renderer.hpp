#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <memory>
#include <vector>
#include <limits>

#include <glm/ext.hpp>

#include "figures.hpp"
#include "lights.hpp"

class Renderer
{
    struct RenderSettings
    {
        glm::vec3 view_point = { 0.0f, 0.0f, 0.0f };
        glm::vec3 view_dir = { 0.0f, 0.0f, 1.0f };
        glm::vec3 view_up = { 0.0f, 1.0f, 0.0f };
        Color default_color = glm::vec4(0, 0, 0, 0);
        float field_of_view_hor = glm::pi<float>() / 3.0f;
        float draw_depth_max = std::numeric_limits<float>::max();
        float draw_depth_min = 0.00001f;
        uint8_t trace_depth = 3;
    } settings_;

    struct LightSources
    {
        AmbientLight ambient = AmbientLight(0.2f);
        std::vector<PointLight> point_sources;
        std::vector<DirectionalLight> directional_sources;
    } light_;

    std::vector<std::unique_ptr<Figure>> figures_;
    std::vector<uint8_t> pixels_;

    int width_, height_;

    std::tuple<const Figure*, float> GetRayIntersection(const glm::vec3& orig, const glm::vec3& dir, float min_depth, float max_depth) const;
    Color RayTrace(const glm::vec3 &orig, const glm::vec3 &dir, size_t depth) const;

public:
    Renderer(int width, int height);

    Renderer& SetViewCellDistance(float dist);
    Renderer& SetView(glm::vec3 view, glm::vec3 dir, glm::vec3 up);
    Renderer& SetDrawDepthMax(float depth);
    Renderer& SetDrawDepthMin(float depth);
    Renderer& SetTraceDepth(uint8_t depth);
    Renderer& SetFieldOfViewHor(float fov);
    Renderer& SetDefaultColor(Color color);

    Renderer& AddDirectionalLightSource(glm::vec3 dir, float intencity);
    Renderer& AddPointLightSource(glm::vec3 origin, float intencity);
    Renderer& SetAmbientLight(float intencity);

    Renderer& AddPlane(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, Color color, float reflect, float spec);
    Renderer& AddSphere(glm::vec3 center, float radius, Color color, float reflect, float spec);
    Renderer& AddFigure(std::unique_ptr<Figure> figure);

    const std::vector<uint8_t>& RenderToPixelArray();
};

#endif // RENDERER_HPP
