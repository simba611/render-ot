#include <lightwave.hpp>

namespace lightwave {

class ImageTexture : public Texture {
    enum class BorderMode {
        Clamp,
        Repeat,
    };

    enum class FilterMode {
        Nearest,
        Bilinear,
    };

    ref<Image> m_image;
    float m_exposure;
    BorderMode m_border;
    FilterMode m_filter;

public:
    ImageTexture(const Properties &properties) {
        if (properties.has("filename")) {
            m_image = std::make_shared<Image>(properties);
        } else {
            m_image = properties.getChild<Image>();
        }
        m_exposure = properties.get<float>("exposure", 1);

        m_border =
            properties.getEnum<BorderMode>("border", BorderMode::Repeat,
                                           {
                                               { "clamp", BorderMode::Clamp },
                                               { "repeat", BorderMode::Repeat },
                                           });

        m_filter = properties.getEnum<FilterMode>(
            "filter", FilterMode::Bilinear,
            {
                { "nearest", FilterMode::Nearest },
                { "bilinear", FilterMode::Bilinear },
            });
    }

    Color evaluate(const Point2 &uv) const override {
        double u, v;

        Point2i res = (m_image -> resolution());

        // Clamp mode
        if (m_border == BorderMode::Clamp) {
            if (uv[0] < 0) {
                u = 0;
            }
            else if (uv[0] > 1) {
                u = 1;
            }
            else {
                u = uv[0];
            }

            if (uv[1] < 0) {
                v = 0;
            }
            else if (uv[1] > 1) {
                v = 1;
            }
            else {
                v = uv[1];
            }
        }
        else { // Repeat mode
            u = uv[0] - floor(uv[0]);
            v = uv[1] - floor(uv[1]);
        }


        // Nearest neighbour
        if (m_filter == FilterMode::Nearest) {
            Point2i pix = Point2i(min((int)(u * res[0]), res[0] - 1), min((int)((1 - v) * res[1]), res[1] - 1));
            return (m_image -> get(pix)) * m_exposure;
        }

        // Bilinear interpolation
        float x = (u * res[0]) - 0.5f;
        float y = ((1 - v) * res[1]) - 0.5f;

        int xi = (int)floor(x);
        int yi = (int)floor(y);

        xi = max(0, min(xi, res[0] - 1));
        yi = max(0, min(yi, res[1] - 1));

        float dx = max(0, min(x - ((float)xi), 1));
        float dy = max(0, min(y - ((float)yi), 1));

        float lu = 1 - dx;
        float lv = 1 - dy;

        Color pix00 = m_image->get(Point2i(xi, yi));
        Color pix01 = m_image->get(Point2i(xi, min(yi + 1, res[1] - 1)));
        Color pix10 = m_image->get(Point2i(min(xi + 1, res[0] - 1), yi));
        Color pix11 = m_image->get(Point2i(min(xi + 1, res[0] - 1), min(yi + 1, res[1] - 1)));

        return ((lu * lv * pix00) + (lu * dy * pix01) + (dx * lv * pix10) + (dx * dy * pix11)) * m_exposure;
    }

    std::string toString() const override {
        return tfm::format("ImageTexture[\n"
                           "  image = %s,\n"
                           "  exposure = %f,\n"
                           "]",
                           indent(m_image), m_exposure);
    }
};

} // namespace lightwave

REGISTER_TEXTURE(ImageTexture, "image")
