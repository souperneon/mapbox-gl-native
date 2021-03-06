#ifndef MBGL_GEOMETRY_GLYPH_ATLAS
#define MBGL_GEOMETRY_GLYPH_ATLAS

#include <mbgl/geometry/binpack.hpp>
#include <mbgl/text/glyph_store.hpp>
#include <mbgl/util/noncopyable.hpp>

#include <string>
#include <set>
#include <map>
#include <mutex>
#include <atomic>

namespace mbgl {

class GlyphAtlas : public util::noncopyable {
public:
    GlyphAtlas(uint16_t width, uint16_t height);

    void addGlyphs(uintptr_t tileUID,
                   const std::u32string& text,
                   const std::string& stackName,
                   const FontStack&,
                   GlyphPositions&);
    void removeGlyphs(uintptr_t tileUID);

    void bind();

    const uint16_t width = 0;
    const uint16_t height = 0;

private:
    struct GlyphValue {
        GlyphValue(const Rect<uint16_t>& rect_, uintptr_t id)
            : rect(rect_), ids({ id }) {}
        Rect<uint16_t> rect;
        std::set<uintptr_t> ids;
    };

    Rect<uint16_t> addGlyph(uintptr_t tileID,
                            const std::string& stackName,
                            const SDFGlyph&);

    std::mutex mtx;
    BinPack<uint16_t> bin;
    std::map<std::string, std::map<uint32_t, GlyphValue>> index;
    std::unique_ptr<char[]> data;
    std::atomic<bool> dirty;
    uint32_t texture = 0;
};

};

#endif
