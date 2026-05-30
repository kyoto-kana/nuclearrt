#pragma once

#include <unordered_map>
#include <string>
#include <memory>

struct ImageInfo {
    unsigned int Handle;

    int Width;
    int Height;
    short HotspotX;
    short HotspotY;
    short ActionPointX;
    short ActionPointY;
    short MosaicIndex;
    short MosaicX;
    short MosaicY;
    uint32_t TransparentColor;

    ImageInfo() : Handle(0), Width(0), Height(0), HotspotX(0), HotspotY(0), ActionPointX(0), ActionPointY(0), MosaicIndex(0), MosaicX(0), MosaicY(0), TransparentColor(0) {}
    
    ImageInfo(unsigned int handle, int width, int height, short hotspotX, short hotspotY, short actionPointX, short actionPointY, short mosaicIndex, short mosaicX, short mosaicY, uint32_t transparentColor)
        : Handle(handle), Width(width), Height(height), HotspotX(hotspotX), HotspotY(hotspotY), ActionPointX(actionPointX), ActionPointY(actionPointY), MosaicIndex(mosaicIndex), MosaicX(mosaicX), MosaicY(mosaicY), TransparentColor(transparentColor) {}
};

class ImageBank {
public:
    static ImageBank& Instance() {
        static ImageBank instance;
        return instance;
    }
    
    const ImageInfo* GetImage(unsigned int handle) const {
        auto it = Images.find(handle);
        return it != Images.end() ? &it->second : nullptr;
    }
    
private:
    ImageBank();
    
    std::unordered_map<unsigned int, ImageInfo> Images;
}; 