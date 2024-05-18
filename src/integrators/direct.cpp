#include <lightwave.hpp>

namespace lightwave {

class DirectIntegrator : public SamplingIntegrator {
private:
 
public:
    DirectIntegrator(const Properties &properties)
    : SamplingIntegrator(properties) {
    }

    Color Li(const Ray &ray, Sampler &rng) override {
        // Intersect the ray with the scene geometry.
        Intersection its = m_scene -> intersect(ray, rng);

        if (its) {
            Color emission = its.evaluateEmission();
            
            if (m_scene->hasLights()) {
                LightSample lightSample = m_scene -> sampleLight(rng);
                DirectLightSample l = lightSample.light -> sampleDirect(ray(its.t), rng);
                
                Ray r = Ray(ray(its.t), l.wi);
                bool inter_light = m_scene -> intersect(r, l.distance, rng);

                if((!inter_light) && (!(lightSample.light -> canBeIntersected()))) {
                    BsdfEval light_bsdf = its.evaluateBsdf(l.wi);
                    emission += ((light_bsdf.value * l.weight) / lightSample.probability);
                }
            }

            BsdfSample smp = its.sampleBsdf(rng);

            Ray n1 = Ray(ray(its.t), smp.wi).normalized();
            Intersection its_next = m_scene -> intersect(n1, rng);
            if (!its_next) {
                return emission + (smp.weight * (m_scene -> evaluateBackground(n1.direction)).value);
            }
            return emission + (smp.weight * its_next.evaluateEmission());
        }
        
        return m_scene -> evaluateBackground(ray.direction).value;
    }

    /// @brief An optional textual representation of this class, which can be useful for debugging. 
    std::string toString() const override {
        return tfm::format(
            "DirectIntegrator[\n"
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
REGISTER_INTEGRATOR(DirectIntegrator, "direct")