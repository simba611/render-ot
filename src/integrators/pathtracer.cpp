#include <lightwave.hpp>

namespace lightwave {

class PathTracerIntegrator : public SamplingIntegrator {
private:
    int depth;
public:
    PathTracerIntegrator(const Properties &properties)
    : SamplingIntegrator(properties) {
        depth = properties.get<int>("depth", 2);
    }

    Color Li(const Ray &ray, Sampler &rng) override {
        Color ret = Color(0.f);
        Color weight = Color(1.f);
        Ray cur_ray = ray;

        for (int i = 0 ; i < depth ; i++) {
            Intersection its = m_scene -> intersect(cur_ray, rng);
            if (its) {
                ret += (its.evaluateEmission() * weight);

                if (i == (depth - 1)) {
                    break;
                }    

                if (m_scene->hasLights()) {
                    LightSample lightSample = m_scene -> sampleLight(rng);
                    DirectLightSample l = lightSample.light -> sampleDirect(cur_ray(its.t), rng);
                    
                    Ray r = Ray(cur_ray(its.t), l.wi);
                    bool inter_light = m_scene -> intersect(r, l.distance, rng);

                    if((!inter_light) && (!(lightSample.light -> canBeIntersected()))) {
                        BsdfEval light_bsdf = its.evaluateBsdf(l.wi);
                        ret += (((light_bsdf.value * l.weight) / lightSample.probability) * weight);
                    }
                }

                BsdfSample smp = its.sampleBsdf(rng);
                weight *= smp.weight;
                cur_ray = Ray(cur_ray(its.t), smp.wi).normalized();
            }
            else {
                return ret + ((m_scene -> evaluateBackground(cur_ray.direction).value) * weight);
            }
        }

        return ret;
    }

    /// @brief An optional textual representation of this class, which can be useful for debugging. 
    std::string toString() const override {
        return tfm::format(
            "PathTracerIntegrator[\n"
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
REGISTER_INTEGRATOR(PathTracerIntegrator, "pathtracer")