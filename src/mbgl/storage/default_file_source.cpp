#include <mbgl/storage/default_file_source_impl.hpp>
#include <mbgl/storage/request.hpp>
#include <mbgl/storage/asset_request.hpp>
#include <mbgl/storage/http_request.hpp>

#include <mbgl/storage/response.hpp>
#include <mbgl/platform/platform.hpp>

#include <mbgl/util/uv_detail.hpp>
#include <mbgl/util/chrono.hpp>
#include <mbgl/util/thread.hpp>
#include <mbgl/platform/log.hpp>
#include <mbgl/map/environment.hpp>

#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wshadow"
#endif
#include <boost/algorithm/string.hpp>
#pragma GCC diagnostic pop

#include <algorithm>
#include <cassert>


namespace algo = boost::algorithm;

namespace mbgl {

DefaultFileSource::DefaultFileSource(FileCache* cache, const std::string& root)
    : thread(util::make_unique<util::Thread<Impl>>("FileSource", cache, root)) {
}

DefaultFileSource::~DefaultFileSource() {
    MBGL_VERIFY_THREAD(tid);
}

Request* DefaultFileSource::request(const Resource& resource,
                                    uv_loop_t* l,
                                    const Environment& env,
                                    Callback callback) {
    auto req = new Request(resource, l, env, std::move(callback));

    // This function can be called from any thread. Make sure we're executing the actual call in the
    // file source loop by sending it over the queue.
    thread->invoke(&Impl::add, req, thread->get());

    return req;
}

void DefaultFileSource::request(const Resource& resource, const Environment& env, Callback callback) {
    request(resource, nullptr, env, std::move(callback));
}

void DefaultFileSource::cancel(Request *req) {
    req->cancel();

    // This function can be called from any thread. Make sure we're executing the actual call in the
    // file source loop by sending it over the queue.
    thread->invoke(&Impl::cancel, req);
}

void DefaultFileSource::abort(const Environment &env) {
    thread->invoke(&Impl::abort, std::ref(env));
}

// ----- Impl -----

DefaultFileSource::Impl::Impl(FileCache* cache_, const std::string& root)
    : cache(cache_), assetRoot(root.empty() ? platform::assetRoot() : root) {
}

DefaultFileRequest* DefaultFileSource::Impl::find(const Resource& resource) {
    const auto it = pending.find(resource);
    if (it != pending.end()) {
        return &it->second;
    }
    return nullptr;
}

void DefaultFileSource::Impl::add(Request* req, uv_loop_t* loop) {
    const Resource& resource = req->resource;
    DefaultFileRequest* request = find(resource);

    if (request) {
        request->observers.insert(req);
        return;
    }

    request = &pending.emplace(resource, DefaultFileRequest(resource, loop)).first->second;
    request->observers.insert(req);

    if (cache) {
        startCacheRequest(resource);
    } else {
        startRealRequest(resource);
    }
}

void DefaultFileSource::Impl::startCacheRequest(const Resource& resource) {
    // Check the cache for existing data so that we can potentially
    // revalidate the information without having to redownload everything.
    cache->get(resource, [this, resource](std::unique_ptr<Response> response) {
        DefaultFileRequest* request = find(resource);

        if (!request) {
            // There is no request for this URL anymore. Likely, the request was canceled
            // before we got around to process the cache result.
            return;
        }

        auto expired = [&response] {
            const int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
                                    SystemClock::now().time_since_epoch()).count();
            return response->expires <= now;
        };

        if (!response || expired()) {
            // No response or stale cache. Run the real request.
            startRealRequest(resource, std::move(response));
        } else {
            // The response is fresh. We're good to notify the caller.
            notify(request, std::move(response), FileCache::Hint::No);
        }
    });
}

void DefaultFileSource::Impl::startRealRequest(const Resource& resource, std::shared_ptr<const Response> response) {
    DefaultFileRequest* request = find(resource);

    auto callback = [request, this] (std::shared_ptr<const Response> res, FileCache::Hint hint) {
        notify(request, res, hint);
    };

    if (algo::starts_with(resource.url, "asset://")) {
        request->request = new AssetRequest(resource, callback, assetRoot);
    } else {
        request->request = new HTTPRequest(resource, callback);
    }

    request->request->start(request->loop, response);
}

void DefaultFileSource::Impl::cancel(Request* req) {
    DefaultFileRequest* request = find(req->resource);

    if (request) {
        // If the number of dependent requests of the DefaultFileRequest drops to zero,
        // cancel the request and remove it from the pending list.
        request->observers.erase(req);
        if (request->observers.empty()) {
            if (request->request) {
                request->request->cancel();
            }
            pending.erase(request->resource);
        }
    } else {
        // There is no request for this URL anymore. Likely, the request already completed
        // before we got around to process the cancelation request.
    }

    // Send a message back to the requesting thread and notify it that this request has been
    // canceled and is now safe to be deleted.
    req->destruct();
}

// Aborts all requests that are part of the current environment.
void DefaultFileSource::Impl::abort(const Environment& env) {
    // Construct a cancellation response.
    auto response = std::make_shared<Response>();
    response->status = Response::Error;
    response->message = "Environment is terminating";

    // Iterate through all pending requests and remove them in case they're abandoned.
    util::erase_if(pending, [&](std::pair<const Resource, DefaultFileRequest>& it) -> bool {
        // Notify all pending requests that are in the current environment.
        util::erase_if(it.second.observers, [&](Request* req) -> bool {
            if (&req->env == &env) {
                req->notify(response);
                return true;
            } else {
                return false;
            }
        });

        bool abandoned = it.second.observers.empty();

        if (abandoned && it.second.request) {
            it.second.request->cancel();
        }

        return abandoned;
    });
}

void DefaultFileSource::Impl::notify(DefaultFileRequest* request, std::shared_ptr<const Response> response, FileCache::Hint hint) {
    // First, remove the request, since it might be destructed at any point now.
    assert(find(request->resource) == request);
    assert(response);

    // Notify all observers.
    for (auto req : request->observers) {
        req->notify(response);
    }

    if (cache) {
        // Store response in database
        cache->put(request->resource, response, hint);
    }

    pending.erase(request->resource);
}

}
