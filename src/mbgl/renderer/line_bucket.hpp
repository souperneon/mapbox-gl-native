#ifndef MBGL_RENDERER_LINEBUCKET
#define MBGL_RENDERER_LINEBUCKET

#include <mbgl/renderer/bucket.hpp>
#include <mbgl/map/geometry_tile.hpp>
#include <mbgl/geometry/vao.hpp>
#include <mbgl/geometry/elements_buffer.hpp>
#include <mbgl/geometry/line_buffer.hpp>
#include <mbgl/style/style_bucket.hpp>
#include <mbgl/style/style_layout.hpp>
#include <mbgl/util/vec.hpp>

#include <vector>

namespace mbgl {

class Style;
class LineVertexBuffer;
class TriangleElementsBuffer;
class LineShader;
class LinejoinShader;
class LineSDFShader;
class LinepatternShader;

class LineBucket : public Bucket {
    typedef ElementGroup<3> triangle_group_type;
    typedef ElementGroup<1> point_group_type;

public:
    LineBucket(LineVertexBuffer &vertexBuffer,
               TriangleElementsBuffer &triangleElementsBuffer,
               PointElementsBuffer &pointElementsBuffer);
    ~LineBucket() override;

    void render(Painter &painter, const StyleLayer &layer_desc, const TileID &id,
                const mat4 &matrix) override;
    bool hasData() const override;

    void addGeometry(const GeometryCollection&);
    void addGeometry(const std::vector<Coordinate>& line);

    bool hasPoints() const;

    void drawLines(LineShader& shader);
    void drawLineSDF(LineSDFShader& shader);
    void drawLinePatterns(LinepatternShader& shader);
    void drawPoints(LinejoinShader& shader);

public:
    StyleLayoutLine layout;

private:
    LineVertexBuffer& vertexBuffer;
    TriangleElementsBuffer& triangleElementsBuffer;
    PointElementsBuffer& pointElementsBuffer;

    const size_t vertex_start;
    const size_t triangle_elements_start;
    const size_t point_elements_start;

    std::vector<std::unique_ptr<triangle_group_type>> triangleGroups;
    std::vector<std::unique_ptr<point_group_type>> pointGroups;
};

}

#endif
