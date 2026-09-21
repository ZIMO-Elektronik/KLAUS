cmake_minimum_required(VERSION 3.21)

include(FetchContent)
FetchContent_Declare(
  CMakeModules
  GIT_REPOSITORY "https://gitea.zimo.at:3000/ZIMO/CMakeModules.git"
  GIT_TAG v0.12.2
  SOURCE_DIR ${CMAKE_BINARY_DIR}/CMakeModules)
FetchContent_MakeAvailable(CMakeModules)

version_from_git()
project(
  z-klaus
  VERSION ${VERSION_FROM_GIT}
  LANGUAGES CXX)

add_executable(
  ${CMAKE_PROJECT_NAME} WIN32
  src/main.cpp
  src/process/mdu_ein/update.cpp
  src/process/mdu_ein/soundload.cpp
  src/process/susiv2/soundload.cpp
  src/process/base.cpp
  src/ui/helper/progress_tracker.cpp
  src/ui/process_manager.cpp
  src/ui/update_backend.cpp
  src/ui/soundload_backend.cpp
  src/ui/app_backend.cpp
  # Add the icon as resource for Windows
  $<$<BOOL:${WIN32}>:${CMAKE_CURRENT_SOURCE_DIR}/data/kleinweich.rc>)

target_compile_definitions(${CMAKE_PROJECT_NAME}
                           PRIVATE PROJECT_VERSION=${VERSION_FROM_GIT})

FetchContent_Declare(
  Slint
  GIT_REPOSITORY https://github.com/slint-ui/slint.git
  # `release/1` will auto-upgrade to the latest Slint >= 1.0.0 and < 2.0.0
  # `release/1.0` will auto-upgrade to the latest Slint >= 1.0.0 and < 1.1.0
  GIT_TAG release/1
  SOURCE_SUBDIR api/cpp)
FetchContent_MakeAvailable(Slint)

set(SLINT_OPTIONS "SLINT_BUILD_EXAMPLES ON")

if(WIN32)
  list(
    APPEND
    SLINT_OPTIONS
    "SLINT_FEATURE_RENDERER_FEMTOVG OFF"
    "SLINT_FEATURE_RENDERER_SOFTWARE ON"
    "SLINT_FEATURE_RENDERER_WINIT_SKIA ON"
    "SLINT_FEATURE_BACKEND_WINIT ON"
    "SKIA_GN_ARGS win_vc=\"\"")
endif()

if(NOT TARGET Slint::Slint)
  cpmaddpackage(
    NAME
    Slint
    GITHUB_REPOSITORY
    slint-ui/slint
    GIT_TAG
    release/1
    OPTIONS
    ${SLINT_OPTIONS})
endif()

if(NOT TARGET tinyfiledialogs::tinyfiledialogs)
  cpmaddpackage(
    NAME
    Tinyfiledialogs
    GITHUB_REPOSITORY
    D31m05z/tinyfiledialogs
    SYSTEM
    YES
    GIT_TAG
    master)
endif()

if(NOT TARGET ULF::ULF)
  cpmaddpackage(
    NAME
    ULF
    GIT_REPOSITORY
    git@github.com:ZIMO-Elektronik/libULF.git
    SYSTEM
    YES
    VERSION
    0.2.0)
endif()

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE src)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Slint::Slint ULF
                                                    ::tinyfiledialogs)
slint_target_sources(${CMAKE_PROJECT_NAME} ui/app-window.slint)
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY SLINT_EMBED_RESOURCES
                                                   embed-files)

# On Windows, copy the Slint DLL next to the application binary so that it's
# found.
if(WIN32)
  add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME}
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${CMAKE_PROJECT_NAME}>
      $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>
    COMMAND_EXPAND_LISTS)
endif()

if(PROJECT_IS_TOP_LEVEL)
  file(
    DOWNLOAD
    "https://github.com/ZIMO-Elektronik/.github/raw/master/data/.clang-format"
    ${CMAKE_CURRENT_LIST_DIR}/.clang-format)
  file(GLOB_RECURSE SRC include/*.*pp src/*.*pp tests/*.*pp)
  add_clang_format_target(KLUGFormat OPTIONS -i FILES ${SRC})
endif()

set(CMAKE_INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

# ---- CPack instructions ----

install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION bin)
install(
  TARGETS ULF
  LIBRARY DESTINATION lib # .so-Files
  RUNTIME DESTINATION bin # .dll-Files (to avoid linking issues)
  ARCHIVE DESTINATION lib # [.a/.lib]-Files
)
install(
  RUNTIME_DEPENDENCY_SET
  deps
  DESTINATION
  lib
  PRE_EXCLUDE_REGEXES
  "^libpthread"
  "^libc\\."
  "^libm\\."
  "^libgcc"
  "^libstdc\\+\\+")

if(UNIX AND NOT APPLE)
  # Install .deb specifics
  install(FILES "data/z-klaus.desktop" DESTINATION "share/applications")
  install(FILES "data/zimo_logo.png" DESTINATION "share/pixmaps")
  install(FILES "data/zimo_logo.svg"
          DESTINATION "share/icons/hicolor/scalable/apps")

  # Check filter lists at
  # https://github.com/AppImageCommunity/pkg2appimage/blob/master/excludelist#L18
  install(
    CODE [[
    file(GET_RUNTIME_DEPENDENCIES
        EXECUTABLES $<TARGET_FILE:z-klaus>
        RESOLVED_DEPENDENCIES_VAR resolved_deps
        UNRESOLVED_DEPENDENCIES_VAR unresolved_deps
    )
    foreach(lib ${resolved_deps})
        # Don't include system libs (libpthread, etc.)
        if(lib MATCHES "ld-linux(-[a-zA-Z0-9_-]+)?\\.so"
              OR lib MATCHES "libc\\.so"
              OR lib MATCHES "libpthread\\.so"
              OR lib MATCHES "libdl\\.so"
              OR lib MATCHES "libexpat\\.so"
              OR lib MATCHES "libfontconfig\\.so"
              OR lib MATCHES "libfreetype\\.so"
              OR lib MATCHES "libgcc_s\\.so"
              OR lib MATCHES "libm\\.so"
              OR lib MATCHES "libstdc\\+\\+\\.so"
              OR lib MATCHES "libz\\.so")
            continue()
        endif()

        # copy symlink
        file(INSTALL ${lib} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

        # resolve symlink
        file(REAL_PATH ${lib} resolved_lib_path)

        # copy resolved dependency
        file(COPY ${resolved_lib_path} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
    endforeach()
  ]])
elseif(WIN32)
  install(FILES "data/zimo_logo.png" DESTINATION bin)

  # Move misplaced DLLs to bin dir for Windows
  install(
    CODE [[
        file(GLOB_RECURSE _misplaced_dlls "${CMAKE_INSTALL_PREFIX}/lib/*.dll")
        if(_misplaced_dlls)
            message(STATUS "Fixing Windows DLL paths for third-party libraries...")
            file(MAKE_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")

            foreach(_dll ${_misplaced_dlls})
                get_filename_component(_dll_name "${_dll}" NAME)
                file(RENAME "${_dll}" "${CMAKE_INSTALL_PREFIX}/bin/${_dll_name}")
            endforeach()
        endif()
    ]])
endif()

set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "ZIMO-Elektronik")
set(CPACK_PACKAGE_DESCRIPTION "A cross-platform decoder update utility")

include(CPack)
