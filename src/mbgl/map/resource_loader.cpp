#include <mbgl/map/resource_loader.hpp>

#include <mbgl/map/environment.hpp>
#include <mbgl/map/source.hpp>
#include <mbgl/style/style.hpp>

#include <cassert>

namespace mbgl {

ResourceLoader::ResourceLoader() : observer_(nullptr) {
    assert(Environment::currentlyOn(ThreadType::Map));
}

ResourceLoader::~ResourceLoader() {
    assert(Environment::currentlyOn(ThreadType::Map));
}

void ResourceLoader::setObserver(Observer* observer) {
    assert(Environment::currentlyOn(ThreadType::Map));
    assert(!observer_);

    observer_ = observer;
}

void ResourceLoader::setStyle(util::ptr<Style> style) {
    style_ = style;

    Environment& env = Environment::Get();
    for (const auto& source : style->sources) {
        source->setObserver(this);
        source->load(accessToken_, env);
    }
}

void ResourceLoader::setAccessToken(const std::string& accessToken) {
    accessToken_ = accessToken;
}

void ResourceLoader::update(Map& map,
                           Worker& worker,
                           GlyphAtlas& glyphAtlas,
                           GlyphStore& glyphStore,
                           SpriteAtlas& spriteAtlas,
                           util::ptr<Sprite> sprite,
                           TexturePool& texturePool) {
    if (!style_) {
        return;
    }

    for (const auto& source : style_->sources) {
        source->update(
            map, worker, style_, glyphAtlas, glyphStore, spriteAtlas, sprite, texturePool);
    }
}

void ResourceLoader::onSourceLoaded() {
    emitTileDataChanged();
}

void ResourceLoader::onTileLoaded() {
    emitTileDataChanged();
}

void ResourceLoader::emitTileDataChanged() {
    assert(Environment::currentlyOn(ThreadType::Map));

    if (observer_) {
        observer_->onTileDataChanged();
    }
}

}