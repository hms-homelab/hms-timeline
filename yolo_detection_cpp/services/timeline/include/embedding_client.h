#pragma once

#include <string>
#include <vector>

namespace hms {

/// Synchronous client for Ollama's /api/embed endpoint.
/// Uses libcurl — safe to call from any thread.
class EmbeddingClient {
public:
    explicit EmbeddingClient(const std::string& ollama_url = "http://localhost:11434",
                             const std::string& model = "nomic-embed-text");

    /// Generate a 768-dim embedding for text. Returns empty vector on error.
    std::vector<float> embed(const std::string& text);

private:
    std::string url_;
    std::string model_;
};

}  // namespace hms
