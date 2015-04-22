#ifndef MBGL_RENDERER_DEBUGBUCKET
#define MBGL_RENDERER_DEBUGBUCKET

#include <mbgl/renderer/bucket.hpp>
#include <mbgl/geometry/debug_font_buffer.hpp>
#include <mbgl/geometry/vao.hpp>

#include <vector>

#ifndef BUFFER_OFFSET
#define BUFFER_OFFSET(i) ((char *)nullptr + (i))
#endif

namespace mbgl {

class PlainShader;

class DebugBucket : public Bucket {
public:
    DebugBucket(DebugFontBuffer& fontBuffer);

    void prepare() override;
    void render(Painter&, const StyleLayer&, const TileID&, const mat4&) override;

    void drawLines(PlainShader& shader);
    void drawPoints(PlainShader& shader);

private:
    DebugFontBuffer& fontBuffer;
    VertexArrayObject array;
};

}

#endif
