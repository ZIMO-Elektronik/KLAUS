cmake_minimum_required(VERSION 3.21)

project(z-klaus LANGUAGES CXX)

# Because of some override when using vcpkg, we can't fetch CMakeModules before
# the project call. The effect is an ininite recursion within either
# add_executable or find_package
include(FetchContent)
FetchContent_Declare(
  CMakeModules
  GIT_REPOSITORY "https://github.com/ZIMO-Elektronik/CMakeModules"
  GIT_TAG v0.13.0
  SOURCE_DIR ${CMAKE_BINARY_DIR}/CMakeModules)
FetchContent_MakeAvailable(CMakeModules)
version_from_git()
set(CMAKE_PROJECT_VERSION ${VERSION_FROM_GIT})
set(CMAKE_PROJECT_VERSION_MAJOR ${VERSION_MAJOR_FROM_GIT})
set(CMAKE_PROJECT_VERSION_MINOR ${VERSION_MINOR_FROM_GIT})
set(CMAKE_PROJECT_VERSION_PATCH ${VERSION_PATCH_FROM_GIT})

add_executable(
  ${CMAKE_PROJECT_NAME} WIN32
  src/main.cpp
  src/process/mdu_ein/update.cpp
  src/process/mdu_ein/soundload.cpp
  src/process/susiv2/soundload.cpp
  src/process/base.cpp
  src/ui/helper/progress_tracker.cpp
  src/ui/helper/firmware_fetcher.cpp
  src/ui/process_manager.cpp
  src/ui/update_backend.cpp
  src/ui/soundload_backend.cpp
  src/ui/app_backend.cpp
  # Add the icon as resource for Windows
  $<$<BOOL:${WIN32}>:${CMAKE_CURRENT_SOURCE_DIR}/data/kleinweich.rc>)

target_compile_definitions(${CMAKE_PROJECT_NAME}
                           PRIVATE PROJECT_VERSION=${VERSION_FROM_GIT})

set(SLINT_OPTIONS "SLINT_BUILD_EXAMPLES ON")

find_package(tinyfiledialogs CONFIG REQUIRED)
find_package(CURL REQUIRED)

if(NOT TARGET Slint::Slint)
  cpmaddpackage("gh:slint-ui/slint#release/1")
endif()

if(NOT TARGET ULF::ULF)
  cpmaddpackage("gh:ZIMO-Elektronik/libULF@0.3.0")
endif()

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE src)

target_link_libraries(
  ${CMAKE_PROJECT_NAME} PRIVATE Slint::Slint ULF
                                tinyfiledialogs::tinyfiledialogs CURL::libcurl)
slint_target_sources(${CMAKE_PROJECT_NAME} ui/app-window.slint)
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY SLINT_EMBED_RESOURCES
                                                   embed-files)

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

install(
  TARGETS z-klaus ULF
  LIBRARY DESTINATION lib # .so-Files
  RUNTIME DESTINATION bin # .dll-Files (to avoid linking issues)
  ARCHIVE DESTINATION lib # [.a/.lib]-Files
)

if(UNIX AND NOT APPLE)
  find_package(appimagetool)

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

  install(FILES $<TARGET_RUNTIME_DLLS:z-klaus> DESTINATION bin)
  install(FILES $<TARGET_RUNTIME_DLLS:ULF> DESTINATION bin)

  if(${CMAKE_CROSSCOMPILING})
    # Depends on the actual compiler lib dir being set in `CMAKE_LIBRARY_PATH`
    install(
      CODE "
      # Determine and install dependencies
      file(GET_RUNTIME_DEPENDENCIES
        EXECUTABLES $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
        RESOLVED_DEPENDENCIES_VAR resolved_deps
        UNRESOLVED_DEPENDENCIES_VAR unresolved_deps
        DIRECTORIES ${CMAKE_LIBRARY_PATH}
      )
      foreach(dll \${resolved_deps})
        file(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}/bin\" TYPE FILE FILES \"\${dll}\")
      endforeach()

      if(${unresolved_deps})
        message(WARNING \"Unresolved dependencies. Note, that system32 dependencies will always be unresolved when cross-compiling\")
      endif()
    ")
  else()
    get_filename_component(COMPILER_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY
                           CACHE PATH)
    cmake_policy(SET CMP0207 NEW)

    install(
      CODE "
        cmake_policy(SET CMP0207 NEW)

        # Determine dependencies
        file(GET_RUNTIME_DEPENDENCIES
          EXECUTABLES $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
          RESOLVED_DEPENDENCIES_VAR resolved_deps
          UNRESOLVED_DEPENDENCIES_VAR unresolved_deps
          DIRECTORIES \"${COMPILER_BIN_DIR}\"
          PRE_INCLUDE_REGEXES \".*\"
          PRE_EXCLUDE_REGEXES \"^.*[Ss]ystem32.*\$\"
        )

        # Install dependencies EXCEPT those in system32
        foreach(dll \${resolved_deps})
          get_filename_component(dir \"\${dll}\" DIRECTORY)
          if(NOT dir MATCHES \"^.*[Ss]ystem32.*\$\")
            file(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}/bin\" TYPE FILE FILES \"\${dll}\")
          endif()
        endforeach()

        foreach(dll \${unresolved_deps})
          message(WARNING \"Unresolved dll: \${dll}\")
        endforeach()
      ")
  endif()
endif()

set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME
    ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}
)
set(CPACK_PACKAGE_VENDOR "ZIMO-Elektronik")
set(CPACK_PACKAGE_DESCRIPTION "A cross-platform decoder update utility")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "z-klaus")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)

set(CPACK_NSIS_MODIFY_PATH OFF)
set(CPACK_NSIS_DISPLAY_NAME z-klaus)
set(CPACK_NSIS_PACKAGE_NAME z-klaus)
set(CPACK_NSIS_MUI_ICON ${CMAKE_SOURCE_DIR}/data/zimo_logo.ico)
set(CPACK_NSIS_MUI_UNIICON ${CMAKE_SOURCE_DIR}/data/zimo_logo.ico)
set(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut '$SMPROGRAMS\\\\z-klaus\\\\KLAUS.lnk' '$INSTDIR\\\\bin\\\\z-klaus.exe'"
)
set(CPACK_NSIS_DELETE_ICONS_EXTRA
    "Delete '$SMPROGRAMS\\\\z-klaus\\\\KLAUS.lnk'")
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
    "CreateShortCut '$DESKTOP\\\\KLAUS.lnk' '$INSTDIR\\\\bin\\\\z-klaus.exe'")
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "Delete '$DESKTOP\\\\KLAUS.lnk'")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

set(CPACK_DMG_NAME z-klaus)
set(CPACK_DMG_ICON ${CMAKE_SOURCE_DIR}/data/zimo_logo.svg)
set(CPACK_DMG_PLIST ${CMAKE_SOURCE_DIR}/data/Entitlements.plist)

set(CPACK_BUNDLE_NAME z-klaus)
set(CPACK_BUNDLE_ICON ${CMAKE_SOURCE_DIR}/data/zimo_logo.svg)
set(CPACK_BUNDLE_PLIST ${CMAKE_SOURCE_DIR}/data/Entitlements.plist)

include(CPack)
