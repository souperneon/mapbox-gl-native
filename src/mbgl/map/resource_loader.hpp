#ifndef MBGL_MAP_RESOURCE_LOADER
#define MBGL_MAP_RESOURCE_LOADER

#include <mbgl/map/source.hpp>
#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/ptr.hpp>

#include <string>

namespace mbgl {

class Style;

// ResourceLoader is responsible for loading and updating the Source(s) owned
// by the Style. The Source object currently owns all the tiles, thus this
// class will notify its observers of any change on these tiles which will
// ultimately cause a new rendering to be triggered.
class ResourceLoader : public Source::Observer, private util::noncopyable {
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

    // Source::Observer implementation.
    void onSourceLoaded();
    void onTileLoaded();

private:
    void emitTileDataChanged();

    std::string accessToken_;
    util::ptr<Style> style_;
    Observer* observer_;
};

}

#endif
