/**
  \file Image.cpp
  \author Corey Taylor
  Copyright 2002-2012, Morgan McGuire

  \created 2002-05-27
  \edited  2012-12-25
 */

#include "G3D/platform.h"
#include "FreeImagePlus.h"
#include "G3D/g3dmath.h"
#include "G3D/BinaryInput.h"
#include "G3D/Image.h"
#include "G3D/ImageFormat.h"
#include "G3D/FileSystem.h"
#include "G3D/Rect2D.h"
#include "G3D/ImageConvert.h"
#include "G3D/PixelTransferBuffer.h"
#include "G3D/CPUPixelTransferBuffer.h"

namespace G3D {

static const ImageFormat* determineImageFormat(const fipImage* image);
static FREE_IMAGE_TYPE determineFreeImageType(const ImageFormat* imageFormat);

Image::Image()
    : m_image(NULL)
    , m_format(ImageFormat::AUTO()) {

    // todo: if g3d ever has a global init, then this would move there to avoid deinitializing before program exit
    initFreeImage();
    m_image = new fipImage();
}

Image::~Image() {
    if (m_image) {
        delete m_image;
    }
    // This call can deinitialize the plugins if it's the last reference, but they can be re-initialized
    // disabled for now -- initialize FreeImage once in a thread-safe manner, then leave initialize
    //FreeImage_DeInitialise();
}


shared_ptr<Image> Image::create(int width, int height, const ImageFormat* imageFormat) {
    alwaysAssertM(notNull(imageFormat), "imageFormat may not be ImageFormat::AUTO() or NULL");

    shared_ptr<Image> img(new Image());
    img->setSize(width, height, imageFormat);
    return img;
}


GMutex Image::s_freeImageMutex;

void Image::initFreeImage() {
    GMutexLock lock(&s_freeImageMutex);
    static bool hasInitialized = false;

    if (! hasInitialized) {
        FreeImage_Initialise();
        hasInitialized = true;
    }
}


bool Image::fileSupported(const std::string& filename, bool allowCheckSignature) {
    initFreeImage();

    bool knownFormat = false;

    if (allowCheckSignature) {
        knownFormat = (FreeImage_GetFileType(filename.c_str(), 0) != FIF_UNKNOWN);
    }
    
    if (! knownFormat) {
        knownFormat = (FreeImage_GetFIFFromFilename(filename.c_str()) != FIF_UNKNOWN);
    }
    
    return knownFormat;
}


shared_ptr<Image> Image::fromFile(const std::string& filename, const ImageFormat* imageFormat) {
    debugAssertM(fileSupported(filename, true), G3D::format("Image file format not supported! (%s)", filename.c_str()));
    // Use BinaryInput to allow reading from zip files
    try {
        BinaryInput bi(filename, G3D::G3D_LITTLE_ENDIAN);
        return fromBinaryInput(bi, imageFormat);
    } catch (const std::string& e) {
        throw Error(e, filename);
    }
    return shared_ptr<Image>();
}


shared_ptr<Image> Image::fromBinaryInput(BinaryInput& bi, const ImageFormat* imageFormat) {
    Image* img = new Image;
    
    fipMemoryIO memoryIO(const_cast<uint8*>(bi.getCArray() + bi.getPosition()), static_cast<DWORD>(bi.getLength() - bi.getPosition()));

    if (! img->m_image->loadFromMemory(memoryIO)) {
        delete img;

        // todo: replace Image::Error handling with isNull() and notNull() checks in 
        throw Image::Error("Unsupported file format or unable to allocate FreeImage buffer", bi.getFilename());
        return shared_ptr<Image>();
    }

    const ImageFormat* detectedFormat = determineImageFormat(img->m_image);
    
    if (isNull(detectedFormat)) {
        delete img;
        img = NULL;

        // todo: replace Image::Error handling with isNull() and notNull() checks in 
        throw Image::Error("Loaded image pixel format does not map to any existing ImageFormat", bi.getFilename());
        return shared_ptr<Image>();
    }
    
    if (imageFormat == ImageFormat::AUTO()) {
        img->m_format = detectedFormat;
    } else {
        debugAssert(detectedFormat->canInterpretAs(imageFormat));
        if (! detectedFormat->canInterpretAs(imageFormat)) {
            delete img;

            throw Image::Error(G3D::format("Loaded image pixel format is not compatible with requested ImageFormat (%s)", imageFormat->name().c_str()), bi.getFilename());
            return shared_ptr<Image>();
        }
        img->m_format = imageFormat;
    }

    // Convert palettized images so row data can be copied easier
    if (img->m_image->getColorType() == FIC_PALETTE) {
        switch (img->m_image->getBitsPerPixel()) {
        case 1:
            img->convertToL8();
            break;
            
        case 8:
        case 24:
            img->convertToRGB8();
            break;
            
        case 32:
            img->convertToRGBA8();
            break;
            
        default:
            delete img;
            
            throw Image::Error("Loaded image data in unsupported palette format", bi.getFilename());
            return shared_ptr<Image>();
        }
    }
    
    return shared_ptr<Image>(img);
}


shared_ptr<Image> Image::fromPixelTransferBuffer(const PixelTransferBuffer::Ref& buffer) {
    shared_ptr<Image> img = create(buffer->width(), buffer->height(), buffer->format());
    img->set(buffer);
    return img;
}


void Image::convert(const ImageFormat* fmt) {
    PixelTransferBuffer::Ref result = ImageConvert::convertBuffer(toPixelTransferBuffer(), fmt);
    if (notNull(result)) {
        set(result);
    } else {
        throw Image::Error(G3D::format("Could not convert from ImageFormat %s to %s",
                                       m_format->name().c_str(), fmt->name().c_str()));
    }
}


void Image::setSize(int w, int h, const ImageFormat* fmt) {
    if ((width() != w) ||
        (height() != h) ||
        (format() != fmt)) {

        debugAssert(notNull(m_image));
        
        if (fmt != ImageFormat::AUTO()) {
            m_format = fmt;
        }

        debugAssert(notNull(m_format));
        
        const FREE_IMAGE_TYPE fiType = determineFreeImageType(fmt);
        debugAssertM(fiType != FIT_UNKNOWN, 
                     G3D::format("Trying to create Image from unsupported ImageFormat (%s)", fmt->name().c_str()));
        

        if (! m_image->setSize(fiType, w, h, fmt->cpuBitsPerPixel)) {
            throw Image::Error(G3D::format("Unable to allocate FreeImage buffer from ImageFormat (%s)",
                                           fmt->name().c_str()));
        }
    }
}


void Image::set(const shared_ptr<PixelTransferBuffer>& buffer) {
    setSize(buffer->width(), buffer->height(), buffer->format());

    set(buffer, 0, 0);
}


void Image::set(const shared_ptr<PixelTransferBuffer>& buffer, int x, int y) {
    debugAssert(x >= 0 && x < width());
    debugAssert(y >= 0 && y < height());

    // Cannot copy between incompatible formats
    if (! m_format->canInterpretAs(buffer->format())) {
        return;
    }

    BYTE* pixels(m_image->accessPixels());
    debugAssert(pixels);

    if (pixels) {
        // create rect representing area we want to set and clip to image bounds
        const Rect2D& rect = Rect2D::xywh((float)x, (float)y, (float)buffer->width(), (float)buffer->height()).intersect(bounds());

        if (! rect.isEmpty()) {
            const size_t bytesPerPixel = iCeil(buffer->format()->cpuBitsPerPixel / 8.0f);
            const size_t rowStride     = size_t(rect.width() * bytesPerPixel);
            const size_t offsetStride  = size_t(rect.x0()    * bytesPerPixel);

            const uint8* src = static_cast<const uint8*>(buffer->mapRead());
            debugAssert(notNull(src));

            for (int row = 0; row < rect.height(); ++row) {
                System::memcpy(m_image->getScanLine((int)rect.height() - (row + (int)rect.y0()) - 1) + offsetStride, src + rowStride * row, rowStride);
            }
            buffer->unmap();
        }
    }
}


void Image::save(const std::string& filename) const {
    if (! m_image->save(filename.c_str())) {
        debugAssertM(false, G3D::format("Failed to write image to %s", filename.c_str()));
    }
    // Since we are bypassing G3D::FileSystem, our cache does not get updated
    // without this line, we get errors such as multiple screenshots in quick 
    // succession saving to the same file
    FileSystem::clearCache(FilePath::parent(filename));
}


// Helper for FreeImageIO to allow seeking
struct _FIBinaryOutputInfo {
    BinaryOutput* bo;
    int64         startPos;
};

// FreeImageIO implementation for writing to BinaryOutput
static unsigned _FIBinaryOutputWrite(void* buffer, unsigned size, unsigned count, fi_handle handle) {
    _FIBinaryOutputInfo* info = static_cast<_FIBinaryOutputInfo*>(handle);
    
    // Write 'size' number of bytes from 'buffer' for 'count' times
    info->bo->writeBytes(buffer, size * count);

    return count;
}

// FreeImageIO implementation for writing to BinaryOutput
static int _FIBinaryOutputSeek(fi_handle handle, long offset, int origin) {
    _FIBinaryOutputInfo* info = static_cast<_FIBinaryOutputInfo*>(handle);

    switch (origin)
    {
        case SEEK_SET:
        {
            info->bo->setPosition(info->startPos + offset);
            break;
        }

        case SEEK_END:
        {
            int64 oldLength = info->bo->length();
            if (offset > 0) {
                info->bo->setLength(oldLength + offset);
            }

            info->bo->setPosition(oldLength + offset);
            break;
        }

        case SEEK_CUR:
        {
            info->bo->setPosition(info->bo->position() + offset);
            break;
        }

        default:
            return -1;
            break;
    }

    return 0;
}

// FreeImageIO implementation for writing to BinaryOutput
static long _FIBinaryOutputTell(fi_handle handle) {
    _FIBinaryOutputInfo* info = static_cast<_FIBinaryOutputInfo*>(handle);

    return static_cast<long>(info->bo->position() - info->startPos);
}

void Image::serialize(BinaryOutput& bo, ImageFileFormat fileFormat) const {
    FreeImageIO fiIO;
    fiIO.read_proc = NULL;
    fiIO.seek_proc = _FIBinaryOutputSeek;
    fiIO.tell_proc = _FIBinaryOutputTell;
    fiIO.write_proc = _FIBinaryOutputWrite;

    _FIBinaryOutputInfo info;
    info.bo = &bo;
    info.startPos = bo.position();

    if (! m_image->saveToHandle(static_cast<FREE_IMAGE_FORMAT>(fileFormat), &fiIO, &info)) {
        debugAssertM(false, G3D::format("Failed to write image to BinaryOutput in '%s' format", FreeImage_GetFormatFromFIF(static_cast<FREE_IMAGE_FORMAT>(fileFormat))));
    }
}


shared_ptr<CPUPixelTransferBuffer> Image::toPixelTransferBuffer() const {
    return toPixelTransferBuffer(Rect2D::xywh(0, 0, (float)width(), (float)height()));
}


shared_ptr<CPUPixelTransferBuffer> Image::toPixelTransferBuffer(Rect2D rect) const {
    // clip to bounds of image
    rect = rect.intersect(bounds());

    if (rect.isEmpty()) {
        return shared_ptr<CPUPixelTransferBuffer>();
    }

    shared_ptr<CPUPixelTransferBuffer> buffer = CPUPixelTransferBuffer::create((int)rect.width(), (int)rect.height(), m_format, AlignedMemoryManager::create(), 1, 1);

    BYTE* pixels = m_image->accessPixels();
    if (pixels) {
        const size_t bytesPerPixel = iCeil(buffer->format()->cpuBitsPerPixel / 8.0f);
        const size_t rowStride     = int(rect.width())  * bytesPerPixel;
        const size_t offsetStride  = int(rect.x0())     * bytesPerPixel;

        debugAssert(isFinite(rect.width()) && isFinite(rect.height()));

        for (int row = 0; row < int(rect.height()); ++row) {
            // Note that we flip while copying
            System::memcpy(buffer->row(row), m_image->getScanLine(int(rect.height()) - 1 - (row + int(rect.y0()))) + offsetStride, rowStride);
        }
    }

    return buffer;
}


shared_ptr<Image> Image::clone() const {
    Image* c = new Image;

    *(c->m_image) = *m_image;
    c->m_format = m_format;

    return shared_ptr<Image>(c);
}


const ImageFormat* Image::format() const {
    return m_format;
}


int Image::width() const {
    return m_image->getWidth();
}


int Image::height() const {
    return m_image->getHeight();
}


void Image::flipVertical() {
    m_image->flipVertical();
}


void Image::flipHorizontal() {
    m_image->flipHorizontal();
}


void Image::rotateCW(double radians) {
    m_image->rotate(toDegrees(radians));
}


void Image::rotateCCW(double radians) {
    rotateCW(-radians);
}


bool Image::convertToL8() {
    if (m_image->convertToGrayscale()) {
        m_format = ImageFormat::L8();
        return true;
    }
    return false;
}


bool Image::convertToRGB8() {
    if (m_image->convertTo24Bits()) {
        m_format = ImageFormat::RGB8();
        return true;
    }
    return false;
}


bool Image::convertToRGBA8() {
    if (m_image->convertTo32Bits()) {
        m_format = ImageFormat::RGBA8();
        return true;
    }
    return false;
}


void Image::get(const Point2int32& pos, Color4& color) const {
    Point2int32 fipPos(pos.x, m_image->getHeight() - pos.y - 1);

    BYTE* scanline = m_image->getScanLine(fipPos.y);
    switch (m_image->getImageType()) {
        case FIT_BITMAP:
        {
            if (m_image->isGrayscale()) {
                color.r = (scanline[fipPos.x] / 255.0f);
                color.g = color.r;
                color.b = color.r;
                color.a = 1.0f;
            } else if (m_image->getBitsPerPixel() == 24) {
                scanline += 3 * fipPos.x;

                color.r = (scanline[FI_RGBA_RED] / 255.0f);
                color.g = (scanline[FI_RGBA_GREEN] / 255.0f);
                color.b = (scanline[FI_RGBA_BLUE] / 255.0f);
                color.a = 1.0f;
            } else if (m_image->getBitsPerPixel() == 32) {
                scanline += 4 * fipPos.x;

                color.r = (scanline[FI_RGBA_RED] / 255.0f);
                color.g = (scanline[FI_RGBA_GREEN] / 255.0f);
                color.b = (scanline[FI_RGBA_BLUE] / 255.0f);
                color.a = (scanline[FI_RGBA_ALPHA] / 255.0f);
            }
            break;
        }

        case FIT_RGBF:
        {
            const Color3* row(reinterpret_cast<Color3*>(scanline));
            const Color3& pixel(row[fipPos.x]);
            color.r = pixel.r;
            color.g = pixel.g;
            color.b = pixel.b;
            color.a = 1.0f;
            break;
        }

        case FIT_RGBAF:
        {
            const Color4* row(reinterpret_cast<Color4*>(scanline));
            color = row[fipPos.x];
            break;
        }

        case FIT_FLOAT:
        {
            const Color1* row(reinterpret_cast<Color1*>(scanline));
            const Color1& pixel(row[fipPos.x]);
            color.r = pixel.value;
            color.g = pixel.value;
            color.b = pixel.value;
            color.a = 1.0f;
            break;
        }

        default:
            debugAssertM(false, G3D::format("Image::get does not support pixel format (%s)", m_format->name().c_str()));
            break;
    }
}


void Image::get(const Point2int32& pos, Color3& color) const {
    Color4 c;
    get(pos, c);
    color = Color3(c.r, c.g, c.b);
}


void Image::get(const Point2int32& pos, Color4unorm8& color) const {
    Point2int32 fipPos(pos.x, m_image->getHeight() - pos.y - 1);

    BYTE* scanline = m_image->getScanLine(fipPos.y);
    switch (m_image->getImageType()) {
        case FIT_BITMAP:
        {
            if (m_image->isGrayscale()) {
                color.r = unorm8::fromBits(scanline[fipPos.x]);
                color.g = color.r;
                color.b = color.r;
                color.a = unorm8::one();
            } else if (m_image->getBitsPerPixel() == 24) {
                scanline += 3 * fipPos.x;

                color.r = unorm8::fromBits(scanline[FI_RGBA_RED]);
                color.g = unorm8::fromBits(scanline[FI_RGBA_GREEN]);
                color.b = unorm8::fromBits(scanline[FI_RGBA_BLUE]);
                color.a = unorm8::one();
            } else if (m_image->getBitsPerPixel() == 32) {
                scanline += 4 * fipPos.x;

                color.r = unorm8::fromBits(scanline[FI_RGBA_RED]);
                color.g = unorm8::fromBits(scanline[FI_RGBA_GREEN]);
                color.b = unorm8::fromBits(scanline[FI_RGBA_BLUE]);
                color.a = unorm8::fromBits(scanline[FI_RGBA_ALPHA]);
            }
            break;
        }


        case FIT_RGBF:
        {
            const Color3* row(reinterpret_cast<Color3*>(scanline));
            const Color3& pixel(row[fipPos.x]);
            color.r = unorm8(pixel.r);
            color.g = unorm8(pixel.g);
            color.b = unorm8(pixel.b);
            color.a = unorm8::one();
            break;
        }

        case FIT_RGBAF:
        {
            const Color4* row(reinterpret_cast<Color4*>(scanline));
            color = Color4unorm8(row[fipPos.x]);
            break;
        }

        case FIT_FLOAT:
        {
            const Color1* row(reinterpret_cast<Color1*>(scanline));
            const Color1& pixel(row[fipPos.x]);
            color.r = unorm8(pixel.value);
            color.g = color.r;
            color.b = color.r;
            color.a = unorm8::one();
            break;
        }

        default:
            debugAssertM(false, G3D::format("Image::get does not support pixel format (%s)", m_format->name().c_str()));
            break;
    }
}


void Image::get(const Point2int32& pos, Color3unorm8& color) const {
    Color4unorm8 c;
    get(pos, c);
    color = c.rgb();
}


void Image::get(const Point2int32& pos, Color1unorm8& color) const {
    Color4unorm8 c;
    get(pos, c);
    color.value = c.r;
}


void Image::set(const Point2int32& pos, const Color4& color) {
    Point2int32 fipPos(pos.x, m_image->getHeight() - pos.y - 1);

    BYTE* scanline = m_image->getScanLine(fipPos.y);
    switch (m_image->getImageType()) {
        case FIT_BITMAP:
        {
            if (m_image->isGrayscale()) {
                scanline[fipPos.x] = static_cast<BYTE>(iClamp(static_cast<int>(color.r * 255.0f), 0, 255));
            } else if (m_image->getBitsPerPixel() == 24) {
                scanline += 3 * fipPos.x;
                
                scanline[FI_RGBA_RED] = static_cast<BYTE>(iClamp(static_cast<int>(color.r * 255.0f), 0, 255));
                scanline[FI_RGBA_GREEN] = static_cast<BYTE>(iClamp(static_cast<int>(color.g * 255.0f), 0, 255));
                scanline[FI_RGBA_BLUE] = static_cast<BYTE>(iClamp(static_cast<int>(color.b * 255.0f), 0, 255));
            } else if (m_image->getBitsPerPixel() == 32) {
                scanline += 4 * fipPos.x;
                
                scanline[FI_RGBA_RED] = static_cast<BYTE>(iClamp(static_cast<int>(color.r * 255.0f), 0, 255));
                scanline[FI_RGBA_GREEN] = static_cast<BYTE>(iClamp(static_cast<int>(color.g * 255.0f), 0, 255));
                scanline[FI_RGBA_BLUE] = static_cast<BYTE>(iClamp(static_cast<int>(color.b * 255.0f), 0, 255));
                scanline[FI_RGBA_ALPHA] = static_cast<BYTE>(iClamp(static_cast<int>(color.a * 255.0f), 0, 255));
            }
            break;
        }

        case FIT_RGBF:
        {
            Color3* row(reinterpret_cast<Color3*>(scanline));
            Color3& pixel(row[fipPos.x]);
            pixel.r = color.r;
            pixel.g = color.g;
            pixel.b = color.b;
            break;
        }

        case FIT_RGBAF:
        {
            Color4* row(reinterpret_cast<Color4*>(scanline));
            row[fipPos.x] = color;
            break;
        }

        case FIT_FLOAT:
        {
            Color1* row(reinterpret_cast<Color1*>(scanline));
            row[fipPos.x].value = color.r;
            break;
        }

        default:
            debugAssertM(false, G3D::format("Image::set does not support pixel format (%s)", m_format->name().c_str()));
            break;
    }
}


void Image::set(const Point2int32& pos, const Color3& color) {
    set(pos, Color4(color));
}


void Image::set(const Point2int32& pos, const Color4unorm8& color) {
    Point2int32 fipPos(pos.x, m_image->getHeight() - pos.y - 1);

    BYTE* scanline = m_image->getScanLine(fipPos.y);
    switch (m_image->getImageType()) {
        case FIT_BITMAP:
        {
            if (m_image->isGrayscale()) {
                scanline[fipPos.x] = color.r.bits();
            } else if (m_image->getBitsPerPixel() == 24) {
                scanline += 3 * fipPos.x;

                scanline[FI_RGBA_RED]   = color.r.bits();
                scanline[FI_RGBA_GREEN] = color.g.bits();
                scanline[FI_RGBA_BLUE]  = color.b.bits();
            } else if (m_image->getBitsPerPixel() == 32) {
                scanline += 4 * fipPos.x;

                scanline[FI_RGBA_RED]   = color.r.bits();
                scanline[FI_RGBA_GREEN] = color.g.bits();
                scanline[FI_RGBA_BLUE]  = color.b.bits();
                scanline[FI_RGBA_ALPHA] = color.a.bits();
            }
            break;
        }

        case FIT_RGBF:
        {
            Color3* row(reinterpret_cast<Color3*>(scanline));
            Color3& pixel(row[fipPos.x]);
            pixel.r = color.r;
            pixel.g = color.g;
            pixel.b = color.b;
            break;
        }

        case FIT_RGBAF:
        {
            Color4* row(reinterpret_cast<Color4*>(scanline));
            row[fipPos.x] = color;
            break;
        }

        case FIT_FLOAT:
        {
            Color1* row(reinterpret_cast<Color1*>(scanline));
            row[fipPos.x].value = color.r;
            break;
        }

        default:
            debugAssertM(false, G3D::format("Image::set does not support pixel format (%s)", m_format->name().c_str()));
            break;
    }
}


void Image::set(const Point2int32& pos, const Color3unorm8& color) {
    set(pos, Color4unorm8(color, unorm8::one()));
}


void Image::set(const Point2int32& pos, const Color1unorm8& color) {
    set(pos, Color4unorm8(color.value, color.value, color.value, unorm8::one()));
}


Color4 Image::nearest(const Vector2& pos, WrapMode w) const {
    return get<Color4>(iFloor(pos.x), iFloor(pos.y), w);
}


Color4  Image::nearest(int x, int y, WrapMode w) const {
    return nearest(Point2((float)x, (float)y), w);
}


Color4  Image::bilinear(float x, float y, WrapMode wrap) const {
    typedef Color4 Compute;
    const int i = iFloor(x);
    const int j = iFloor(y);
    
    const float fX = x - i;
    const float fY = y - j;

    // Horizontal interpolation, first row
    const Color4& t0 = get<Color4>(i, j, wrap);
    const Color4& t1 = get<Color4>(i + 1, j, wrap);

    // Horizontal interpolation, second row
    const Color4& t2 = get<Color4>(i, j + 1, wrap);
    const Color4& t3 = get<Color4>(i + 1, j + 1, wrap);

    const Color4& A = t0.lerp(t1, fX);
    const Color4& B = t2.lerp(t3, fX);

    // Vertical interpolation
    return A.lerp(B, fY);
}


Color4 Image::bilinear(const Vector2& pos, WrapMode wrap) const {
    return bilinear(pos.x, pos.y, wrap);
}


/// Static Helpers

static const ImageFormat* determineImageFormat(const fipImage* image) {
    debugAssert(image->isValid() && image->getImageType() != FIT_UNKNOWN);
    
    const ImageFormat* imageFormat = NULL;
    switch (image->getImageType())
    {
        case FIT_BITMAP:
        {
            switch (image->getBitsPerPixel())
            {
                case 8:
                    imageFormat = ImageFormat::L8();
                    break;

                case 16:
                    // todo: find matching image format
                    debugAssertM(false, "Unsupported bit depth loaded.");
                    break;

                case 24:
                    imageFormat = ImageFormat::RGB8();
                    break;

                case 32:
                    imageFormat = ImageFormat::RGBA8();
                    break;

                default:
                    debugAssertM(false, "Unsupported bit depth loaded.");
                    break;
            }
            break;
        }

        case FIT_UINT16:
            imageFormat = ImageFormat::L16();
            break;

        case FIT_FLOAT:
            imageFormat = ImageFormat::L32F();
            break;

        case FIT_RGBF:
            imageFormat = ImageFormat::RGB32F();
            break;

        case FIT_RGBAF:
            imageFormat = ImageFormat::RGBA32F();
            break;

        case FIT_INT16:
        case FIT_UINT32:
        case FIT_INT32:
        case FIT_DOUBLE:
        case FIT_RGB16:
        case FIT_RGBA16:
        case FIT_COMPLEX:
        default:
            debugAssertM(false, "Unsupported FreeImage type loaded.");
            break;
    }

    if (image->getColorType() == FIC_CMYK) {
        debugAssertM(false, "Unsupported FreeImage color space (CMYK) loaded.");
        imageFormat = NULL;
    }

    return imageFormat;
}


static FREE_IMAGE_TYPE determineFreeImageType(const ImageFormat* imageFormat) {
    FREE_IMAGE_TYPE fiType = FIT_UNKNOWN;
    if (imageFormat == NULL) {
        return fiType;
    }

    switch (imageFormat->code) {
    case ImageFormat::CODE_L8:
    case ImageFormat::CODE_R8:
    case ImageFormat::CODE_RGB8:
    case ImageFormat::CODE_RGBA8:
        fiType = FIT_BITMAP;
        break;
        
    case ImageFormat::CODE_L16:
    case ImageFormat::CODE_A16:
        fiType = FIT_UINT16;
        break;

    case ImageFormat::CODE_L32F:
    case ImageFormat::CODE_A32F:
    case ImageFormat::CODE_R32F:
        fiType = FIT_FLOAT;
        break;
        
    case ImageFormat::CODE_RGB32F:
        fiType = FIT_RGBF;
        break;
        
    case ImageFormat::CODE_RGBA32F:
        fiType = FIT_RGBAF;
        break;
        
    default:
        break;
    }

    return fiType;
}


} // namespace G3D
