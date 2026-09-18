#include "QpdfEngine.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>
#include <csignal>

namespace pdfpro::qpdf {

static std::unique_ptr<QpdfEngine> g_engine;
static std::atomic<bool> g_shutdown{false};

void signalHandler(int sig) {
    spdlog::info("Received signal {}, shutting down...", sig);
    g_shutdown.store(true);
    if (g_engine) {
        g_engine->stop();
    }
}

} // namespace pdfpro::qpdf

int main(int argc, char* argv[]) {
    using namespace pdfpro::qpdf;

    auto console = spdlog::stdout_color_mt("qpdf");
    console->set_level(spdlog::level::info);
    console->set_pattern("[%H:%M:%S.%e] [%^%l%$] [%n] %v");
    spdlog::set_default_logger(console);

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
#ifdef _WIN32
    std::signal(SIGBREAK, signalHandler);
#endif

    spdlog::info("Starting PDFPRO Qpdf Engine (write operations)");
    spdlog::info("PID: {}", getpid());

    try {
        g_engine = std::make_unique<QpdfEngine>();

        if (!g_engine->initialize()) {
            spdlog::error("Failed to initialize QpdfEngine");
            return 1;
        }

        spdlog::info("QpdfEngine ready, waiting for requests on stdio...");

        while (!g_shutdown.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        spdlog::info("Shutdown complete");
        return 0;
    } catch (const std::exception& e) {
        spdlog::error("Fatal error: {}", e.what());
        return 1;
    } catch (...) {
        spdlog::error("Unknown fatal error");
        return 1;
    }
}