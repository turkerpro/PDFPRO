# UI Module

# AppState
qt_add_executable(pdfpro
    app/main.cpp
    app/AppState.cpp
)

# Add QML resources directly to the executable
# Use absolute paths to avoid confusion with CMAKE_CURRENT_SOURCE_DIR
qt6_add_resources(pdfpro
    PREFIX /qml
    BASE ${CMAKE_SOURCE_DIR}/resources/qml
    FILES
        ${CMAKE_SOURCE_DIR}/resources/qml/MainWindow.qml
        ${CMAKE_SOURCE_DIR}/resources/qml/ToolBar.qml
        ${CMAKE_SOURCE_DIR}/resources/qml/ContextBar.qml
        ${CMAKE_SOURCE_DIR}/resources/qml/SearchBar.qml
        ${CMAKE_SOURCE_DIR}/resources/qml/Sidebar.qml
        ${CMAKE_SOURCE_DIR}/resources/qml/ThumbnailStrip.qml
        ${CMAKE_SOURCE_DIR}/resources/qml/BookmarksPanel.qml
        ${CMAKE_SOURCE_DIR}/resources/qml/AnnotationsPanel.qml
        ${CMAKE_SOURCE_DIR}/resources/qml/SearchResultsPanel.qml
        ${CMAKE_SOURCE_DIR}/resources/qml/PDFViewer.qml
        ${CMAKE_SOURCE_DIR}/resources/qml/Viewport.qml
        ${CMAKE_SOURCE_DIR}/resources/qml/PageView.qml
        ${CMAKE_SOURCE_DIR}/resources/qml/SelectionLayer.qml
        ${CMAKE_SOURCE_DIR}/resources/qml/StatusBar.qml
        ${CMAKE_SOURCE_DIR}/resources/qml/CommandPalette.qml
)

target_link_libraries(pdfpro PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickTemplates2
    Qt6::Network
    Qt6::Svg
    pdfpro_core
)

if(TARGET nlohmann_json)
    target_link_libraries(pdfpro PRIVATE nlohmann_json)
endif()
if(TARGET spdlog)
    target_link_libraries(pdfpro PRIVATE spdlog)
endif()

if(Qt6Charts_FOUND)
    target_link_libraries(pdfpro PRIVATE Qt6::Charts)
endif()
if(Qt6WebEngine_FOUND)
    target_link_libraries(pdfpro PRIVATE Qt6::WebEngine Qt6::WebEngineCore Qt6::WebEngineWidgets)
endif()
if(Qt6Dbus_FOUND)
    target_link_libraries(pdfpro PRIVATE Qt6::DBus)
endif()

# Install
install(TARGETS pdfpro
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Copy QML files for development (optional)
# install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/qml/
#     DESTINATION share/pdfpro/qml
#     FILES_MATCHING PATTERN "*.qml")