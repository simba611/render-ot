#include <lightwave.hpp>

namespace lightwave
{

    /**
     * @brief A perspective camera with a given field of view angle and transform.
     *
     * In local coordinates (before applying m_transform), the camera looks in positive z direction [0,0,1].
     * Pixels on the left side of the image ( @code normalized.x < 0 @endcode ) are directed in negative x
     * direction ( @code ray.direction.x < 0 ), and pixels at the bottom of the image ( @code normalized.y < 0 @endcode )
     * are directed in negative y direction ( @code ray.direction.y < 0 ).
     */
    class Perspective : public Camera
    {
    private:
        float x_scaled;
        float y_scaled; 
    public:
        Perspective(const Properties &properties)
            : Camera(properties){

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
            Vector direction(normalized.x() * x_scaled , normalized.y() * y_scaled, 1.f);

            Ray ray(Vector(0.f, 0.f, 0.f), direction.normalized()); 
            ray = m_transform->apply(ray);

            return CameraSample{
                .ray = ray,
                .weight = Color(1.0f)};
        }

        std::string toString() const override
        {
            return tfm::format(
                "Perspective[\n"
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

REGISTER_CAMERA(Perspective, "perspective")
