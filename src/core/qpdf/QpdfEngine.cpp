#include "QpdfEngine.h"
#include <spdlog/spdlog.h>
#include <qpdf/QPDF.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QUtil.hh>
#include <memory>
#include <vector>
#include <string>

namespace pdfpro::qpdf {

using namespace pdfpro::ipc;

QpdfEngine::QpdfEngine()
    : JsonRpcServer(std::make_unique<StdioTransport>()) {

    // Document lifecycle
    registerMethod(methods::OPEN_DOCUMENT, [this](const json& p) -> json { return openDocument(p); });
    registerMethod(methods::CLOSE_DOCUMENT, [this](const json& p) -> json { closeDocument(p); return json{}; });
    registerMethod(methods::SAVE_DOCUMENT, [this](const json& p) -> json { return saveDocument(p); });
    registerMethod(methods::SAVE_DOCUMENT_AS, [this](const json& p) -> json { return saveDocumentAs(p); });
    registerMethod(methods::GET_DOCUMENT_INFO, [this](const json& p) -> json { return getDocumentInfo(p); });
    registerMethod(methods::GET_PAGE_COUNT, [this](const json& p) -> json { return getPageCount(p); });

    // Page management
    registerMethod(methods::INSERT_PAGE, [this](const json& p) -> json { insertPage(p); return json{}; });
    registerMethod(methods::REMOVE_PAGE, [this](const json& p) -> json { removePage(p); return json{}; });
    registerMethod(methods::MOVE_PAGE, [this](const json& p) -> json { movePage(p); return json{}; });
    registerMethod(methods::COPY_PAGE, [this](const json& p) -> json { copyPage(p); return json{}; });
    registerMethod(methods::SET_PAGE_ROTATION, [this](const json& p) -> json { rotatePage(p); return json{}; });

    // Page rendering (basic)
    registerMethod(methods::RENDER_PAGE, [this](const json& p) -> json { return renderPage(p); });
    registerMethod(methods::GET_PAGE_SIZE, [this](const json& p) -> json { return getPageSize(p); });
    registerMethod(methods::GET_PAGE_ROTATION, [this](const json& p) -> json { return getPageRotation(p); });

    // Text (basic)
    registerMethod(methods::GET_PAGE_TEXT, [this](const json& p) -> json { return getPageText(p); });
    registerMethod(methods::SEARCH_TEXT, [this](const json& p) -> json { return searchText(p); });

    // Annotations
    registerMethod(methods::ADD_ANNOTATION, [this](const json& p) -> json { return addAnnotation(p); });
    registerMethod(methods::UPDATE_ANNOTATION, [this](const json& p) -> json { updateAnnotation(p); return json{}; });
    registerMethod(methods::REMOVE_ANNOTATION, [this](const json& p) -> json { removeAnnotation(p); return json{}; });

    // Forms
    registerMethod(methods::SET_FORM_FIELD, [this](const json& p) -> json { setFormField(p); return json{}; });
    registerMethod(methods::FLATTEN_FORM, [this](const json& p) -> json { flattenForm(p); return json{}; });

    // Security
    registerMethod(methods::SET_PASSWORD, [this](const json& p) -> json { setPassword(p); return json{}; });
    registerMethod(methods::REMOVE_PASSWORD, [this](const json& p) -> json { removePassword(p); return json{}; });
    registerMethod(methods::GET_PERMISSIONS, [this](const json& p) -> json { return getPermissions(p); });

    // Operations
    registerMethod(methods::MERGE_DOCUMENTS, [this](const json& p) -> json { return mergeDocuments(p); });
    registerMethod(methods::SPLIT_DOCUMENT, [this](const json& p) -> json { return splitDocument(p); });
    registerMethod(methods::EXPORT_IMAGES, [this](const json& p) -> json { return exportImages(p); });
    registerMethod(methods::REDACT, [this](const json& p) -> json { return redact(p); });

    // Thumbnails
    registerMethod(methods::GET_THUMBNAIL, [this](const json& p) -> json { return getThumbnail(p); });
    registerMethod(methods::GET_THUMBNAILS, [this](const json& p) -> json { return getThumbnails(p); });
}

QpdfEngine::~QpdfEngine() {
    std::lock_guard<std::mutex> lock(docsMutex_);
    for (auto& [_, data] : documents_) {
        if (data->qpdf) {
            qpdf_data_release(data->qpdf);
        }
    }
    documents_.clear();
}

bool QpdfEngine::initialize() {
    spdlog::info("QpdfEngine initializing...");
    return start();
}

DocumentHandle QpdfEngine::openDocument(const json& params) {
    std::string filePath = params.at("filePath").get<std::string>();
    std::string password = params.value("password", "");

    qpdf_data qpdf = qpdf_init();
    if (!qpdf) {
        throw std::runtime_error("Failed to initialize qpdf");
    }

    qpdf_error_code_e err = qpdf_read(qpdf, filePath.c_str(), password.empty() ? nullptr : password.c_str());
    if (err != QPDF_SUCCESS) {
        qpdf_data_release(qpdf);
        throw std::runtime_error("Failed to read PDF: " + qpdfErrorToString(err));
    }

    DocumentHandle handle;
    {
        std::lock_guard<std::mutex> lock(docsMutex_);
        handle.value = nextDocHandle_++;
        auto data = std::make_unique<DocumentData>();
        data->qpdf = qpdf;
        data->filePath = filePath;
        data->password = password;
        documents_[handle.value] = std::move(data);
    }

    spdlog::info("Opened document: {} (handle: {})", filePath, handle.value);
    return handle;
}

void QpdfEngine::closeDocument(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    std::lock_guard<std::mutex> lock(docsMutex_);
    auto it = documents_.find(handle.value);
    if (it != documents_.end()) {
        if (it->second->qpdf) {
            qpdf_data_release(it->second->qpdf);
        }
        documents_.erase(it);
    }
}

json QpdfEngine::saveDocument(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    auto data = getDocumentData(handle);
    if (!data) throw std::runtime_error("Invalid document handle");

    qpdf_error_code_e err = qpdf_write(data->qpdf, data->filePath.c_str(), nullptr, 0);
    if (err != QPDF_SUCCESS) {
        return {{"success", false}, {"error", qpdfErrorToString(err)}};
    }

    data->modified = false;
    return {{"success", true}};
}

json QpdfEngine::saveDocumentAs(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    std::string newPath = params.at("filePath").get<std::string>();
    std::string password = params.value("password", "");

    auto data = getDocumentData(handle);
    if (!data) throw std::runtime_error("Invalid document handle");

    QPDFWriter writer(*reinterpret_cast<QPDF*>(data->qpdf), newPath.c_str());
    if (!password.empty()) {
        writer.setStaticID(true);
        writer.setEncryptionParameters(
            password.c_str(), password.c_str(),
            qpdf_e_aes_256, qpdf_c_default,
            false, false, false, false, false, false, false, false
        );
    }
    writer.write();

    data->filePath = newPath;
    data->password = password;
    data->modified = false;

    return {{"success", true}, {"filePath", newPath}};
}

DocumentInfo QpdfEngine::getDocumentInfo(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    qpdf_data qpdf = getQpdf(handle);
    if (!qpdf) throw std::runtime_error("Invalid document handle");

    QPDF* pdf = reinterpret_cast<QPDF*>(qpdf);
    DocumentInfo info;
    info.pageCount = pdf->getAllPages().size();
    info.isEncrypted = pdf->isEncrypted();
    info.isLinearized = pdf->isLinearized();

    // Get metadata from Info dictionary
    auto infoDict = pdf->getTrailer()->getKey("/Info");
    if (infoDict && infoDict->isDictionary()) {
        auto getStr = [&](const char* key) -> std::string {
            auto val = infoDict->getKey(key);
            return val && val->isString() ? val->getUTF8Value() : "";
        };
        info.title = getStr("/Title");
        info.author = getStr("/Author");
        info.subject = getStr("/Subject");
        info.keywords = getStr("/Keywords");
        info.creator = getStr("/Creator");
        info.producer = getStr("/Producer");
        info.creationDate = getStr("/CreationDate");
        info.modificationDate = getStr("/ModDate");
    }

    // PDF version
    info.pdfVersion = "1." + std::to_string(pdf->getPDFVersion().getVersion());

    // First page size
    auto pages = pdf->getAllPages();
    if (!pages.empty()) {
        auto page = pages[0];
        auto mediabox = page->getMediaBox();
        info.pageSize = {mediabox.getWidth().getAsDouble(), mediabox.getHeight().getAsDouble()};
    }

    return info;
}

int QpdfEngine::getPageCount(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    qpdf_data qpdf = getQpdf(handle);
    if (!qpdf) throw std::runtime_error("Invalid document handle");

    QPDF* pdf = reinterpret_cast<QPDF*>(qpdf);
    return pdf->getAllPages().size();
}

void QpdfEngine::insertPage(const json& params) {
    // TODO: Implement page insertion from another document
    throw std::runtime_error("Not implemented yet");
}

void QpdfEngine::removePage(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    int pageIndex = params.at("pageIndex").get<int>();

    qpdf_data qpdf = getQpdf(handle);
    if (!qpdf) throw std::runtime_error("Invalid document handle");

    QPDF* pdf = reinterpret_cast<QPDF*>(qpdf);
    auto pages = pdf->getAllPages();
    if (pageIndex < 0 || pageIndex >= (int)pages.size()) {
        throw std::runtime_error("Invalid page index");
    }

    pdf->removePage(pages.begin() + pageIndex);
    getDocumentData(handle)->modified = true;
}

void QpdfEngine::movePage(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    int fromIndex = params.at("fromIndex").get<int>();
    int toIndex = params.at("toIndex").get<int>();

    qpdf_data qpdf = getQpdf(handle);
    if (!qpdf) throw std::runtime_error("Invalid document handle");

    QPDF* pdf = reinterpret_cast<QPDF*>(qpdf);
    auto pages = pdf->getAllPages();
    if (fromIndex < 0 || fromIndex >= (int)pages.size() ||
        toIndex < 0 || toIndex > (int)pages.size()) {
        throw std::runtime_error("Invalid page index");
    }

    auto page = pages[fromIndex];
    pages.erase(pages.begin() + fromIndex);
    if (toIndex >= (int)pages.size()) {
        pages.push_back(page);
    } else {
        pages.insert(pages.begin() + toIndex, page);
    }
    pdf->replacePages(pages);
    getDocumentData(handle)->modified = true;
}

void QpdfEngine::copyPage(const json& params) {
    // TODO: Copy page to clipboard or another document
    throw std::runtime_error("Not implemented yet");
}

void QpdfEngine::rotatePage(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    int pageIndex = params.at("pageIndex").get<int>();
    int rotation = params.at("rotation").get<int>(); // 0, 90, 180, 270

    qpdf_data qpdf = getQpdf(handle);
    if (!qpdf) throw std::runtime_error("Invalid document handle");

    QPDF* pdf = reinterpret_cast<QPDF*>(qpdf);
    auto pages = pdf->getAllPages();
    if (pageIndex < 0 || pageIndex >= (int)pages.size()) {
        throw std::runtime_error("Invalid page index");
    }

    auto page = pages[pageIndex];
    int currentRot = page->getRotate().getValue();
    int newRot = (currentRot + rotation) % 360;
    if (newRot < 0) newRot += 360;
    page->setRotate(newRot);
    getDocumentData(handle)->modified = true;
}

json QpdfEngine::renderPage(const json& params) {
    // QPDF doesn't render - delegate to Poppler for rendering
    return {{"success", false}, {"error", "Rendering not supported by qpdf. Use Poppler engine."}};
}

SizeF QpdfEngine::getPageSize(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    int pageIndex = params.at("pageIndex").get<int>();

    qpdf_data qpdf = getQpdf(handle);
    if (!qpdf) throw std::runtime_error("Invalid document handle");

    QPDF* pdf = reinterpret_cast<QPDF*>(qpdf);
    auto pages = pdf->getAllPages();
    if (pageIndex < 0 || pageIndex >= (int)pages.size()) {
        throw std::runtime_error("Invalid page index");
    }

    auto page = pages[pageIndex];
    auto mediabox = page->getMediaBox();
    return {mediabox.getWidth().getAsDouble(), mediabox.getHeight().getAsDouble()};
}

int QpdfEngine::getPageRotation(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    int pageIndex = params.at("pageIndex").get<int>();

    qpdf_data qpdf = getQpdf(handle);
    if (!qpdf) throw std::runtime_error("Invalid document handle");

    QPDF* pdf = reinterpret_cast<QPDF*>(qpdf);
    auto pages = pdf->getAllPages();
    if (pageIndex < 0 || pageIndex >= (int)pages.size()) {
        throw std::runtime_error("Invalid page index");
    }

    return pages[pageIndex]->getRotate().getValue();
}

PageText QpdfEngine::getPageText(const json& params) {
    // QPDF doesn't extract text well - delegate to Poppler
    return {{"text", ""}, {"charBounds", json::array()}, {"wordBounds", json::array()}, {"lineBounds", json::array()}, {"blockBounds", json::array()}};
}

std::vector<SearchResult> QpdfEngine::searchText(const json& params) {
    // Delegate to Poppler
    return {};
}

uint64_t QpdfEngine::addAnnotation(const json& params) {
    // TODO: Add annotation using QPDF's annotation API
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    auto data = getDocumentData(handle);
    if (!data) throw std::runtime_error("Invalid document handle");
    return data->nextAnnotId++;
}

void QpdfEngine::updateAnnotation(const json& params) {
    // TODO
}

void QpdfEngine::removeAnnotation(const json& params) {
    // TODO
}

void QpdfEngine::setFormField(const json& params) {
    // TODO: Set form field value
}

void QpdfEngine::flattenForm(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    qpdf_data qpdf = getQpdf(handle);
    if (!qpdf) throw std::runtime_error("Invalid document handle");

    QPDF* pdf = reinterpret_cast<QPDF*>(qpdf);
    pdf->flattenAnnotations();
    getDocumentData(handle)->modified = true;
}

void QpdfEngine::setPassword(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    std::string userPass = params.value("userPassword", "");
    std::string ownerPass = params.value("ownerPassword", "");

    qpdf_data qpdf = getQpdf(handle);
    if (!qpdf) throw std::runtime_error("Invalid document handle");

    QPDF* pdf = reinterpret_cast<QPDF*>(qpdf);
    if (!userPass.empty() || !ownerPass.empty()) {
        pdf->setEncryptionParameters(
            userPass.empty() ? ownerPass.c_str() : userPass.c_str(),
            ownerPass.empty() ? userPass.c_str() : ownerPass.c_str(),
            qpdf_e_aes_256, qpdf_c_default,
            false, false, false, false, false, false, false, false
        );
    }
    getDocumentData(handle)->modified = true;
}

void QpdfEngine::removePassword(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    std::string password = params.at("password").get<std::string>();

    qpdf_data qpdf = getQpdf(handle);
    if (!qpdf) throw std::runtime_error("Invalid document handle");

    QPDF* pdf = reinterpret_cast<QPDF*>(qpdf);
    if (pdf->isEncrypted()) {
        pdf->setEncryptionParameters("", "", qpdf_e_none, qpdf_c_default, false, false, false, false, false, false, false, false);
    }
    getDocumentData(handle)->modified = true;
}

Permissions QpdfEngine::getPermissions(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    qpdf_data qpdf = getQpdf(handle);
    if (!qpdf) throw std::runtime_error("Invalid document handle");

    QPDF* pdf = reinterpret_cast<QPDF*>(qpdf);
    Permissions perms;
    if (pdf->isEncrypted()) {
        auto enc = pdf->getEncryption();
        perms.canPrint = enc->canPrint();
        perms.canModify = enc->canModify();
        perms.canCopy = enc->canCopy();
        perms.canAnnotate = enc->canAddOrModifyAnnotations();
        perms.canFillForms = enc->canFillForms();
        perms.canExtract = enc->canExtractContent();
        perms.canAssemble = enc->canAssembleDocument();
        perms.canPrintHighQuality = enc->canPrintHighResolution();
    } else {
        perms = Permissions{}; // all true by default
    }
    return perms;
}

json QpdfEngine::mergeDocuments(const json& params) {
    auto handles = params.at("handles").get<std::vector<DocumentHandle>>();
    std::string outputPath = params.at("outputPath").get<std::string>();

    if (handles.size() < 2) {
        return {{"success", false}, {"error", "Need at least 2 documents to merge"}};
    }

    QPDF merged;
    merged.emptyPDF();

    for (size_t i = 0; i < handles.size(); ++i) {
        auto data = getDocumentData(handles[i]);
        if (!data) {
            return {{"success", false}, {"error", "Invalid document handle at index " + std::to_string(i)}};
        }
        QPDF* src = reinterpret_cast<QPDF*>(data->qpdf);
        merged.appendPages(*src);
    }

    QPDFWriter writer(merged, outputPath.c_str());
    writer.write();

    return {{"success", true}, {"outputPath", outputPath}, {"pageCount", (int)merged.getAllPages().size()}};
}

json QpdfEngine::splitDocument(const json& params) {
    DocumentHandle handle = params.at("handle").get<DocumentHandle>();
    std::string outputDir = params.value("outputDir", "");
    std::string pattern = params.value("pattern", "page_%d.pdf");

    auto data = getDocumentData(handle);
    if (!data) throw std::runtime_error("Invalid document handle");

    QPDF* src = reinterpret_cast<QPDF*>(data->qpdf);
    auto pages = src->getAllPages();

    json results = json::array();
    for (size_t i = 0; i < pages.size(); ++i) {
        QPDF single;
        single.emptyPDF();
        single.addPage(pages[i], false);

        char buf[256];
        snprintf(buf, sizeof(buf), pattern.c_str(), (int)(i + 1));
        std::string outputPath = outputDir.empty() ? buf : (outputDir + "/" + buf);

        QPDFWriter writer(single, outputPath.c_str());
        writer.write();

        results.push_back({{"page", (int)(i + 1)}, {"path", outputPath}});
    }

    return {{"success", true}, {"files", results}};
}

json QpdfEngine::exportImages(const json& params) {
    // QPDF doesn't extract images easily - would need additional library
    return {{"success", false}, {"error", "Not implemented"}};
}

json QpdfEngine::redact(const json& params) {
    // TODO: Implement redaction using QPDF content stream manipulation
    return {{"success", false}, {"error", "Not implemented yet"}};
}

json QpdfEngine::getThumbnail(const json& params) {
    return {{"success", false}, {"error", "Thumbnails not supported by qpdf. Use Poppler."}};
}

json QpdfEngine::getThumbnails(const json& params) {
    return {{"success", false}, {"error", "Thumbnails not supported by qpdf. Use Poppler."}};
}

// Helpers

QpdfEngine::DocumentData* QpdfEngine::getDocumentData(DocumentHandle handle) {
    std::lock_guard<std::mutex> lock(docsMutex_);
    auto it = documents_.find(handle.value);
    return it != documents_.end() ? it->second.get() : nullptr;
}

qpdf_data QpdfEngine::getQpdf(DocumentHandle handle) {
    auto data = getDocumentData(handle);
    return data ? data->qpdf : nullptr;
}

std::string QpdfEngine::qpdfErrorToString(qpdf_error_code_e code) {
    const char* msg = qpdf_error_code_to_string(code);
    return msg ? msg : "Unknown qpdf error";
}

json QpdfEngine::pageToJson(qpdf_data qpdf, int pageIndex) {
    QPDF* pdf = reinterpret_cast<QPDF*>(qpdf);
    auto pages = pdf->getAllPages();
    if (pageIndex < 0 || pageIndex >= (int)pages.size()) return {};

    auto page = pages[pageIndex];
    auto mediabox = page->getMediaBox();
    return {{"width", mediabox.getWidth().getAsDouble()}, {"height", mediabox.getHeight().getAsDouble()},
            {"rotation", page->getRotate().getValue()}};
}

std::string QpdfEngine::imageToBase64(const unsigned char*, size_t, size_t, const std::string&) {
    return "";
}

} // namespace pdfpro::qpdf