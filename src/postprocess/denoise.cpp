#include <lightwave.hpp>
#include <OpenImageDenoise/oidn.hpp>

namespace lightwave {

class Denoise final : public Postprocess {
private:
    ref<Image> normal_img;
    ref<Image> albedo_img;
public:
    Denoise(const Properties &properties) : Postprocess(properties) {
        normal_img = properties.get<Image>("normals");
        albedo_img = properties.get<Image>("albedo");
    }

    void execute() override {
        Point2i res = m_input->resolution();
        m_output->initialize(res);
        
        oidn::DeviceRef device = oidn::newDevice();
        device.commit();
        
        int width = res[1];
        int height = res[0];
        
        // Allocate buffers
        oidn::BufferRef colorBuf = device.newBuffer(width * height * 3 * sizeof(float));
        oidn::BufferRef normalBuf = device.newBuffer(width * height * 3 * sizeof(float));
        oidn::BufferRef albedoBuf = device.newBuffer(width * height * 3 * sizeof(float));
        oidn::BufferRef outputBuf = device.newBuffer(width * height * 3 * sizeof(float));
        
        // Copy input data to buffers
        memcpy(colorBuf.getData(), m_input->data(), width * height * 3 * sizeof(float));
        memcpy(normalBuf.getData(), normal_img->data(), width * height * 3 * sizeof(float));
        memcpy(albedoBuf.getData(), albedo_img->data(), width * height * 3 * sizeof(float));
        
        oidn::FilterRef filter = device.newFilter("RT");
        filter.setImage("color", colorBuf, oidn::Format::Float3, width, height);
        filter.setImage("normal", normalBuf, oidn::Format::Float3, width, height);
        filter.setImage("albedo", albedoBuf, oidn::Format::Float3, width, height);
        filter.setImage("output", outputBuf, oidn::Format::Float3, width, height);
        filter.set("hdr", true);
        filter.commit();
        
        // Execute filter
        filter.execute();
        
        // Copy denoised data back to output image
        const char* errorMessage;
        if (device.getError(errorMessage) != oidn::Error::None)
            std::cerr << "Error: " << errorMessage << std::endl;
        
        memcpy(m_output->data(), outputBuf.getData(), width * height * 3 * sizeof(float));
        
        // Save output image
        m_output->save();
        
    };

    std::string toString() const override {
        return tfm::format("Denoise[\n"
                           "]");
    }
};

} // namespace lightwave

REGISTER_POSTPROCESS(Denoise, "denoise")
