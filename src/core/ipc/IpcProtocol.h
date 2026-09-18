#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include <variant>
#include <vector>
#include <map>

namespace pdfpro::ipc {

using json = nlohmann::json;

// =============================================================================
// JSON-RPC 2.0 Message Types
// =============================================================================

struct Request {
    std::string jsonrpc = "2.0";
    std::string method;
    json params;
    json id; // null for notifications

    json toJson() const;
    static Request fromJson(const json& j);
};

struct Response {
    std::string jsonrpc = "2.0";
    json result;
    json error; // null if success
    json id;

    json toJson() const;
    static Response fromJson(const json& j);

    bool isError() const { return !error.is_null(); }
};

struct Notification {
    std::string jsonrpc = "2.0";
    std::string method;
    json params;

    json toJson() const;
    static Notification fromJson(const json& j);
};

// =============================================================================
// PDF Engine Methods
// =============================================================================

namespace methods {
    // Document lifecycle
    constexpr auto OPEN_DOCUMENT = "document.open";
    constexpr auto CLOSE_DOCUMENT = "document.close";
    constexpr auto SAVE_DOCUMENT = "document.save";
    constexpr auto SAVE_DOCUMENT_AS = "document.saveAs";
    constexpr auto GET_DOCUMENT_INFO = "document.getInfo";
    constexpr auto GET_PAGE_COUNT = "document.getPageCount";

    // Page rendering
    constexpr auto RENDER_PAGE = "page.render";
    constexpr auto RENDER_PAGE_TILES = "page.renderTiles";
    constexpr auto GET_PAGE_SIZE = "page.getSize";
    constexpr auto GET_PAGE_ROTATION = "page.getRotation";
    constexpr auto SET_PAGE_ROTATION = "page.setRotation";

    // Page management
    constexpr auto INSERT_PAGE = "page.insert";
    constexpr auto REMOVE_PAGE = "page.remove";
    constexpr auto MOVE_PAGE = "page.move";
    constexpr auto COPY_PAGE = "page.copy";

    // Text & Selection
    constexpr auto GET_PAGE_TEXT = "text.getPageText";
    constexpr auto GET_TEXT_BOUNDS = "text.getBounds";
    constexpr auto SEARCH_TEXT = "text.search";
    constexpr auto GET_SELECTION = "text.getSelection";

    // Annotations
    constexpr auto GET_ANNOTATIONS = "annotation.getAll";
    constexpr auto ADD_ANNOTATION = "annotation.add";
    constexpr auto UPDATE_ANNOTATION = "annotation.update";
    constexpr auto REMOVE_ANNOTATION = "annotation.remove";

    // Forms
    constexpr auto GET_FORM_FIELDS = "form.getFields";
    constexpr auto SET_FORM_FIELD = "form.setField";
    constexpr auto FLATTEN_FORM = "form.flatten";

    // Security
    constexpr auto SET_PASSWORD = "security.setPassword";
    constexpr auto REMOVE_PASSWORD = "security.removePassword";
    constexpr auto GET_PERMISSIONS = "security.getPermissions";

    // Operations
    constexpr auto MERGE_DOCUMENTS = "operation.merge";
    constexpr auto SPLIT_DOCUMENT = "operation.split";
    constexpr auto EXPORT_IMAGES = "operation.exportImages";
    constexpr auto OCR_PAGE = "operation.ocr";
    constexpr auto REDACT = "operation.redact";

    // Thumbnails
    constexpr auto GET_THUMBNAIL = "thumbnail.get";
    constexpr auto GET_THUMBNAILS = "thumbnail.getAll";

    // Notifications (engine -> UI)
    constexpr auto PROGRESS = "progress";
    constexpr auto ERROR = "error";
    constexpr auto PAGE_RENDERED = "page.rendered";
    constexpr auto DOCUMENT_CHANGED = "document.changed";
} // namespace methods

// =============================================================================
// Parameter/Result Types
// =============================================================================

struct DocumentHandle {
    uint64_t value = 0;
    bool isValid() const { return value != 0; }
    bool operator==(const DocumentHandle& other) const { return value == other.value; }
};

struct PageHandle {
    DocumentHandle doc;
    int index = -1; // 0-based
    bool isValid() const { return doc.isValid() && index >= 0; }
};

struct RectF {
    double x = 0, y = 0, w = 0, h = 0;
    bool isEmpty() const { return w <= 0 || h <= 0; }
};

struct PointF {
    double x = 0, y = 0;
};

struct SizeF {
    double w = 0, h = 0;
};

struct RenderOptions {
    double dpi = 150.0;
    double scale = 1.0;
    int rotation = 0; // 0, 90, 180, 270
    bool antialias = true;
    bool textAntialias = true;
    bool useColors = true;
    RectF clipRect; // empty = full page
    std::string format = "png"; // png, jpeg, webp, raw
};

struct Tile {
    int x = 0, y = 0; // tile grid position
    int level = 0;    // zoom level
    std::string data; // base64 encoded image
};

struct PageText {
    std::string text;
    std::vector<RectF> charBounds; // per-character bounds
    std::vector<RectF> wordBounds;
    std::vector<RectF> lineBounds;
    std::vector<RectF> blockBounds;
};

struct SearchResult {
    int pageIndex = 0;
    std::string matchText;
    RectF bounds;
    int charOffset = 0;
};

struct Annotation {
    enum class Type {
        Highlight,
        Underline,
        StrikeOut,
        Squiggly,
        FreeText,
        Line,
        Square,
        Circle,
        Ink,
        Stamp,
        Caret,
        Link,
        Widget // form field
    };

