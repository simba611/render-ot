#include <lightwave.hpp>

namespace lightwave {

class Diffuse : public Bsdf {
    ref<Texture> m_albedo;

public:
    Diffuse(const Properties &properties) {
        m_albedo = properties.get<Texture>("albedo");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        if (Frame::cosTheta(wi) > 0) {
            return BsdfEval((m_albedo -> evaluate(uv) * Frame::cosTheta(wi)) * InvPi);
        }

        return BsdfEval(Color(0.f));
    }

    Color albedo(const Point2 &uv) {
        return m_albedo -> evaluate(uv);
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        Vector next_ray = squareToCosineHemisphere(rng.next2D());
        return BsdfSample(next_ray, (m_albedo -> evaluate(uv)));
    }

    std::string toString() const override {
        return tfm::format("Diffuse[\n"
                           "  albedo = %s\n"
                           "]",
                           indent(m_albedo));
    }
};

} // namespace lightwave

REGISTER_BSDF(Diffuse, "diffuse")
