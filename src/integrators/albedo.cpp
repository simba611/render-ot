#include <lightwave.hpp>

namespace lightwave {

class AlbedoIntegrator : public SamplingIntegrator {
private:
    bool remap;    
public:
    AlbedoIntegrator(const Properties &properties)
    : SamplingIntegrator(properties) {
    }

    Color Li(const Ray &ray, Sampler &rng) override {
        // Intersect the ray with the scene geometry.
        Intersection its = m_scene -> intersect(ray, rng);
        Color albedo = Color();
        if (its)
            albedo = its.evaluateAlbedo();

        return albedo;
    }

    /// @brief An optional textual representation of this class, which can be useful for debugging. 
    std::string toString() const override {
        return tfm::format(
            "NormalIntegrator[\n"
            "]"
        );
    }
};

}

REGISTER_INTEGRATOR(AlbedoIntegrator, "albedo")