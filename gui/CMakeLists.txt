find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)
qt_add_executable(BeamMP-Desktop WIN32 main.cpp LauncherWindow.cpp LauncherWindow.h Localization.h LaunchArguments.h)
target_link_libraries(BeamMP-Desktop PRIVATE Qt6::Widgets)
target_compile_features(BeamMP-Desktop PRIVATE cxx_std_20)

# Supply gui/assets/launcher.ico before configuring to embed a custom icon.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/launcher.ico")
    qt_add_resources(BeamMP-Desktop "launcher_icon"
        PREFIX "/"
        BASE "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/launcher.ico")
    if(WIN32)
        enable_language(RC)
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/launcher-icon.rc"
            "IDI_ICON1 ICON \"${CMAKE_CURRENT_SOURCE_DIR}/assets/launcher.ico\"\n")
        target_sources(BeamMP-Desktop PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/launcher-icon.rc")
    endif()
endif()
# Official Qt MSVC binaries use the shared CRT. The console backend can continue
# using the static CRT required by the original vcpkg build.
set_property(TARGET BeamMP-Desktop PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
install(TARGETS BeamMP-Desktop RUNTIME DESTINATION .)

target_compile_options(BeamMP-Desktop PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8> $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>)
if(BUILD_TESTING)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Test)
    qt_add_executable(gui-fake-backend ../tests/fake_backend.cpp)
    target_link_libraries(gui-fake-backend PRIVATE Qt6::Core)
    target_compile_options(gui-fake-backend PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8> $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>)
    set_property(TARGET gui-fake-backend PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    qt_add_executable(gui-smoke-tests ../tests/frontend_smoke.cpp LauncherWindow.cpp LauncherWindow.h)
    target_link_libraries(gui-smoke-tests PRIVATE Qt6::Widgets Qt6::Test)
    target_include_directories(gui-smoke-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_options(gui-smoke-tests PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8> $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>)
    set_property(TARGET gui-smoke-tests PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    add_dependencies(gui-smoke-tests gui-fake-backend)
    add_test(NAME gui-process-integration COMMAND gui-smoke-tests $<TARGET_FILE:gui-fake-backend>)
    set_tests_properties(gui-process-integration PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 30)
endif()
