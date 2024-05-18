#include <lightwave.hpp>

namespace lightwave {

class NormalIntegrator : public SamplingIntegrator {
private:
    bool remap;    
public:
    NormalIntegrator(const Properties &properties)
    : SamplingIntegrator(properties) {
        remap = properties.get<bool>("remap", true);
    }

    Color Li(const Ray &ray, Sampler &rng) override {
        // Intersect the ray with the scene geometry.
        Intersection its = m_scene -> intersect(ray, rng);

        Vector normal = Vector(0.f, 0.f, 0.f);
        if (its) {
            // Get the normal at the intersection point.
            normal = its.frame.normal;
        }

        if (remap) {
            normal = (normal + Vector(1.f, 1.f, 1.f)) / 2;
        }
        
        return Color(normal);
    }

    /// @brief An optional textual representation of this class, which can be useful for debugging. 
    std::string toString() const override {
        return tfm::format(
            "NormalIntegrator[\n"
            "  sampler = %s,\n"
            "  image = %s,\n"
            "]",
            indent(m_sampler),
            indent(m_image)
        );
    }
};

}

// this informs lightwave to use our class CameraIntegrator whenever a <integrator type="camera" /> is found in a scene file
REGISTER_INTEGRATOR(NormalIntegrator, "normals")