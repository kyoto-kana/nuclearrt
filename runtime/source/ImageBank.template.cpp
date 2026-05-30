#include "ImageBank.h"

ImageBank::ImageBank() {
    struct RawData {
        unsigned int handle; int w; int h; short hx; short hy; short ax; short ay; short mi; short mx; short my; uint32_t tc;
    };

    static const RawData RAW_IMAGE_BANK[] = {
{{ IMAGES }}
    };

    static const size_t IMAGE_COUNT = sizeof(RAW_IMAGE_BANK) / sizeof(RawData);

    if (IMAGE_COUNT > 0) {
        Images.reserve(IMAGE_COUNT);
        for (size_t i = 0; i < IMAGE_COUNT; ++i) {
            const auto& d = RAW_IMAGE_BANK[i];
            Images.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(d.handle),
                std::forward_as_tuple(d.handle, d.w, d.h, d.hx, d.hy, d.ax, d.ay, d.mi, d.mx, d.my, d.tc)
            );
        }
    }
}
