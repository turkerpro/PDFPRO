#include "PopplerEngine.h"
#include <spdlog/spdlog.h>
#include <QBuffer>
#include <QImageWriter>
#include <QTemporaryFile>
#include <QFile>
#include <QDir>

namespace pdfpro::poppler {

PopplerEngine::PopplerEngine()
    : JsonRpcServer(std::make_unique<StdioTransport>()) {

    // Document lifecycle
    registerMethod(methods::OPEN_DOCUMENT, [this](const json& p) -> json { return openDocument(p); });
    registerMethod(methods::CLOSE_DOCUMENT, [this](const json& p) -> json { closeDocument(p); return json{}; });
    registerMethod(methods::SAVE_DOCUMENT, [this](const json& p) -> json { return saveDocument(p); });
    registerMethod(methods::SAVE_DOCUMENT_AS, [this](const json& p) -> json { return saveDocumentAs(p); });
    registerMethod(methods::GET_DOCUMENT_INFO, [this](const json& p) -> json { return getDocumentInfo(p); });
    registerMethod(methods::GET_PAGE_COUNT, [this](const json& p) -> json { return getPageCount(p); });

    // Page rendering
    registerMethod(methods::RENDER_PAGE, [this](const json& p) -> json { return renderPage(p); });
    registerMethod(methods::RENDER_PAGE_TILES, [this](const json& p) -> json { return renderPageTiles(p); });
    registerMethod(methods::GET_PAGE_SIZE, [this](const json& p) -> json { return getPageSize(p); });
    registerMethod(methods::GET_PAGE_ROTATION, [this](const json& p) -> json { return getPageRotation(p); });
    registerMethod(methods::SET_PAGE_ROTATION, [this](const json& p) -> json { setPageRotation(p); return json{}; });

    // Page management
    registerMethod(methods::INSERT_PAGE, [this](const json& p) -> json { insertPage(p); return json{}; });
    registerMethod(methods::REMOVE_PAGE, [this](const json& p) -> json { removePage(p); return json{}; });
    registerMethod(methods::MOVE_PAGE, [this](const json& p) -> json { movePage(p); return json{}; });
    registerMethod(methods::COPY_PAGE, [this](const json& p) -> json { copyPage(p); return json{}; });

    // Text
    registerMethod(methods::GET_PAGE_TEXT, [this](const json& p) -> json { return getPageText(p); });
    registerMethod(methods::GET_TEXT_BOUNDS, [this](const json& p) -> json { return getTextBounds(p); });
    registerMethod(methods::SEARCH_TEXT, [this](const json& p) -> json { return searchText(p); });
    registerMethod(methods::GET_SELECTION, [this](const json& p) -> json { return getSelection(p); });

    // Annotations
    registerMethod(methods::GET_ANNOTATIONS, [this](const json& p) -> json { return getAnnotations(p); });
    registerMethod(methods::ADD_ANNOTATION, [this](const json& p) -> json { return addAnnotation(p); });
    registerMethod(methods::UPDATE_ANNOTATION, [this](const json& p) -> json { updateAnnotation(p); return json{}; });
    registerMethod(methods::REMOVE_ANNOTATION, [this](const json& p) -> json { removeAnnotation(p); return json{}; });

    // Forms
    registerMethod(methods::GET_FORM_FIELDS, [this](const json& p) -> json { return getFormFields(p); });
    registerMethod(methods::SET_FORM_FIELD, [this](const json& p) -> json { setFormField(p); return json{}; });
    registerMethod(methods::FLATTEN_FORM, [this](const json& p) -> json { flattenForm(p); return json{}; });

    // Security
    registerMethod(methods::SET_PASSWORD, [this](const json& p) -> json { setPassword(p); return json{}; });
    registerMethod(methods::REMOVE_PASSWORD, [this](const json& p) -> json { removePassword(p); return json{}; });
    registerMethod(methods::GET_PERMISSIONS, [this](const json& p) -> json { return getPermissions(p); });

    // Thumbnails
    registerMethod(methods::GET_THUMBNAIL, [this](const json& p) -> json { return getThumbnail(p); });
    registerMethod(methods::GET_THUMBNAILS, [this](const json& p) -> json { return getThumbnails(p); });
}

PopplerEngine::~PopplerEngine() {
    std::lock_guard<std::mutex> lock(docsMutex_);
    documents_.clear();
}

bool PopplerEngine::initialize() {
    spdlog::info("PopplerEngine initializing...");
    return start();
}

DocumentHandle PopplerEngine::openDocument(const json& params) {
    std::string filePath = params.at("filePath").get<std::string>();
    std::string password = params.value("password", "");

    auto doc = Poppler::Document::load(QString::fromStdString(filePath));
    if (!doc) {
        throw std::runtime_error("Failed to load document: " + filePath);
    }

    if (doc->isLocked()) {
        if (!password.empty()) {
            if (!doc->unlock(QString::fromStdString(password), QString())) {
                throw std::runtime_error("Invalid password");
            }
        } else {
            throw std::runtime_error("Document is password protected");
        }
    }

    doc->setRenderHint(Poppler::Document::Antialiasing, true);
    doc->setRenderHint(Poppler::Document::TextAntialiasing, true);
    doc->setRenderHint(Poppler::Document::TextHinting, true);

    DocumentHandle handle;
    {
        std::lock_guard<std::mutex> lock(docsMutex_);
        handle.value = nextDocHandle_++;
        auto data = std::make_unique<DocumentData>();
        data->doc = std::unique_ptr<Poppler::Document>(doc);
        data->filePath = filePath;
        data->password = password;
        documents_[handle.value] = std::move(data);
    }

    spdlog::info("Opened document: {} (handle: {})", filePath, handle.value);
    return handle;
}

void PopplerEngine::closeDocument(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    std::lock_guard<std::mutex> lock(docsMutex_);
    documents_.erase(handle.value);
    spdlog::info("Closed document handle: {}", handle.value);
}

json PopplerEngine::saveDocument(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    Poppler::Document* doc = getDocument(handle);
    if (!doc) throw std::runtime_error("Invalid document handle");

    auto it = documents_.find(handle.value);
    if (it == documents_.end()) throw std::runtime_error("Document not found");

    // Poppler doesn't support saving directly - we'd need qpdf for that
    // This is a limitation noted in the analysis
    return {{"success", false}, {"error", "Poppler does not support saving. Use qpdf engine."}};
}

json PopplerEngine::saveDocumentAs(const json& params) {
    return {{"success", false}, {"error", "Poppler does not support saving. Use qpdf engine."}};
}

DocumentInfo PopplerEngine::getDocumentInfo(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    Poppler::Document* doc = getDocument(handle);
    if (!doc) throw std::runtime_error("Invalid document handle");

    DocumentInfo info;
    info.title = doc->info("Title").toStdString();
    info.author = doc->info("Author").toStdString();
    info.subject = doc->info("Subject").toStdString();
    info.keywords = doc->info("Keywords").toStdString();
    info.creator = doc->info("Creator").toStdString();
    info.producer = doc->info("Producer").toStdString();
    info.creationDate = doc->info("CreationDate").toStdString();
    info.modificationDate = doc->info("ModDate").toStdString();
    info.pageCount = doc->numPages();
    info.isEncrypted = doc->isEncrypted();
    info.isLinearized = doc->isLinearized();
    info.pdfVersion = QString("1.%1").arg(doc->pdfVersion()).toStdString();

    if (doc->numPages() > 0) {
        auto page = doc->page(0);
        if (page) {
            QSizeF size = page->pageSizeF();
            info.pageSize = {size.width(), size.height()};
        }
    }

    return info;
}

int PopplerEngine::getPageCount(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    Poppler::Document* doc = getDocument(handle);
    if (!doc) throw std::runtime_error("Invalid document handle");
    return doc->numPages();
}

json PopplerEngine::renderPage(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    int pageIndex = params.at("pageIndex").get<int>();
    RenderOptions options = params.value("options", RenderOptions{});

    Poppler::Document* doc = getDocument(handle);
    if (!doc) throw std::runtime_error("Invalid document handle");

    auto page = doc->page(pageIndex);
    if (!page) throw std::runtime_error("Invalid page index: " + std::to_string(pageIndex));

    QImage image = renderPageToImage(page, options);
    std::string base64 = imageToBase64(image, options.format);

    return {{"image", base64}, {"width", image.width()}, {"height", image.height()}, {"format", options.format}};
}

json PopplerEngine::renderPageTiles(const json& params) {
    // For large PDFs - render in tiles for progressive loading
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    int pageIndex = params.at("pageIndex").get<int>();
    int tileSize = params.value("tileSize", 256);
    int maxLevel = params.value("maxLevel", 3);

    Poppler::Document* doc = getDocument(handle);
    if (!doc) throw std::runtime_error("Invalid document handle");

    auto page = doc->page(pageIndex);
    if (!page) throw std::runtime_error("Invalid page index");

    QSizeF pageSize = page->pageSizeF();
    json result = json::array();

    for (int level = 0; level <= maxLevel; ++level) {
        double scale = std::pow(2.0, level);
        int tilesX = std::ceil(pageSize.width() * scale / tileSize);
        int tilesY = std::ceil(pageSize.height() * scale / tileSize);

        for (int tx = 0; tx < tilesX; ++tx) {
            for (int ty = 0; ty < tilesY; ++ty) {
                RenderOptions opts;
                opts.scale = scale;
                opts.dpi = 72.0 * scale;
                opts.clipRect = {tx * tileSize / scale, ty * tileSize / scale, tileSize / scale, tileSize / scale};

                QImage tile = renderPageToImage(page, opts);
                if (!tile.isNull()) {
                    std::string base64 = imageToBase64(tile, "png");
                    result.push_back({{"x", tx}, {"y", ty}, {"level", level}, {"data", base64}});
                }
            }
        }
    }

    return {{"tiles", result}, {"pageWidth", pageSize.width()}, {"pageHeight", pageSize.height()}};
}

SizeF PopplerEngine::getPageSize(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    int pageIndex = params.at("pageIndex").get<int>();

    Poppler::Document* doc = getDocument(handle);
    if (!doc) throw std::runtime_error("Invalid document handle");

    auto page = doc->page(pageIndex);
    if (!page) throw std::runtime_error("Invalid page index");

    QSizeF size = page->pageSizeF();
    return {size.width(), size.height()};
}

int PopplerEngine::getPageRotation(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    int pageIndex = params.at("pageIndex").get<int>();

    Poppler::Document* doc = getDocument(handle);
    if (!doc) throw std::runtime_error("Invalid document handle");

    auto page = doc->page(pageIndex);
    if (!page) throw std::runtime_error("Invalid page index");

    return page->pageRotation();
}

void PopplerEngine::setPageRotation(const json& params) {
    // Poppler doesn't support modifying page rotation directly
    // This would require qpdf
    throw std::runtime_error("Page rotation modification not supported by Poppler. Use qpdf engine.");
}

void PopplerEngine::insertPage(const json& params) {
    throw std::runtime_error("Page insertion not supported by Poppler. Use qpdf engine.");
}

void PopplerEngine::removePage(const json& params) {
    throw std::runtime_error("Page removal not supported by Poppler. Use qpdf engine.");
}

void PopplerEngine::movePage(const json& params) {
    throw std::runtime_error("Page move not supported by Poppler. Use qpdf engine.");
}

void PopplerEngine::copyPage(const json& params) {
    throw std::runtime_error("Page copy not supported by Poppler. Use qpdf engine.");
}

PageText PopplerEngine::getPageText(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    int pageIndex = params.at("pageIndex").get<int>();

    Poppler::Document* doc = getDocument(handle);
    if (!doc) throw std::runtime_error("Invalid document handle");

    auto page = doc->page(pageIndex);
    if (!page) throw std::runtime_error("Invalid page index");

    QString text = page->text();
    PageText result;
    result.text = text.toStdString();

    // Get character bounds
    auto chars = page->textList();
    for (const auto& box : chars) {
        if (box.boundingBox().isValid()) {
            QRectF r = box.boundingBox();
            result.charBounds.push_back({r.x(), r.y(), r.width(), r.height()});
        }
    }

    return result;
}

std::vector<RectF> PopplerEngine::getTextBounds(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    int pageIndex = params.at("pageIndex").get<int>();
    int charStart = params.value("charStart", 0);
    int charLength = params.value("charLength", -1);

    Poppler::Document* doc = getDocument(handle);
    if (!doc) throw std::runtime_error("Invalid document handle");

    auto page = doc->page(pageIndex);
    if (!page) throw std::runtime_error("Invalid page index");

    auto chars = page->textList();
    std::vector<RectF> bounds;

    int end = (charLength >= 0) ? charStart + charLength : chars.size();
    for (int i = charStart; i < std::min(end, (int)chars.size()); ++i) {
        QRectF r = chars[i].boundingBox();
        if (r.isValid()) {
            bounds.push_back({r.x(), r.y(), r.width(), r.height()});
        }
    }

    return bounds;
}

std::vector<SearchResult> PopplerEngine::searchText(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    std::string query = params.at("query").get<std::string>();
    bool caseSensitive = params.value("caseSensitive", false);
    bool wholeWord = params.value("wholeWord", false);

    Poppler::Document* doc = getDocument(handle);
    if (!doc) throw std::runtime_error("Invalid document handle");

    std::vector<SearchResult> results;

    for (int i = 0; i < doc->numPages(); ++i) {
        auto page = doc->page(i);
        if (!page) continue;

        auto textList = page->textList();
        QString fullText;
        for (const auto& box : textList) {
            fullText += box.text();
        }

        Qt::CaseSensitivity cs = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
        int pos = 0;
        while ((pos = fullText.indexOf(QString::fromStdString(query), pos, cs)) != -1) {
            if (wholeWord) {
                // Check word boundaries
                QChar before = (pos > 0) ? fullText[pos - 1] : QChar();
                QChar after = (pos + query.length() < fullText.length()) ? fullText[pos + query.length()] : QChar();
                if ((before.isLetterOrNumber() || before == '_') ||
                    (after.isLetterOrNumber() || after == '_')) {
                    pos += query.length();
                    continue;
                }
            }

            // Find bounding box for this match
            int charCount = 0;
            RectF matchBounds;
            bool foundBounds = false;

            for (const auto& box : textList) {
                QString boxText = box.text();
                int boxLen = boxText.length();
                if (charCount + boxLen > pos && !foundBounds) {
                    QRectF r = box.boundingBox();
                    matchBounds = {r.x(), r.y(), r.width(), r.height()};
                    foundBounds = true;
                }
                charCount += boxLen;
                if (charCount >= pos + query.length()) break;
            }

            results.push_back({i, query, matchBounds, pos});
            pos += query.length();
        }
    }

    return results;
}

json PopplerEngine::getSelection(const json& params) {
    // Return current text selection (if any)
    return {{"hasSelection", false}};
}

std::vector<Annotation> PopplerEngine::getAnnotations(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    int pageIndex = params.value("pageIndex", -1);

    Poppler::Document* doc = getDocument(handle);
    if (!doc) throw std::runtime_error("Invalid document handle");

    std::vector<Annotation> annotations;

    int start = (pageIndex >= 0) ? pageIndex : 0;
    int end = (pageIndex >= 0) ? pageIndex + 1 : doc->numPages();

    for (int i = start; i < end; ++i) {
        auto page = doc->page(i);
        if (!page) continue;

        auto annotList = page->annotations();
        for (auto* annot : annotList) {
            annotations.push_back(convertAnnotation(annot, i));
        }
    }

    return annotations;
}

uint64_t PopplerEngine::addAnnotation(const json& params) {
    // Poppler doesn't support adding annotations directly
    // Would need to use qpdf or modify the document
    throw std::runtime_error("Adding annotations not supported by Poppler. Use qpdf engine.");
}

void PopplerEngine::updateAnnotation(const json& params) {
    throw std::runtime_error("Updating annotations not supported by Poppler. Use qpdf engine.");
}

void PopplerEngine::removeAnnotation(const json& params) {
    throw std::runtime_error("Removing annotations not supported by Poppler. Use qpdf engine.");
}

std::vector<FormField> PopplerEngine::getFormFields(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    int pageIndex = params.value("pageIndex", -1);

    Poppler::Document* doc = getDocument(handle);
    if (!doc) throw std::runtime_error("Invalid document handle");

    std::vector<FormField> fields;

    auto form = doc->form();
    if (!form) return fields;

    auto fieldList = form->fields();
    for (auto* field : fieldList) {
        // Poppler form fields don't have page info easily accessible
        fields.push_back(convertFormField(field, 0));
    }

    return fields;
}

void PopplerEngine::setFormField(const json& params) {
    throw std::runtime_error("Form field modification not supported by Poppler. Use qpdf engine.");
}

void PopplerEngine::flattenForm(const json& params) {
    throw std::runtime_error("Form flattening not supported by Poppler. Use qpdf engine.");
}

void PopplerEngine::setPassword(const json& params) {
    throw std::runtime_error("Password setting not supported by Poppler. Use qpdf engine.");
}

void PopplerEngine::removePassword(const json& params) {
    throw std::runtime_error("Password removal not supported by Poppler. Use qpdf engine.");
}

Permissions PopplerEngine::getPermissions(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    Poppler::Document* doc = getDocument(handle);
    if (!doc) throw std::runtime_error("Invalid document handle");

    Permissions perms;
    // Poppler doesn't expose fine-grained permissions easily
    // Default to all allowed for non-encrypted
    perms.canPrint = !doc->isEncrypted();
    perms.canModify = !doc->isEncrypted();
    perms.canCopy = !doc->isEncrypted();
    perms.canAnnotate = !doc->isEncrypted();
    perms.canFillForms = !doc->isEncrypted();
    perms.canExtract = !doc->isEncrypted();
    perms.canAssemble = !doc->isEncrypted();
    perms.canPrintHighQuality = !doc->isEncrypted();

    return perms;
}

json PopplerEngine::getThumbnail(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    int pageIndex = params.at("pageIndex").get<int>();
    int maxSize = params.value("maxSize", 128);

    Poppler::Document* doc = getDocument(handle);
    if (!doc) throw std::runtime_error("Invalid document handle");

    auto page = doc->page(pageIndex);
    if (!page) throw std::runtime_error("Invalid page index");

    QSizeF pageSize = page->pageSizeF();
    double scale = std::min(maxSize / pageSize.width(), maxSize / pageSize.height());

    RenderOptions opts;
    opts.scale = scale;
    opts.dpi = 72.0 * scale;

    QImage thumb = renderPageToImage(page, opts);
    std::string base64 = imageToBase64(thumb, "png");

    return {{"image", base64}, {"width", thumb.width()}, {"height", thumb.height()}};
}

json PopplerEngine::getThumbnails(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    int maxSize = params.value("maxSize", 128);

    Poppler::Document* doc = getDocument(handle);
    if (!doc) throw std::runtime_error("Invalid document handle");

    json result = json::array();

    for (int i = 0; i < doc->numPages(); ++i) {
        auto page = doc->page(i);
        if (!page) continue;

        QSizeF pageSize = page->pageSizeF();
        double scale = std::min(maxSize / pageSize.width(), maxSize / pageSize.height());

        RenderOptions opts;
        opts.scale = scale;
        opts.dpi = 72.0 * scale;

        QImage thumb = renderPageToImage(page, opts);
        std::string base64 = imageToBase64(thumb, "png");

        result.push_back({{"pageIndex", i}, {"image", base64}, {"width", thumb.width()}, {"height", thumb.height()}});
    }

    return {{"thumbnails", result}};
}

// =============================================================================
// Helpers
// =============================================================================

Poppler::Document* PopplerEngine::getDocument(DocumentHandle handle) {
    std::lock_guard<std::mutex> lock(docsMutex_);
    auto it = documents_.find(handle.value);
    if (it == documents_.end()) return nullptr;
    return it->second->doc.get();
}

Poppler::Page* PopplerEngine::getPage(PageHandle handle) {
    Poppler::Document* doc = getDocument(handle.doc);
    if (!doc) return nullptr;
    if (handle.index < 0 || handle.index >= doc->numPages()) return nullptr;
    return doc->page(handle.index);
}

QImage PopplerEngine::renderPageToImage(Poppler::Page* page, const RenderOptions& options) {
    QSizeF pageSize = page->pageSizeF();

    int width = static_cast<int>(pageSize.width() * options.scale);
    int height = static_cast<int>(pageSize.height() * options.scale);

    if (options.clipRect.w > 0 && options.clipRect.h > 0) {
        width = static_cast<int>(options.clipRect.w * options.scale);
        height = static_cast<int>(options.clipRect.h * options.scale);
    }

    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, options.antialias);
    painter.setRenderHint(QPainter::TextAntialiasing, options.textAntialias);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (options.rotation != 0) {
        painter.translate(width / 2.0, height / 2.0);
        painter.rotate(options.rotation);
        painter.translate(-width / 2.0, -height / 2.0);
    }

