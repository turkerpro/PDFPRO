#pragma once

#include "../ipc/IpcProtocol.h"
#include "../ipc/IpcTransport.h"
#include <poppler/qt6/document.h>
#include <poppler/qt6/page.h>
#include <poppler/qt6/annotation.h>
#include <poppler/qt6/form.h>
#include <poppler/qt6/link.h>
#include <poppler/qt6/image.h>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace pdfpro::poppler {

using namespace pdfpro::ipc;

class PopplerEngine : public JsonRpcServer {
public:
    PopplerEngine();
    ~PopplerEngine() override;

    bool initialize();

private:
    // Document management
    DocumentHandle openDocument(const json& params);
    void closeDocument(const json& params);
    json saveDocument(const json& params);
    json saveDocumentAs(const json& params);
    DocumentInfo getDocumentInfo(const json& params);
    int getPageCount(const json& params);

    // Page rendering
    json renderPage(const json& params);
    json renderPageTiles(const json& params);
    SizeF getPageSize(const json& params);
    int getPageRotation(const json& params);
    void setPageRotation(const json& params);

    // Page management
    void insertPage(const json& params);
    void removePage(const json& params);
    void movePage(const json& params);
    void copyPage(const json& params);

    // Text extraction
    PageText getPageText(const json& params);
    std::vector<RectF> getTextBounds(const json& params);
    std::vector<SearchResult> searchText(const json& params);
    json getSelection(const json& params);

    // Annotations
    std::vector<Annotation> getAnnotations(const json& params);
    uint64_t addAnnotation(const json& params);
    void updateAnnotation(const json& params);
    void removeAnnotation(const json& params);

    // Forms
    std::vector<FormField> getFormFields(const json& params);
    void setFormField(const json& params);
    void flattenForm(const json& params);

    // Security
    void setPassword(const json& params);
    void removePassword(const json& params);
    Permissions getPermissions(const json& params);

    // Thumbnails
    json getThumbnail(const json& params);
    json getThumbnails(const json& params);

    // Helpers
    Poppler::Document* getDocument(DocumentHandle handle);
    Poppler::Page* getPage(PageHandle handle);
    QImage renderPageToImage(Poppler::Page* page, const RenderOptions& options);
    std::string imageToBase64(const QImage& image, const std::string& format);
    Annotation convertAnnotation(Poppler::Annotation* annot, int pageIndex);
    FormField convertFormField(Poppler::FormField* field, int pageIndex);

    struct DocumentData {
        std::unique_ptr<Poppler::Document> doc;
        std::string filePath;
        std::string password;
        bool modified = false;
        uint64_t nextAnnotId = 1;
    };

    std::mutex docsMutex_;
    std::unordered_map<uint64_t, std::unique_ptr<DocumentData>> documents_;
    uint64_t nextDocHandle_ = 1;
};

} // namespace pdfpro::poppler