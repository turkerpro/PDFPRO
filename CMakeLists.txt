cmake_minimum_required(VERSION 3.22...3.30)
project(PDFPRO
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "PDF Professional - Acrobat Pro alternative"
)

# =============================================================================
# Policies & C++ Standard
# =============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus /Zc:throwingNew)
    add_link_options(/DEBUG:FULL)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wformat=2)
endif()

# =============================================================================
# Qt6 Setup
# =============================================================================
find_package(Qt6 6.10 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Qml
    Quick
    QuickControls2
    QuickTemplates2
    Network
    Svg
    Test
)

# Optional Qt components
find_package(Qt6Charts QUIET)
find_package(Qt6WebEngine QUIET)
find_package(Qt6WebEngineCore QUIET)
find_package(Qt6WebEngineWidgets QUIET)
find_package(Qt6Dbus QUIET)

# Qt6 tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# =============================================================================
# vcpkg dependencies (Poppler, qpdf, tesseract, nlohmann-json, spdlog)
# =============================================================================
find_package(Poppler QUIET COMPONENTS Qt6 cpp glib)
find_package(qpdf QUIET)
find_package(Tesseract QUIET)

# nlohmann-json - use local copy
add_library(nlohmann_json INTERFACE IMPORTED)
set_target_properties(nlohmann_json PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/third_party"
)
message(STATUS "nlohmann_json from local copy")

# spdlog - use local copy
add_library(spdlog INTERFACE IMPORTED)
set_target_properties(spdlog PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/third_party"
)
message(STATUS "spdlog from local copy")

if(Poppler_FOUND)
    add_definitions(-DPDFPRO_HAVE_POPPLER=1)
    message(STATUS "Poppler found: ${Poppler_VERSION}")
else()
    message(WARNING "Poppler not found - Poppler engine will not build")
endif()

if(qpdf_FOUND)
    add_definitions(-DPDFPRO_HAVE_QPDF=1)
    message(STATUS "qpdf found")
else()
    message(WARNING "qpdf not found - Qpdf engine will not build")
endif()

if(Tesseract_FOUND)
    add_definitions(-DPDFPRO_HAVE_TESSERACT=1)
    message(STATUS "Tesseract found")
else()
    message(WARNING "Tesseract not found - OCR will not be available")
endif()

# Optional: Qt6Charts, WebEngine, DBus
if(Qt6Charts_FOUND)
    add_definitions(-DPDFPRO_HAVE_CHARTS=1)
endif()
if(Qt6WebEngine_FOUND)
    add_definitions(-DPDFPRO_HAVE_WEBENGINE=1)
endif()
if(Qt6Dbus_FOUND)
    add_definitions(-DPDFPRO_HAVE_DBUS=1)
endif()

# =============================================================================
# Platform-specific settings
# =============================================================================
if(WIN32)
    add_definitions(-DPDFPRO_WINDOWS=1)
    # Windows-specific: Mica/Acrylic, DPI awareness
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /D_WIN32_WINNT=0x0A00")
    # High DPI
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DQT_AUTO_SCREEN_SCALE_FACTOR=1")
elseif(UNIX AND NOT APPLE)
    add_definitions(-DPDFPRO_LINUX=1)
    # Linux: Wayland/X11 detection
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND wayland-client)
    if(WAYLAND_FOUND)
        add_definitions(-DPDFPRO_WAYLAND=1)
    endif()
endif()

# =============================================================================
# Output directories
# =============================================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# =============================================================================
# Subdirectories
# =============================================================================
add_subdirectory(src/core)
add_subdirectory(src/ui)
# add_subdirectory(tests)  # TODO: add tests

# =============================================================================
# Install targets (for packaging)
# =============================================================================
install(TARGETS pdfpro
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

install(DIRECTORY resources/
    DESTINATION share/pdfpro
    FILES_MATCHING PATTERN "*.qml" PATTERN "*.ttf" PATTERN "*.svg"
)

# =============================================================================
# CPack for packaging (NSIS on Windows, DEB on Linux)
# =============================================================================
set(CPACK_PACKAGE_NAME "PDFPRO")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "PDF Professional - Acrobat Pro alternative")
set(CPACK_PACKAGE_VENDOR "Turker")
set(CPACK_PACKAGE_CONTACT "turker@example.com")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64/PDFPRO")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/resources/icons/app.ico")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_CREATE_DESKTOP_LINKS "pdfpro")
else()
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Turker <turker@example.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
endif()

include(CPack)