#include <lightwave.hpp>

#include "fresnel.hpp"
#include "microfacet.hpp"

namespace lightwave {

struct DiffuseLobe {
    Color color;

    BsdfEval evaluate(const Vector &wo, const Vector &wi) const {
        if (Frame::cosTheta(wi) > 0) {
            return BsdfEval((color * Frame::cosTheta(wi)) * InvPi);
        }

        return BsdfEval(Color(0.f));

        // hints:
        // * copy your diffuse bsdf evaluate here
        // * you do not need to query a texture, the albedo is given by `color`
    }

    BsdfSample sample(const Vector &wo, Sampler &rng) const {
        Vector next_ray = squareToCosineHemisphere(rng.next2D());
        return BsdfSample(next_ray, color);

        // hints:
        // * copy your diffuse bsdf evaluate here
        // * you do not need to query a texture, the albedo is given by `color`
    }
};

struct MetallicLobe {
    float alpha;
    Color color;

    BsdfEval evaluate(const Vector &wo, const Vector &wi) const {
        Vector sampledNormal = (wi + wo).normalized();
        return BsdfEval(color * lightwave::microfacet::smithG1(alpha, sampledNormal, wo) * lightwave::microfacet::smithG1(alpha, sampledNormal, wi) * lightwave::microfacet::evaluateGGX(alpha, sampledNormal) / (4 * Frame::absCosTheta(wo)));

        // hints:
        // * copy your roughconductor bsdf evaluate here
        // * you do not need to query textures
        //   * the reflectance is given by `color'
        //   * the variable `alpha' is already provided for you
    }

    BsdfSample sample(const Vector &wo, Sampler &rng) const {
        Vector norm = lightwave::microfacet::sampleGGXVNDF(alpha, wo.normalized(), rng.next2D()).normalized();
        Vector wi = reflect(wo.normalized(), norm.normalized()).normalized();

        return BsdfSample(wi, color * lightwave::microfacet::smithG1(alpha, norm, wi));

        // hints:
        // * copy your roughconductor bsdf sample here
        // * you do not need to query textures
        //   * the reflectance is given by `color'
        //   * the variable `alpha' is already provided for you
    }
};

class Principled : public Bsdf {
    ref<Texture> m_baseColor;
    ref<Texture> m_roughness;
    ref<Texture> m_metallic;
    ref<Texture> m_specular;

    struct Combination {
        float diffuseSelectionProb;
        DiffuseLobe diffuse;
        MetallicLobe metallic;
    };

    Combination combine(const Point2 &uv, const Vector &wo) const {
        const auto baseColor = m_baseColor->evaluate(uv);
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        const auto specular = m_specular->scalar(uv);
        const auto metallic = m_metallic->scalar(uv);
        const auto F =
            specular * schlick((1 - metallic) * 0.08f, Frame::cosTheta(wo));

        const DiffuseLobe diffuseLobe = {
            .color = (1 - F) * (1 - metallic) * baseColor,
        };
        const MetallicLobe metallicLobe = {
            .alpha = alpha,
            .color = F * Color(1) + (1 - F) * metallic * baseColor,
        };

        const auto diffuseAlbedo = diffuseLobe.color.mean();
        const auto totalAlbedo =
            diffuseLobe.color.mean() + metallicLobe.color.mean();
        return {
            .diffuseSelectionProb =
                totalAlbedo > 0 ? diffuseAlbedo / totalAlbedo : 1.0f,
            .diffuse  = diffuseLobe,
            .metallic = metallicLobe,
        };
    }

public:
    Principled(const Properties &properties) {
        m_baseColor = properties.get<Texture>("baseColor");
        m_roughness = properties.get<Texture>("roughness");
        m_metallic  = properties.get<Texture>("metallic");
        m_specular  = properties.get<Texture>("specular");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        const auto combination = combine(uv, wo);
        return BsdfEval(combination.diffuse.evaluate(wo, wi).value + combination.metallic.evaluate(wo, wi).value);

        // hint: evaluate `combination.diffuse` and `combination.metallic` and
        // combine their results
    }

    Color albedo(const Point2 &uv) {
        return m_baseColor -> evaluate(uv);
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        const auto combination = combine(uv, wo);
        if (rng.next() < combination.diffuseSelectionProb) {
            BsdfSample r = combination.diffuse.sample(wo, rng);
            return BsdfSample(r.wi, r.weight / combination.diffuseSelectionProb);
        }
        BsdfSample r = combination.metallic.sample(wo, rng);
        return BsdfSample(r.wi, r.weight / (1 - combination.diffuseSelectionProb));;

        // hint: sample either `combination.diffuse` (probability
        // `combination.diffuseSelectionProb`) or `combination.metallic`
    }

    std::string toString() const override {
        return tfm::format("Principled[\n"
                           "  baseColor = %s,\n"
                           "  roughness = %s,\n"
                           "  metallic  = %s,\n"
                           "  specular  = %s,\n"
                           "]",
                           indent(m_baseColor), indent(m_roughness),
                           indent(m_metallic), indent(m_specular));
    }
};

} // namespace lightwave

REGISTER_BSDF(Principled, "principled")
