#include <lightwave.hpp>

namespace lightwave {

class CheckerboardTexture : public Texture {
private:
    Color color0;
    Color color1;
    Vector2 scale;
public:
    CheckerboardTexture(const Properties &properties) {
        color0 = properties.get<Color>("color0");
        color1 = properties.get<Color>("color1");
        scale = properties.get<Vector2>("scale"); 
    }

    Color evaluate(const Point2 &uv) const override {
        float new_u = uv[0] * scale[0];
        float new_v = uv[1] * scale[1];

        if (((((int)floor(new_u)) % 2)) ^ ((((int)floor(new_v)) % 2)))
            return color1; 
        else
            return color0;
    }

    std::string toString() const override {
        return tfm::format("CheckerboardTexture[\n"
                           "  color0 = %s\n"
                           "  color1 = %s\n"
                           "  scale  = %s\n"
                           "]",
                           indent(color0),
                           indent(color1),
                           indent(scale));
    }
};

} // namespace lightwave

REGISTER_TEXTURE(CheckerboardTexture, "checkerboard")
