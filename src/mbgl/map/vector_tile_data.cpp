#include <mbgl/map/vector_tile_data.hpp>
#include <mbgl/map/tile_parser.hpp>
#include <mbgl/util/std.hpp>
#include <mbgl/style/style_layer.hpp>
#include <mbgl/style/style_bucket.hpp>
#include <mbgl/map/source.hpp>
#include <mbgl/geometry/glyph_atlas.hpp>
#include <mbgl/platform/log.hpp>
#include <mbgl/util/pbf.hpp>

using namespace mbgl;

VectorTileData::VectorTileData(const TileID& id_,
                               float mapMaxZoom,
                               util::ptr<Style> style_,
                               GlyphAtlas& glyphAtlas_,
                               GlyphStore& glyphStore_,
                               SpriteAtlas& spriteAtlas_,
                               util::ptr<Sprite> sprite_,
                               const SourceInfo& source_)
    : TileData(id_, source_),
      glyphAtlas(glyphAtlas_),
      glyphStore(glyphStore_),
      spriteAtlas(spriteAtlas_),
      sprite(sprite_),
      style(style_),
      depth(id.z >= source.max_zoom ? mapMaxZoom - id.z : 1) {
}

VectorTileData::~VectorTileData() {
    glyphAtlas.removeGlyphs(reinterpret_cast<uintptr_t>(this));
}

void VectorTileData::parse() {
    if (state != State::loaded && state != State::partial) {
        return;
    }

    try {
        if (!style) {
            throw std::runtime_error("style isn't present in VectorTileData object anymore");
        }

        // Parsing creates state that is encapsulated in TileParser. While parsing,
        // the TileParser object writes results into this objects. All other state
        // is going to be discarded afterwards.
        VectorTile vectorTile(pbf((const uint8_t *)data.data(), data.size()));
        const VectorTile* vt = &vectorTile;
        TileParser parser(*vt, *this, style, glyphAtlas, glyphStore, spriteAtlas, sprite);

        parser.parse();

        if (state == State::obsolete) {
            style.reset();
            return;
        }

        if (parser.isPartialParse()) {
            // If the tile was only partially parsed, we keep a reference
            // to the style until we finally parse it completely.
            state = State::partial;
        } else {
            style.reset();
            state = State::parsed;
        }
    } catch (const std::exception& ex) {
        Log::Error(Event::ParseTile, "Parsing [%d/%d/%d] failed: %s", id.z, id.x, id.y, ex.what());
        state = State::obsolete;
        return;
    }
}

void VectorTileData::render(Painter &painter, const StyleLayer &layer_desc, const mat4 &matrix) {
    if (!ready() || !layer_desc.bucket) {
        return;
    }

    Bucket* bucket = getBucket(layer_desc.bucket->name);
    if (!bucket) {
        return;
    }

    bucket->render(painter, layer_desc, id, matrix);
}

bool VectorTileData::hasData(const StyleLayer &layer_desc) const {
    if (!ready() || !layer_desc.bucket) {
        return false;
    }

    Bucket* bucket = getBucket(layer_desc.bucket->name);
    if (!bucket) {
        return false;
    }

    return bucket->hasData();
}

Bucket* VectorTileData::getBucket(const std::string& key) const {
    std::lock_guard<std::mutex> lock(bucketsMutex);

    auto it = buckets.find(key);
    if (it == buckets.end())
        return nullptr;

    assert(it->second);
    return it->second.get();
}

void VectorTileData::setBucket(const std::string& key, std::unique_ptr<Bucket> bucket) {
    std::lock_guard<std::mutex> lock(bucketsMutex);

    buckets[key] = std::move(bucket);
}
