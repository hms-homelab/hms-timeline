#pragma once

#include <drogon/HttpResponse.h>
#include <nlohmann/json.hpp>

namespace yolo {

/// Create a Drogon HTTP response with JSON body from an nlohmann::json object.
/// Drogon's newHttpJsonResponse uses jsoncpp; this helper serialises via nlohmann.
inline drogon::HttpResponsePtr makeJsonResponse(
    const nlohmann::json& j,
    drogon::HttpStatusCode code = drogon::k200OK)
{
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(code);
    resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    resp->setBody(j.dump());
    return resp;
}

} // namespace yolo
