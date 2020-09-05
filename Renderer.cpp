#include "Renderer.hpp"

#include <thread>
#include <future>

#include <SFML/Graphics.hpp>

using namespace std;

Renderer::Renderer(int width, int height) :
    width_(width),
    height_(height)
{
    pixels_.resize(width * height * 4, 0);
}

std::tuple<const Figure *, float> Renderer::GetRayIntersection(const glm::vec3& orig, const glm::vec3& dir,
                                                               float min_depth, float max_depth) const
{
    float t = max_depth;
    const Figure * closest_ptr = nullptr;
    for(const auto& x : figures_)
    {
        auto coefs = x->GetIntersectionCoef(orig, dir);
        for(auto coef : coefs)
            if(coef >= min_depth &&
               coef <= max_depth &&
               coef < t)
            {
                t = coef;
                closest_ptr = x.get();
            }
    }
    return make_tuple(closest_ptr, t);
}

Color Renderer::RayTrace(const glm::vec3& orig, const glm::vec3& dir, size_t depth) const
{
    auto [closest_ptr, t] = GetRayIntersection(orig, dir,
                      settings_.draw_depth_min, settings_.draw_depth_max);
    if(!closest_ptr)
        return settings_.default_color;

    float intencity = light_.ambient.GetIntencity();
    glm::vec3 P = dir * t + orig;
    glm::vec3 N = closest_ptr->GetN(P);

    auto calc_light = [&N, &dir](const Light& light, const Figure& figure, const glm::vec3& L)
    {
        float N_dot_L = glm::dot(N, L);
        float intencity = 0.0f;
        if(N_dot_L > 0.0f)
            intencity += (N_dot_L / (glm::length(N) * glm::length(L))) * light.GetIntencity();

        if(auto spec = figure.GetSpec(); spec.has_value())
        {
            glm::vec3 V = -dir;
            glm::vec3 R = 2.0f * N * N_dot_L - L;
            float R_dot_V = glm::dot(R, V);
            if(R_dot_V > 0)
                intencity += light.GetIntencity() * glm::pow(R_dot_V / (glm::length(R) * glm::length(V)), spec.value());
        }

        return intencity;
    };


    for(const auto& point_src : light_.point_sources)
    {
        glm::vec3 L = point_src.GetOrigin() - P;
        if(auto [figure_ptr, dummy] =
                GetRayIntersection(P, L,
                         point_src.GetMinDepth(),
                         point_src.GetMaxDepth());
                figure_ptr)
            continue;
        intencity += calc_light(point_src, *closest_ptr, L);
        if(intencity > 1.0f) break;
    }


    for(const auto& directional_src : light_.directional_sources)
    {
        glm::vec3 L = -directional_src.GetDirection();
        if(auto [figure_ptr, dummy] =
                GetRayIntersection(P, L,
                         directional_src.GetMinDepth(),
                         directional_src.GetMaxDepth());
                figure_ptr)
            continue;
        intencity += calc_light(directional_src, *closest_ptr, L);
        if(intencity > 1.0f) break;
    }

    intencity = intencity > 1.0f ? 1.0f : intencity;

    float reflect_coef = closest_ptr->GetReflectionCoef();

    Color color_out = closest_ptr->GetColor() * intencity;

    if(depth == 0 || reflect_coef <= 0)
        return color_out;

    return color_out * (1.0 - reflect_coef) + reflect_coef
            * RayTrace(P, closest_ptr->ReflectedRay(P, dir), depth - 1);
}

const vector<uint8_t>& Renderer::RenderToPixelArray()
{
    float z_distance = glm::abs((width_ / 2) / glm::tan(settings_.field_of_view_hor / 2.0f));

    uint8_t threads = thread::hardware_concurrency();
    if(threads != 1)
        threads = threads - threads % 2;

    auto i_base = glm::cross(settings_.view_up, settings_.view_dir);

    glm::mat3x3 translation = {
        {i_base.x, settings_.view_up.x, settings_.view_dir.x},
        {i_base.y, settings_.view_up.y, settings_.view_dir.y},
        {i_base.z, settings_.view_up.z, settings_.view_dir.z}
    };

    vector<future<void>> render_threads;
    auto render =  [this, &translation](int branch, int threads, float z_distance){
        for(int i = height_ / 2 - branch; i > -height_ / 2; i -= threads)
        {
            for(int j = -width_ / 2; j <= width_ / 2 ; j++)
            {
                int index = glm::abs(height_ / 2 - i) * width_ * 4 + glm::abs(-width_ / 2 - j) * 4;

                auto color = RayTrace(settings_.view_point,
                                      translation * glm::vec3(j - (j >= 0), i + (i <= 0), z_distance) - settings_.view_point,
                                      settings_.trace_depth);

                pixels_[index++] = static_cast<uint8_t>(color.r);
                pixels_[index++] = static_cast<uint8_t>(color.g);
                pixels_[index++] = static_cast<uint8_t>(color.b);
                pixels_[index] = static_cast<uint8_t>(color.a);
            }
        }
    };

    for(int i = 0; i < threads; i++)
        render_threads.push_back(async(render, i, threads, z_distance));

    for(auto & x: render_threads)
        x.get();

    return pixels_;
}


Renderer &Renderer::SetView(glm::vec3 view, glm::vec3 dir, glm::vec3 up)
{
    settings_.view_point = move(view);
    settings_.view_dir = dir / glm::length(dir);
    settings_.view_up = up / glm::length(up);

    return *this;
}

Renderer &Renderer::SetTraceDepth(uint8_t depth)
{
    settings_.trace_depth = depth;

    return *this;
}

Renderer &Renderer::SetFieldOfViewHor(float fov)
{
    settings_.field_of_view_hor = fov;

    return *this;
}

Renderer &Renderer::SetDefaultColor(Color color)
{
    settings_.default_color = color;

    return *this;
}

Renderer &Renderer::AddDirectionalLightSource(glm::vec3 dir, float intencity)
{
    light_.directional_sources.emplace_back(dir, intencity);

    return *this;
}

Renderer &Renderer::AddPointLightSource(glm::vec3 origin, float intencity)
{
    light_.point_sources.emplace_back(origin, intencity);

    return *this;
}

Renderer &Renderer::SetAmbientLight(float intencity)
{
    light_.ambient = AmbientLight(intencity);

    return *this;
}

Renderer &Renderer::SetDrawDepthMax(float depth)
{
    settings_.draw_depth_max = depth;

    return *this;
}

Renderer &Renderer::SetDrawDepthMin(float depth)
{
    settings_.draw_depth_min = depth;

    return *this;
}

Renderer &Renderer::AddSphere(glm::vec3 center, float radius, Color color, float reflect, float spec)
{
    figures_.push_back(make_unique<Sphere>(center, radius, color, reflect, spec));

    return *this;
}

Renderer &Renderer::AddPlane(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, Color color, float reflect, float spec)
{
    figures_.push_back(make_unique<Plane>(p1, p2, p3, color, reflect, spec));

    return *this;
}

Renderer &Renderer::AddFigure(std::unique_ptr<Figure> figure)
{
    figures_.push_back(move(figure));

    return *this;
}
