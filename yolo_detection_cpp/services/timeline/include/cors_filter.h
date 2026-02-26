#pragma once

#include <drogon/HttpFilter.h>
#include <vector>
#include <string>

namespace yolo {

/// CORS middleware filter for Drogon.
/// Adds Access-Control-Allow-* headers to all responses and handles preflight OPTIONS requests.
class CorsFilter : public drogon::HttpFilter<CorsFilter> {
public:
    void doFilter(const drogon::HttpRequestPtr& req,
                  drogon::FilterCallback&& fcb,
                  drogon::FilterChainCallback&& fccb) override;

    /// Set allowed origins (called during startup from config)
    static void setAllowedOrigins(std::vector<std::string> origins);

private:
    static inline std::vector<std::string> allowed_origins_ = {"*"};
};

} // namespace yolo
