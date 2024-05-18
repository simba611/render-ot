#include "fresnel.hpp"
#include "microfacet.hpp"
#include <lightwave.hpp>

namespace lightwave {

class RoughConductor : public Bsdf {
    ref<Texture> m_reflectance;
    ref<Texture> m_roughness;

public:
    RoughConductor(const Properties &properties) {
        m_reflectance = properties.get<Texture>("reflectance");
        m_roughness   = properties.get<Texture>("roughness");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // Using the squared roughness parameter results in a more gradual
        // transition from specular to rough. For numerical stability, we avoid
        // extremely specular distributions (alpha values below 10^-3)
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));

        Vector sampledNormal = (wi + wo).normalized();
        return BsdfEval((m_reflectance -> evaluate(uv) * lightwave::microfacet::smithG1(alpha, sampledNormal, wo) * lightwave::microfacet::smithG1(alpha, sampledNormal, wi) * lightwave::microfacet::evaluateGGX(alpha, sampledNormal) / (4 * Frame::absCosTheta(wo))));

        // hints:
        // * the microfacet normal can be computed from `wi' and `wo'
    }


    Color albedo(const Point2 &uv) {
        return Color(1.f);
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));

        Vector norm = lightwave::microfacet::sampleGGXVNDF(alpha, wo.normalized(), rng.next2D()).normalized();
        Vector wi = reflect(wo.normalized(), norm.normalized()).normalized();

        return BsdfSample(wi, (m_reflectance -> evaluate(uv) * lightwave::microfacet::smithG1(alpha, norm, wi)));
        
        
        // hints:
        // * do not forget to cancel out as many terms from your equations as possible!
        //   (the resulting sample weight is only a product of two factors)
    }

    std::string toString() const override {
        return tfm::format("RoughConductor[\n"
                           "  reflectance = %s,\n"
                           "  roughness = %s\n"
                           "]",
                           indent(m_reflectance), indent(m_roughness));
    }
};

} // namespace lightwave

REGISTER_BSDF(RoughConductor, "roughconductor")
