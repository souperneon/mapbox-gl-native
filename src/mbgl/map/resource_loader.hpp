#ifndef MBGL_MAP_RESOURCE_LOADER
#define MBGL_MAP_RESOURCE_LOADER

#include <mbgl/map/source.hpp>
#include <mbgl/map/sprite.hpp>
#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/ptr.hpp>

#include <string>

namespace mbgl {

class GlyphAtlas;
class GlyphStore;
class Map;
class SpriteAtlas;
class Style;
class TexturePool;
class Worker;

// ResourceLoader is responsible for loading and updating the Source(s) owned
// by the Style. The Source object currently owns all the tiles, thus this
// class will notify its observers of any change on these tiles which will
// ultimately cause a new rendering to be triggered.
class ResourceLoader : public Source::Observer, public Sprite::Observer, private util::noncopyable {
public:
    class Observer {
    public:
        virtual ~Observer() = default;

        virtual void onTileDataChanged() = 0;
    };

    ResourceLoader();
    ~ResourceLoader();

    void setObserver(Observer* observer);

    // The style object currently owns all the sources. When setting
    // a new style we will go through all of them and try to load.
    void setStyle(util::ptr<Style> style);

    // Set the access token to be used for loading the tile data.
    void setAccessToken(const std::string& accessToken);

    // Fetch the tiles needed by the current viewport and emit a signal when
    // a tile is ready so observers can render the tile.
    void update(Map&, GlyphAtlas&, GlyphStore&, SpriteAtlas&);

    // FIXME: There is probably a better place for this.
    inline util::ptr<Sprite> getSprite() const {
        return sprite_;
    }

    // Source::Observer implementation.
    void onSourceLoaded();
    void onTileLoaded();

    // Sprite::Observer implementation.
    void onSpriteLoaded();

private:
    void emitTileDataChanged();

    std::unique_ptr<Worker> worker_;
    std::unique_ptr<TexturePool> texturePool_;
    std::string accessToken_;
    util::ptr<Style> style_;
    util::ptr<Sprite> sprite_;
    Observer* observer_;
};

}

#endif