    page->renderToPainter(&painter, options.clipRect.isEmpty() ? QRectF() : QRectF(options.clipRect.x, options.clipRect.y, options.clipRect.w, options.clipRect.h));

    return image;
}

std::string PopplerEngine::imageToBase64(const QImage& image, const std::string& format) {
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);

    QString fmt = QString::fromStdString(format).toUpper();
    if (fmt == "JPG") fmt = "JPEG";

    image.save(&buffer, fmt.toLatin1().constData());
    return ba.toBase64().toStdString();
}

Annotation PopplerEngine::convertAnnotation(Poppler::Annotation* annot, int pageIndex) {
    Annotation a;
    a.id = reinterpret_cast<uintptr_t>(annot); // Use pointer as ID
    a.pageIndex = pageIndex;

    QRectF rect = annot->boundary();
    a.rect = {rect.x(), rect.y(), rect.width(), rect.height()};

    a.contents = annot->contents().toStdString();
    a.author = QString().toStdString(); // Poppler doesn't expose author easily
    a.color = annot->color().rgba();
    a.opacity = 1.0; // Poppler doesn't expose opacity

    switch (annot->subType()) {
        case Poppler::Annotation::AHighlight: a.type = Annotation::Type::Highlight; break;
        case Poppler::Annotation::AUnderline: a.type = Annotation::Type::Underline; break;
        case Poppler::Annotation::AStrikeOut: a.type = Annotation::Type::StrikeOut; break;
        case Poppler::Annotation::ASquiggly: a.type = Annotation::Type::Squiggly; break;
        case Poppler::Annotation::AFreeText: a.type = Annotation::Type::FreeText; break;
        case Poppler::Annotation::ALine: a.type = Annotation::Type::Line; break;
        case Poppler::Annotation::ASquare: a.type = Annotation::Type::Square; break;
        case Poppler::Annotation::ACircle: a.type = Annotation::Type::Circle; break;
        case Poppler::Annotation::AInk: a.type = Annotation::Type::Ink; break;
        case Poppler::Annotation::AStamp: a.type = Annotation::Type::Stamp; break;
        case Poppler::Annotation::ACaret: a.type = Annotation::Type::Caret; break;
        case Poppler::Annotation::ALink: a.type = Annotation::Type::Link; break;
        case Poppler::Annotation::AWidget: a.type = Annotation::Type::Widget; break;
        default: a.type = Annotation::Type::Highlight; break;
    }

    // Type-specific properties
    if (auto* highlight = dynamic_cast<Poppler::HighlightAnnotation*>(annot)) {
        a.properties["quadPoints"] = json::array();
        // Poppler doesn't expose quad points easily
    } else if (auto* link = dynamic_cast<Poppler::LinkAnnotation*>(annot)) {
        a.properties["action"] = link->action() ? link->action()->toString().toStdString() : "";
        a.properties["destination"] = link->destination() ? link->destination()->toString().toStdString() : "";
    }

    return a;
}

FormField PopplerEngine::convertFormField(Poppler::FormField* field, int pageIndex) {
    FormField f;
    f.name = field->fullyQualifiedName().toStdString();
    f.pageIndex = pageIndex;

    switch (field->type()) {
        case Poppler::FormField::FieldText:
            f.type = "text";
            break;
        case Poppler::FormField::FieldButton:
            f.type = "button";
            break;
        case Poppler::FormField::FieldChoice:
            f.type = "choice";
            break;
        case Poppler::FormField::FieldSignature:
            f.type = "signature";
            break;
        default:
            f.type = "unknown";
    }

    f.value = field->value().toString().toStdString();
    f.readOnly = field->isReadOnly();

    QRectF rect = field->boundingBox();
    f.rect = {rect.x(), rect.y(), rect.width(), rect.height()};

    return f;
}

} // namespace pdfpro::poppler