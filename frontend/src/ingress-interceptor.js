(function() {
    "use strict";

    // Detect ingress path from current URL
    // HA ingress URLs look like: /api/hassio/ingress/<token>/...
    var match = window.location.pathname.match(/^(\/api\/hassio\/ingress\/[^/]+)/);
    if (!match) return; // Not running under ingress — no rewriting needed

    var prefix = match[1];

    // Helper: should this URL be rewritten?
    function shouldRewrite(url) {
        return typeof url === "string" &&
               url.startsWith("/") &&
               !url.startsWith(prefix) &&
               !url.startsWith("//"); // protocol-relative URLs
    }

    // 1. Patch window.fetch — Angular uses withFetch(), so all API calls go through here
    var origFetch = window.fetch;
    window.fetch = function(input, init) {
        if (input instanceof Request) {
            var url = new URL(input.url);
            if (shouldRewrite(url.pathname)) {
                input = new Request(prefix + url.pathname + url.search + url.hash, input);
            }
        } else if (shouldRewrite(input)) {
            input = prefix + input;
        }
        return origFetch.call(this, input, init);
    };

    // 2. Patch XMLHttpRequest.open (fallback for any non-fetch calls)
    var origXHROpen = XMLHttpRequest.prototype.open;
    XMLHttpRequest.prototype.open = function(method, url) {
        if (shouldRewrite(url)) {
            arguments[1] = prefix + url;
        }
        return origXHROpen.apply(this, arguments);
    };

    // 3. Override src property setters on media elements
    // This catches Angular template bindings like [src]="/api/cameras/..." and
    // lazy-load directives that set element.src = "/snapshots/..."
    ["HTMLImageElement", "HTMLVideoElement", "HTMLAudioElement", "HTMLSourceElement"].forEach(function(cls) {
        var proto = window[cls] && window[cls].prototype;
        if (!proto) return;

        var desc = Object.getOwnPropertyDescriptor(proto, "src");
        if (!desc || !desc.set) return;

        Object.defineProperty(proto, "src", {
            get: desc.get,
            set: function(val) {
                if (shouldRewrite(val)) {
                    val = prefix + val;
                }
                desc.set.call(this, val);
            },
            enumerable: desc.enumerable,
            configurable: desc.configurable
        });
    });

    // 4. MutationObserver — catch elements added to the DOM with src/href attributes
    // that were set via setAttribute() or static HTML from innerHTML
    var observer = new MutationObserver(function(mutations) {
        mutations.forEach(function(mutation) {
            mutation.addedNodes.forEach(function(node) {
                if (node.nodeType !== 1) return; // Element nodes only

                // Rewrite src attributes
                if (node.hasAttribute && node.hasAttribute("src")) {
                    var src = node.getAttribute("src");
                    if (shouldRewrite(src)) {
                        node.setAttribute("src", prefix + src);
                    }
                }

                // Also check child elements (e.g., innerHTML with <img src="/...">)
                if (node.querySelectorAll) {
                    node.querySelectorAll("[src]").forEach(function(el) {
                        var s = el.getAttribute("src");
                        if (shouldRewrite(s)) {
                            el.setAttribute("src", prefix + s);
                        }
                    });
                }
            });
        });
    });

    observer.observe(document.documentElement, {
        childList: true,
        subtree: true
    });

    console.log("[ingress-interceptor] Active, prefix:", prefix);
})();
