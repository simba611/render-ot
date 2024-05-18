#include <lightwave.hpp>

#include <functional>
#include "pcg32.h"

namespace lightwave {

/**
 * @brief Generates random numbers uniformely distributed in [0,1), which are all stochastically independent from another.
 * This is the simplest form of random number generation, and will be sufficient for our introduction to Computer Graphics.
 * If you want to reduce the noise in your renders, a simple way is to implement more sophisticated random numbers (e.g.,
 * jittered sampling or blue noise sampling).
 * @see Internally, this sampler uses the PCG32 library to generate random numbers.
 */
class Halton : public Sampler {
    uint64_t m_seed;
    pcg32 m_pcg;
    int base_prime;
    int sample_index;
    float offset;

public:
    Halton(const Properties &properties)
    : Sampler(properties) {
        m_seed = properties.get<int>("seed", 1337);
    }
    double reverseDigits(int n, int base) {
        double reversed = 0;
        double invBase = 1.0 / base;

        while (n > 0) {
            reversed += (n % base) * invBase;
            n /= base;
            invBase /= base;
        }

        return reversed;
    }

    void seed(int sampleIndex) override {
        base_prime = 2;
        sample_index = sampleIndex;
        m_pcg.seed(m_seed, sampleIndex);
    }

    void seed(const Point2i &pixel, int sampleIndex) override {
        base_prime = 2;
        sample_index = sampleIndex;
        const uint64_t a = (uint64_t(pixel.x()) << 32) ^ pixel.y();
        m_pcg.seed(m_seed, a);
        offset = m_pcg.nextFloat();
        // m_pcg.seed(m_pcg.nextUInt(), sampleIndex);
    }

    float next() override {
        float ret = reverseDigits(sample_index, base_prime);
        base_prime++;
        ret += offset;
        while (ret >= 1) {
            ret -= 1;
        }
        return ret;
        // return m_pcg.nextFloat();
    }

    ref<Sampler> clone() const override {
        return std::make_shared<Halton>(*this);
    }

    std::string toString() const override {
        return tfm::format(
            "Halton[\n"
            "  count = %d\n"
            "]",
            m_samplesPerPixel
        );
    }
};

}

REGISTER_SAMPLER(Halton, "halton")