    uint64_t id = 0;
    Type type = Type::Highlight;
    int pageIndex = 0;
    RectF rect;
    std::string contents;
    std::string author;
    std::string subject;
    uint32_t color = 0xFFFF0000; // ARGB
    double opacity = 1.0;
    json properties; // type-specific
};

struct FormField {
    std::string name;
    std::string type; // text, button, choice, signature
    std::string value;
    RectF rect;
    int pageIndex = 0;
    bool readOnly = false;
    bool required = false;
    json options; // for choice fields
};

struct DocumentInfo {
    std::string title;
    std::string author;
    std::string subject;
    std::string keywords;
    std::string creator;
    std::string producer;
    std::string creationDate;
    std::string modificationDate;
    int pageCount = 0;
    bool isEncrypted = false;
    bool isLinearized = false;
    std::string pdfVersion;
    SizeF pageSize; // first page
};

struct Permissions {
    bool canPrint = true;
    bool canModify = true;
    bool canCopy = true;
    bool canAnnotate = true;
    bool canFillForms = true;
    bool canExtract = true;
    bool canAssemble = true;
    bool canPrintHighQuality = true;
};

// =============================================================================
// JSON Serialization Helpers
// =============================================================================

inline void to_json(json& j, const RectF& r) {
    j = {{"x", r.x}, {"y", r.y}, {"w", r.w}, {"h", r.h}};
}
inline void from_json(const json& j, RectF& r) {
    r.x = j.value("x", 0.0);
    r.y = j.value("y", 0.0);
    r.w = j.value("w", 0.0);
    r.h = j.value("h", 0.0);
}

inline void to_json(json& j, const PointF& p) {
    j = {{"x", p.x}, {"y", p.y}};
}
inline void from_json(const json& j, PointF& p) {
    p.x = j.value("x", 0.0);
    p.y = j.value("y", 0.0);
}

inline void to_json(json& j, const SizeF& s) {
    j = {{"w", s.w}, {"h", s.h}};
}
inline void from_json(const json& j, SizeF& s) {
    s.w = j.value("w", 0.0);
    s.h = j.value("h", 0.0);
}

inline void to_json(json& j, const RenderOptions& o) {
    j = {{"dpi", o.dpi}, {"scale", o.scale}, {"rotation", o.rotation},
         {"antialias", o.antialias}, {"textAntialias", o.textAntialias},
         {"useColors", o.useColors}, {"format", o.format}};
    if (!o.clipRect.isEmpty()) j["clipRect"] = o.clipRect;
}
inline void from_json(const json& j, RenderOptions& o) {
    o.dpi = j.value("dpi", 150.0);
    o.scale = j.value("scale", 1.0);
    o.rotation = j.value("rotation", 0);
    o.antialias = j.value("antialias", true);
    o.textAntialias = j.value("textAntialias", true);
    o.useColors = j.value("useColors", true);
    o.format = j.value("format", "png");
    if (j.contains("clipRect")) j.at("clipRect").get_to(o.clipRect);
}

inline void to_json(json& j, const DocumentHandle& h) { j = h.value; }
inline void from_json(const json& j, DocumentHandle& h) { h.value = j.get<uint64_t>(); }

inline void to_json(json& j, const PageHandle& h) { j = {{"doc", h.doc}, {"index", h.index}}; }
inline void from_json(const json& j, PageHandle& h) { h.doc = j.at("doc").get<DocumentHandle>(); h.index = j.at("index").get<int>(); }

inline void to_json(json& j, const Tile& t) { j = {{"x", t.x}, {"y", t.y}, {"level", t.level}, {"data", t.data}}; }
inline void from_json(const json& j, Tile& t) { t.x = j.at("x").get<int>(); t.y = j.at("y").get<int>(); t.level = j.at("level").get<int>(); t.data = j.at("data").get<std::string>(); }

inline void to_json(json& j, const PageText& t) {
    j = {{"text", t.text}, {"charBounds", t.charBounds}, {"wordBounds", t.wordBounds},
         {"lineBounds", t.lineBounds}, {"blockBounds", t.blockBounds}};
}
inline void from_json(const json& j, PageText& t) {
    t.text = j.at("text").get<std::string>();
    t.charBounds = j.value("charBounds", std::vector<RectF>{});
    t.wordBounds = j.value("wordBounds", std::vector<RectF>{});
    t.lineBounds = j.value("lineBounds", std::vector<RectF>{});
    t.blockBounds = j.value("blockBounds", std::vector<RectF>{});
}

inline void to_json(json& j, const SearchResult& r) {
    j = {{"pageIndex", r.pageIndex}, {"matchText", r.matchText}, {"bounds", r.bounds}, {"charOffset", r.charOffset}};
}
inline void from_json(const json& j, SearchResult& r) {
    r.pageIndex = j.at("pageIndex").get<int>();
    r.matchText = j.at("matchText").get<std::string>();
    r.bounds = j.at("bounds").get<RectF>();
    r.charOffset = j.at("charOffset").get<int>();
}

inline void to_json(json& j, const Annotation& a) {
    j = {{"id", a.id}, {"type", static_cast<int>(a.type)}, {"pageIndex", a.pageIndex},
         {"rect", a.rect}, {"contents", a.contents}, {"author", a.author},
         {"subject", a.subject}, {"color", a.color}, {"opacity", a.opacity},
         {"properties", a.properties}};
}
inline void from_json(const json& j, Annotation& a) {
    a.id = j.value("id", 0ull);
    a.type = static_cast<Annotation::Type>(j.value("type", 0));
    a.pageIndex = j.value("pageIndex", 0);
    a.rect = j.value("rect", RectF{});
    a.contents = j.value("contents", "");
    a.author = j.value("author", "");
    a.subject = j.value("subject", "");
    a.color = j.value("color", 0xFFFF0000u);
    a.opacity = j.value("opacity", 1.0);
    a.properties = j.value("properties", json{});
}

inline void to_json(json& j, const FormField& f) {
    j = {{"name", f.name}, {"type", f.type}, {"value", f.value}, {"rect", f.rect},
         {"pageIndex", f.pageIndex}, {"readOnly", f.readOnly}, {"required", f.required},
         {"options", f.options}};
}
inline void from_json(const json& j, FormField& f) {
    f.name = j.at("name").get<std::string>();
    f.type = j.at("type").get<std::string>();
    f.value = j.value("value", "");
    f.rect = j.value("rect", RectF{});
    f.pageIndex = j.value("pageIndex", 0);
    f.readOnly = j.value("readOnly", false);
    f.required = j.value("required", false);
    f.options = j.value("options", json{});
}

inline void to_json(json& j, const DocumentInfo& i) {
    j = {{"title", i.title}, {"author", i.author}, {"subject", i.subject},
         {"keywords", i.keywords}, {"creator", i.creator}, {"producer", i.producer},
         {"creationDate", i.creationDate}, {"modificationDate", i.modificationDate},
         {"pageCount", i.pageCount}, {"isEncrypted", i.isEncrypted},
         {"isLinearized", i.isLinearized}, {"pdfVersion", i.pdfVersion},
         {"pageSize", i.pageSize}};
}
inline void from_json(const json& j, DocumentInfo& i) {
    i.title = j.value("title", "");
    i.author = j.value("author", "");
    i.subject = j.value("subject", "");
    i.keywords = j.value("keywords", "");
    i.creator = j.value("creator", "");
    i.producer = j.value("producer", "");
    i.creationDate = j.value("creationDate", "");
    i.modificationDate = j.value("modificationDate", "");
    i.pageCount = j.value("pageCount", 0);
    i.isEncrypted = j.value("isEncrypted", false);
    i.isLinearized = j.value("isLinearized", false);
    i.pdfVersion = j.value("pdfVersion", "");
    i.pageSize = j.value("pageSize", SizeF{});
}

inline void to_json(json& j, const Permissions& p) {
    j = {{"canPrint", p.canPrint}, {"canModify", p.canModify}, {"canCopy", p.canCopy},
         {"canAnnotate", p.canAnnotate}, {"canFillForms", p.canFillForms},
         {"canExtract", p.canExtract}, {"canAssemble", p.canAssemble},
         {"canPrintHighQuality", p.canPrintHighQuality}};
}
inline void from_json(const json& j, Permissions& p) {
    p.canPrint = j.value("canPrint", true);
    p.canModify = j.value("canModify", true);
    p.canCopy = j.value("canCopy", true);
    p.canAnnotate = j.value("canAnnotate", true);
    p.canFillForms = j.value("canFillForms", true);
    p.canExtract = j.value("canExtract", true);
    p.canAssemble = j.value("canAssemble", true);
    p.canPrintHighQuality = j.value("canPrintHighQuality", true);
}

} // namespace pdfpro::ipc