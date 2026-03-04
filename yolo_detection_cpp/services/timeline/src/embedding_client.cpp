#include "embedding_client.h"

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

using json = nlohmann::json;

namespace hms {

EmbeddingClient::EmbeddingClient(const std::string& ollama_url,
                                 const std::string& model)
    : url_(ollama_url + "/api/embed"), model_(model)
{
}

static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* response = static_cast<std::string*>(userdata);
    response->append(ptr, size * nmemb);
    return size * nmemb;
}

std::vector<float> EmbeddingClient::embed(const std::string& text) {
    if (text.empty()) return {};

    json body = {
        {"model", model_},
        {"input", text}
    };
    std::string body_str = body.dump();
    std::string response_body;

    CURL* curl = curl_easy_init();
    if (!curl) {
        spdlog::error("EmbeddingClient: curl_easy_init failed");
        return {};
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body_str.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body_str.size()));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        spdlog::error("EmbeddingClient: curl error: {}", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return {};
    }

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (http_code != 200) {
        spdlog::error("EmbeddingClient: HTTP {}", http_code);
        return {};
    }

    try {
        auto j = json::parse(response_body);
        if (j.contains("embeddings") && !j["embeddings"].empty()) {
            return j["embeddings"][0].get<std::vector<float>>();
        }
        spdlog::error("EmbeddingClient: no embeddings in response");
    } catch (const json::exception& e) {
        spdlog::error("EmbeddingClient: parse error: {}", e.what());
    }

    return {};
}

}  // namespace hms
