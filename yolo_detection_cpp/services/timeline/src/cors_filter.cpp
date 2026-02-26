#include "cors_filter.h"
#include <spdlog/spdlog.h>
#include <algorithm>

using namespace drogon;

namespace yolo {

void CorsFilter::setAllowedOrigins(std::vector<std::string> origins) {
    allowed_origins_ = std::move(origins);
}

void CorsFilter::doFilter(const HttpRequestPtr& req,
                           FilterCallback&& fcb,
                           FilterChainCallback&& fccb) {
    // Collect origin; if not provided fall back to wildcard
    auto origin = std::string(req->getHeader("Origin"));

    bool wildcard = (allowed_origins_.empty() ||
        std::find(allowed_origins_.begin(), allowed_origins_.end(), "*") != allowed_origins_.end());

    bool origin_allowed = wildcard || (!origin.empty() &&
        std::find(allowed_origins_.begin(), allowed_origins_.end(), origin) != allowed_origins_.end());

    std::string allow_origin = (origin_allowed && !origin.empty()) ? origin : "*";

    // Handle preflight OPTIONS
    if (req->method() == HttpMethod::Options) {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(HttpStatusCode::k204NoContent);
        resp->addHeader("Access-Control-Allow-Origin", allow_origin);
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, Accept");
        resp->addHeader("Access-Control-Max-Age", "86400");
        if (allow_origin != "*") {
            resp->addHeader("Vary", "Origin");
        }
        fcb(resp);
        return;
    }

    // For other methods, pass through — CORS headers added globally via post-advice
    fccb();
}

} // namespace yolo
