#include <lightwave.hpp>

namespace lightwave {

class ToneMap final : public Postprocess {
private:
    float alpha1;
    float alpha2;
    float middle_grey;
    float phi;
    float max_scale;
    float threshold;
    bool local;
    
public:
    ToneMap(const Properties &properties) : Postprocess(properties) {
        alpha1 = 0.354;
        alpha2 = 1.6 * 0.354;
        middle_grey = 0.5;
        phi = 8;
        max_scale = 64;
        threshold = 0.05;    
        local = properties.get<bool>("local", false); 
    }

    float reinhard_local(Image img, Point2i pixel, float alpha, float s) {
        float ret = 0.f;
        float alpha_s_squared = (alpha * s) * (alpha * s);
        int kernel_size = (int)(((3.0 * alpha * s ) / std::sqrt(2.0)) + 0.5);
        for (int i = -kernel_size ; i < kernel_size ; i++) {
            for (int j = -kernel_size ; j < kernel_size ; j++) {
                Point2i rel_pix = pixel - Point2i(i, j);
                if (rel_pix[0] < 0 || rel_pix[0] >= img.resolution()[0] || rel_pix[1] < 0 || rel_pix[1] >= img.resolution()[1]) {
                    continue;
                }
                float r = std::exp(-(i * i + j * j) / alpha_s_squared) / (Pi * alpha_s_squared);
                ret += (r * img.get(rel_pix).luminance());
            }
        }
        return ret;
    }

    float reinhard_scale_helper(Image img, Point2i pixel, float s) {
        float v1 = reinhard_local(img, pixel, alpha1, s);
        float v2 = reinhard_local(img, pixel, alpha2, s);

        float denom = ((std::pow(2.0, phi) * middle_grey) / (s * s)) + v1;
        return (v1 - v2) / denom;
    }

    float get_scale(Image img, Point2i pixel) {
        for (float s = 1.f ; s < max_scale ; s *= 2.f) {
            float v = reinhard_scale_helper(img, pixel, s);
            if (std::abs(v) < threshold) {
                return s;
            }
        }
        return max_scale;
    }

    void execute() override {
        Point2i res = m_input -> resolution();
        m_output -> initialize(res);  
        Streaming stream { *m_output };
        if (local) {
            for_each_parallel(BlockSpiral((Vector2i)res, Vector2i(64)), [&](auto block) {
                for (auto a : block){
                    float scale = get_scale(*m_input.get(), a);
                    float denominator = 1.0 + reinhard_local(*m_input.get(), a, alpha1, scale);
                    m_output -> get(a) = (m_input -> get(a) / denominator);
                }
                stream.update();
            });
        }
        else {
            float max_lum = -1;
            for (auto pix : m_input -> bounds()) {
                float l = m_input -> get(pix).luminance();
                if (l > max_lum) {
                    max_lum = l;
                }
            }
            for_each_parallel(BlockSpiral((Vector2i)res, Vector2i(64)), [&](auto block) {
                for (auto pix : block) {
                        Color c = m_input -> get(pix);
                        float l = c.luminance();
                        float numerator = l * (1.0f + (l / (max_lum * max_lum)));
                        float l_new = numerator / (1.0f + l);
                        m_output -> get(pix) = c * (l_new / (l + Epsilon));
                }
                stream.update();
            });


        }
        m_output -> save();
    };

    std::string toString() const override {
        return tfm::format("ToneMap[\n"
                           "]");
    }
};

} // namespace lightwave

REGISTER_POSTPROCESS(ToneMap, "postprocess")
