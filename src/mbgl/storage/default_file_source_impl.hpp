#ifndef MBGL_STORAGE_DEFAULT_DEFAULT_FILE_SOURCE_IMPL
#define MBGL_STORAGE_DEFAULT_DEFAULT_FILE_SOURCE_IMPL

#include <mbgl/storage/default_file_source.hpp>

#include <set>
#include <unordered_map>

typedef struct uv_loop_s uv_loop_t;

namespace mbgl {

class SharedRequestBase;

class DefaultFileSource::Impl {
public:
    Impl(uv_loop_t*, FileCache *cache, const std::string &root = "");

    void notify(SharedRequestBase *sharedRequest, const std::set<Request *> &observers,
                std::shared_ptr<const Response> response, FileCache::Hint hint);
    SharedRequestBase *find(const Resource &resource);

    void add(Request* request);
    void cancel(Request* request);
    void abort(const Environment& env);

    const std::string assetRoot;

private:
    void processResult(const Resource& resource, std::shared_ptr<const Response> response);

    std::unordered_map<Resource, SharedRequestBase *, Resource::Hash> pending;
    uv_loop_t* loop = nullptr;
    FileCache* cache = nullptr;
};

}

#endif
