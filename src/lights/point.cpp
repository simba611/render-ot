#include <lightwave.hpp>

namespace lightwave {

class PointLight final : public Light {
private:
    Point pos;
    Color power; 

public:
    PointLight(const Properties &properties) {
        pos = properties.get<Point>("position");
        power = properties.get<Color>("power");
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        DirectLightSample d;
        d.distance = (pos - origin).length();
        d.weight = power / (4 * Pi * d.distance * d.distance);
        d.wi = (pos - origin).normalized();
        return d;
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format("PointLight[\n"
                           "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(PointLight, "point")
