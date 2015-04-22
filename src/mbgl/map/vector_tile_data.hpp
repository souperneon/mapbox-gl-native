#ifndef MBGL_MAP_VECTOR_TILE_DATA
#define MBGL_MAP_VECTOR_TILE_DATA

#include <mbgl/map/tile_data.hpp>
#include <mbgl/geometry/elements_buffer.hpp>
#include <mbgl/geometry/fill_buffer.hpp>
#include <mbgl/geometry/icon_buffer.hpp>
#include <mbgl/geometry/line_buffer.hpp>
#include <mbgl/geometry/text_buffer.hpp>

#include <iosfwd>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace mbgl {

class Bucket;
class Painter;
class SourceInfo;
class StyleLayer;
class TileParser;
class GlyphAtlas;
class GlyphStore;
class SpriteAtlas;
class Sprite;
class Style;

class VectorTileData : public TileData {
    friend class TileParser;

public:
    VectorTileData(const TileID&,
                   float mapMaxZoom,
                   util::ptr<Style>,
                   GlyphAtlas&,
                   GlyphStore&,
                   SpriteAtlas&,
                   util::ptr<Sprite>,
                   const SourceInfo&);
    ~VectorTileData();

    void parse() override;
    void render(Painter &painter, const StyleLayer &layer_desc, const mat4 &matrix) override;
    bool hasData(StyleLayer const& layer_desc) const override;

    Bucket* getBucket(const std::string& key) const;
    void setBucket(const std::string& key, std::unique_ptr<Bucket> bucket);

protected:
    // Holds the actual geometries in this tile.
    FillVertexBuffer fillVertexBuffer;
    LineVertexBuffer lineVertexBuffer;

    TriangleElementsBuffer triangleElementsBuffer;
    LineElementsBuffer lineElementsBuffer;
    PointElementsBuffer pointElementsBuffer;

    GlyphAtlas& glyphAtlas;
    GlyphStore& glyphStore;
    SpriteAtlas& spriteAtlas;
    util::ptr<Sprite> sprite;
    util::ptr<Style> style;

private:
    // Contains all the Bucket objects for the tile. Bucket are render
    // objects and they get added to this std::map<> by the workers doing
    // the actual tile parsing as they get processed. Tiles partially
    // parsed can get new buckets at any moment but are also fit for
    // rendering. That said, access to this list needs locking unless
    // the tile is completely parsed.
    std::unordered_map<std::string, std::unique_ptr<Bucket>> buckets;
    mutable std::mutex bucketsMutex;

public:
    const float depth;
};

}

#endif
