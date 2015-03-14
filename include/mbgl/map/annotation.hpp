#ifndef MBGL_MAP_ANNOTATIONS
#define MBGL_MAP_ANNOTATIONS

#include <mbgl/map/map.hpp>
#include <mbgl/map/tile.hpp>
#include <mbgl/map/live_tile.hpp>
#include <mbgl/util/geo.hpp>
#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/std.hpp>

#include <string>
#include <vector>
#include <map>

namespace mbgl {

class Annotation;

enum class AnnotationType : uint8_t {
    Point,
    Shape
};

class AnnotationManager : private util::noncopyable {
public:
    void setDefaultPointAnnotationSymbol(const std::string& symbol) { defaultPointAnnotationSymbol = symbol; }
    uint32_t addPointAnnotation(LatLng, const std::string& symbol = "");
    std::vector<uint32_t> addPointAnnotations(std::vector<LatLng>, std::vector<const std::string>& symbols);
    uint32_t addShapeAnnotation(std::vector<AnnotationSegment>);
    std::vector<uint32_t> addShapeAnnotations(std::vector<std::vector<AnnotationSegment>>);
    void removeAnnotation(uint32_t);
    void removeAnnotations(std::vector<uint32_t>);
    std::vector<uint32_t> getAnnotationsInBoundingBox(BoundingBox) const;
    BoundingBox getBoundingBoxForAnnotations(std::vector<uint32_t>) const;

private:
    uint32_t nextID() { return nextID_++; }

private:
    std::string defaultPointAnnotationSymbol;
    std::map<uint32_t, std::unique_ptr<Annotation>> annotations;
    std::map<Tile::ID, LiveTile> annotationTiles;
    uint32_t nextID_;
};

class Annotation : private util::noncopyable {
public:
    Annotation(AnnotationType, std::vector<AnnotationSegment>);

    std::vector<AnnotationSegment> getGeometry() const { return geometry; }
    LatLng getPoint() const { return geometry[0][0]; }
    BoundingBox getBoundingBox() const { return bbox; }

public:
    const AnnotationType type = AnnotationType::Point;
    const std::vector<AnnotationSegment> geometry;
    BoundingBox bbox;
    std::map<Tile::ID, std::vector<LiveTileFeature>> tileFeatures;
};

}

#endif