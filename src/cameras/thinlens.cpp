#include <lightwave.hpp>

namespace lightwave
{

    class ThinLens : public Camera
    {
    private:
        float x_scaled;
        float y_scaled;
        float radius;
        float focal_length;
    public:
        ThinLens(const Properties &properties)
            : Camera(properties){
                radius = properties.get<float>("radius");
                focal_length = properties.get<float>("focal_length");

                const float fov = (Pi * properties.get<float>("fov")) / 180.0f;
                const std::string fov_dir = properties.get<std::string>("fovAxis");

                if (fov_dir == "x") {
                    const float h_w = ((float) m_resolution.y()) / ((float) m_resolution.x());
                    x_scaled = tan(fov / 2);
                    y_scaled = x_scaled * h_w;
                }
                else if (fov_dir == "y") {
                    const float w_h = ((float) m_resolution.x()) / ((float) m_resolution.y());
                    y_scaled = tan(fov / 2);
                    x_scaled = y_scaled * w_h;
                }
                else {
                    fprintf(stderr, "fovAxis not defined\n");
                }
              }

        CameraSample sample(const Point2 &normalized, Sampler &rng) const override
        {
            Point2 p = (Point2)(radius * (Vector2)squareToUniformDiskConcentric(rng.next2D()));   

            Vector direction((normalized.x() * x_scaled) - p.x(), (normalized.y() * y_scaled) - p.y(), focal_length);

            Ray ray(Vector(p.x(), p.y(), 0.f), direction.normalized());
            ray = m_transform->apply(ray);

            return CameraSample{
                .ray = ray,
                .weight = Color(1.0f)};
        }

        std::string toString() const override
        {
            return tfm::format(
                "ThinLens[\n"
                "  width = %d,\n"
                "  height = %d,\n"
                "  transform = %s,\n"
                "]",
                m_resolution.x(),
                m_resolution.y(),
                indent(m_transform));
        }
    };
}

REGISTER_CAMERA(ThinLens, "thinlens")
