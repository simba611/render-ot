#include "fresnel.hpp"
#include <lightwave.hpp>

namespace lightwave {

class Dielectric : public Bsdf {
    ref<Texture> m_ior;
    ref<Texture> m_reflectance;
    ref<Texture> m_transmittance;

public:
    Dielectric(const Properties &properties) {
        m_ior           = properties.get<Texture>("ior");
        m_reflectance   = properties.get<Texture>("reflectance");
        m_transmittance = properties.get<Texture>("transmittance");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // the probability of a light sample picking exactly the direction `wi'
        // that results from reflecting or refracting `wo' is zero, hence we can
        // just ignore that case and always return black
        return BsdfEval::invalid();
    }

    Color albedo(const Point2 &uv) {
        return Color(1.f);
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        float eta = m_ior -> scalar(uv);
        float to_est = wo.z();
        if (wo.z() < 0) {
            eta = 1 / eta;
            to_est = -to_est;
        }

        float prob = fresnelDielectric(to_est, eta);

        if (prob == -1) {
            return BsdfSample(Vector(-wo.x(), -wo.y(), wo.z()), m_reflectance -> evaluate(uv));
        }

        if (rng.next() < prob) {
            return BsdfSample(Vector(-wo.x(), -wo.y(), wo.z()), m_reflectance -> evaluate(uv));
        }
        else {
            Vector ref;
            if (wo.z() < 0) {
                ref = refract(Vector(wo.x(), wo.y(), -wo.z()), Vector(0.f, 0.f, -1.f), eta);
            }
            else {
                ref = refract(wo, Vector(0.f, 0.f, 1.f), eta);
            }

            if (ref == Vector(0.f, 0.f, 0.f)) {
                return BsdfSample(Vector(-wo.x(), -wo.y(), wo.z()), m_reflectance -> evaluate(uv));
            }
            
            return BsdfSample(ref, (m_transmittance -> evaluate(uv)) / (eta * eta));
        }
    }

    std::string toString() const override {
        return tfm::format("Dielectric[\n"
                           "  ior           = %s,\n"
                           "  reflectance   = %s,\n"
                           "  transmittance = %s\n"
                           "]",
                           indent(m_ior), indent(m_reflectance),
                           indent(m_transmittance));
    }
};

} // namespace lightwave

REGISTER_BSDF(Dielectric, "dielectric")
