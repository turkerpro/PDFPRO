#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFontDatabase>
#include <QIcon>
#include <QDir>
#include <QQuickStyle>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "AppState.h"

int main(int argc, char* argv[]) {
    // High DPI scaling is automatic in Qt 6
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QGuiApplication app(argc, argv);
    app.setApplicationName("PDFPRO");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("Turker");
    app.setOrganizationDomain("pdfpro.app");
    app.setWindowIcon(QIcon(":/icons/app.svg"));

    // Setup logging
    auto console = spdlog::stdout_color_mt("pdfpro");
    console->set_level(spdlog::level::info);
    console->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
    spdlog::set_default_logger(console);

    spdlog::info("Starting PDFPRO v{}", qApp->applicationVersion().toStdString());

    // Load bundled fonts
    QFontDatabase::addApplicationFont(":/fonts/Inter-VariableFont.ttf");
    QFontDatabase::addApplicationFont(":/fonts/JetBrainsMono-VariableFont.ttf");

    // Set default font
    QFont defaultFont("Inter", 10);
    defaultFont.setHintingPreference(QFont::PreferFullHinting);
    app.setFont(defaultFont);

    // Use Material style for Qt Quick Controls 2 (clean, modern)
    QQuickStyle::setStyle("Material");

    // Create AppState (singleton-like)
    pdfpro::ui::AppState appState;
    if (!appState.startEngines()) {
        spdlog::error("Failed to start PDF engines");
        return 1;
    }

    // QML Engine
    QQmlApplicationEngine engine;

    // Expose AppState to QML
    engine.rootContext()->setContextProperty("appState", &appState);

    // Add import paths
    engine.addImportPath(":/qml");
    engine.addImportPath("qrc:/qml");

    // Load main QML
    const QUrl url(QStringLiteral("qrc:/qml/MainWindow.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl) {
                spdlog::error("Failed to load QML: {}", url.toString().toStdString());
                QCoreApplication::exit(-1);
            }
        }, Qt::QueuedConnection);

    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    int result = app.exec();

    spdlog::info("PDFPRO exiting");
    return result;
}