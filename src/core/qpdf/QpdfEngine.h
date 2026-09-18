#pragma once

#include "../ipc/IpcProtocol.h"
#include "../ipc/IpcTransport.h"
#include <qpdf/qpdf-c.h>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <string>

namespace pdfpro::qpdf {

using namespace pdfpro::ipc;

class QpdfEngine : public JsonRpcServer {
public:
    QpdfEngine();
    ~QpdfEngine() override;

    bool initialize();

private:
    // Document lifecycle
    DocumentHandle openDocument(const json& params);
    void closeDocument(const json& params);
    json saveDocument(const json& params);
    json saveDocumentAs(const json& params);
    DocumentInfo getDocumentInfo(const json& params);
    int getPageCount(const json& params);

    // Page management (write operations)
    void insertPage(const json& params);
    void removePage(const json& params);
    void movePage(const json& params);
    void copyPage(const json& params);
    void rotatePage(const json& params);

    // Page rendering (basic - for thumbnails)
    json renderPage(const json& params);
    SizeF getPageSize(const json& params);
    int getPageRotation(const json& params);

    // Text extraction (basic)
    PageText getPageText(const json& params);
    std::vector<SearchResult> searchText(const json& params);

    // Annotations (write)
    uint64_t addAnnotation(const json& params);
    void updateAnnotation(const json& params);
    void removeAnnotation(const json& params);

    // Forms (write)
    void setFormField(const json& params);
    void flattenForm(const json& params);

    // Security
    void setPassword(const json& params);
    void removePassword(const json& params);
    Permissions getPermissions(const json& params);

    // Operations
    json mergeDocuments(const json& params);
    json splitDocument(const json& params);
    json exportImages(const json& params);
    json redact(const json& params);

    // Thumbnails
    json getThumbnail(const json& params);
    json getThumbnails(const json& params);

    // Helpers
    qpdf_data getQpdf(DocumentHandle handle);
    std::string qpdfErrorToString(qpdf_error_code_e code);
    json pageToJson(qpdf_data qpdf, int pageIndex);
    std::string imageToBase64(const unsigned char* data, size_t width, size_t height, const std::string& format);

    struct DocumentData {
        qpdf_data qpdf = nullptr;
        std::string filePath;
        std::string password;
        bool modified = false;
        uint64_t nextAnnotId = 1;
    };

    std::mutex docsMutex_;
    std::unordered_map<uint64_t, std::unique_ptr<DocumentData>> documents_;
    uint64_t nextDocHandle_ = 1;
};

} // namespace pdfpro::qpdf