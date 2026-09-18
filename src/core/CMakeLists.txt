# Core Module

# IPC Library
add_library(pdfpro_ipc STATIC
    ipc/IpcProtocol.h
    ipc/IpcTransport.h
    ipc/IpcTransport.cpp
)

target_include_directories(pdfpro_ipc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/ipc
)

target_link_libraries(pdfpro_ipc PUBLIC
)
if(TARGET nlohmann_json)
    target_link_libraries(pdfpro_ipc PUBLIC nlohmann_json)
endif()
if(TARGET spdlog)
    target_link_libraries(pdfpro_ipc PUBLIC spdlog)
endif()

# Poppler Engine (separate process) - optional
if(Poppler_FOUND)
    add_executable(pdfpro-poppler-engine
        poppler/PopplerEngine.cpp
        poppler/main.cpp
    )

    target_include_directories(pdfpro-poppler-engine PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ipc
        ${CMAKE_CURRENT_SOURCE_DIR}/poppler
    )

    target_link_libraries(pdfpro-poppler-engine PRIVATE
        pdfpro_ipc
        Poppler::Qt6
        Poppler::Cpp
        Poppler::Glib
        Qt6::Core
        Qt6::Gui
    )
    if(TARGET spdlog)
        target_link_libraries(pdfpro-poppler-engine PRIVATE spdlog)
    endif()

    install(TARGETS pdfpro-poppler-engine
        RUNTIME DESTINATION bin
    )
else()
    message(WARNING "Skipping Poppler engine build - Poppler not found")
endif()

# QPDF Engine (separate process) - optional (handle different target names: qpdf::qpdf vs qpdf::libqpdf vs PkgConfig)
if(qpdf_FOUND)
    add_executable(pdfpro-qpdf-engine
        qpdf/QpdfEngine.cpp
        qpdf/main.cpp
    )

    target_include_directories(pdfpro-qpdf-engine PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ipc
        ${CMAKE_CURRENT_SOURCE_DIR}/qpdf
    )

    if(TARGET qpdf::qpdf)
        target_link_libraries(pdfpro-qpdf-engine PRIVATE pdfpro_ipc qpdf::qpdf Qt6::Core)
    elseif(TARGET qpdf::libqpdf)
        target_link_libraries(pdfpro-qpdf-engine PRIVATE pdfpro_ipc qpdf::libqpdf Qt6::Core)
    elseif(QPDF_LIBRARIES)
        target_link_libraries(pdfpro-qpdf-engine PRIVATE pdfpro_ipc ${QPDF_LIBRARIES} Qt6::Core)
        target_include_directories(pdfpro-qpdf-engine PRIVATE ${QPDF_INCLUDE_DIRS})
    else()
        # Fallback: try pkg-config
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(QPDF_PKG libqpdf)
            if(QPDF_PKG_FOUND)
                target_link_libraries(pdfpro-qpdf-engine PRIVATE pdfpro_ipc ${QPDF_PKG_LIBRARIES} Qt6::Core)
                target_include_directories(pdfpro-qpdf-engine PRIVATE ${QPDF_PKG_INCLUDE_DIRS})
            else()
                target_link_libraries(pdfpro-qpdf-engine PRIVATE pdfpro_ipc Qt6::Core)
            endif()
        else()
            target_link_libraries(pdfpro-qpdf-engine PRIVATE pdfpro_ipc Qt6::Core)
        endif()
    endif()
    if(TARGET spdlog)
        target_link_libraries(pdfpro-qpdf-engine PRIVATE spdlog)
    endif()

    install(TARGETS pdfpro-qpdf-engine
        RUNTIME DESTINATION bin
    )
else()
    message(WARNING "Skipping Qpdf engine build - qpdf not found")
endif()

# Combined core library for UI
add_library(pdfpro_core INTERFACE)

target_link_libraries(pdfpro_core INTERFACE
    pdfpro_ipc
    Qt6::Core
    Qt6::Gui
)
if(TARGET nlohmann_json)
    target_link_libraries(pdfpro_core INTERFACE nlohmann_json)
endif()
if(TARGET spdlog)
    target_link_libraries(pdfpro_core INTERFACE spdlog)
endif()

# Install engines (conditional)
if(TARGET pdfpro-poppler-engine)
    install(TARGETS pdfpro-poppler-engine RUNTIME DESTINATION bin)
endif()
if(TARGET pdfpro-qpdf-engine)
    install(TARGETS pdfpro-qpdf-engine RUNTIME DESTINATION bin)
endif()