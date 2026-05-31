set(LWZIP_MAC_SOURCES
    main.cpp
    LWMainWindow.cpp
    LWEncodingSwitch.cpp
)

set(LWZIP_MAC_HEADERS
    LWMainWindow.h
    LWEncodingSwitch.h
)

add_executable(lwzip ${LWZIP_MAC_SOURCES} ${LWZIP_MAC_HEADERS})

target_link_libraries(lwzip PRIVATE
    Qt6::Core
    Qt6::Widgets
)

set_target_properties(lwzip PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.lwzip.filemanager"
    MACOSX_BUNDLE_BUNDLE_NAME "LWZip"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
)
