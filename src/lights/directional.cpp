#include <lightwave.hpp>

namespace lightwave {

class DirectionalLight final : public Light {
private:
    Vector dir;
    Color power; 

public:
    DirectionalLight(const Properties &properties) {
        dir = properties.get<Vector>("direction").normalized();
        power = properties.get<Color>("intensity");
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        DirectLightSample d;
        d.distance = Infinity;
        d.weight = power;
        d.wi = dir;
        return d;
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format("DirectionalLight[\n"
                           "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(DirectionalLight, "directional")
