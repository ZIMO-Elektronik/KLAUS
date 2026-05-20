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
  my_application
  VERSION ${VERSION_FROM_GIT}
  LANGUAGES CXX)

add_executable(Updater src/main.cpp)

FetchContent_Declare(
  Slint
  GIT_REPOSITORY https://github.com/slint-ui/slint.git
  # `release/1` will auto-upgrade to the latest Slint >= 1.0.0 and < 2.0.0
  # `release/1.0` will auto-upgrade to the latest Slint >= 1.0.0 and < 1.1.0
  GIT_TAG release/1
  SOURCE_SUBDIR api/cpp)
FetchContent_MakeAvailable(Slint)

if(NOT TARGET Slint::Slint)
  cpmaddpackage(
    NAME
    Slint
    GITHUB_REPOSITORY
    slint-ui/slint
    GIT_TAG
    release/1
    OPTIONS
    SLINT_BUILD_EXAMPLES
    ON)
endif()

if(NOT TARGET KLUG::KLUG)
  cpmaddpackage(
    NAME
    KLUG
    GIT_REPOSITORY
    https://gitea.zimo.at:3000/ZIMO/libklug.git
    SYSTEM
    YES
    GIT_TAG
    master)
endif()

target_link_libraries(Updater PRIVATE Slint::Slint KLUG)
slint_target_sources(Updater ui/app-window.slint)

# On Windows, copy the Slint DLL next to the application binary so that it's
# found.
if(WIN32)
  add_custom_command(
    TARGET Updater
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:my_application>
            $<TARGET_FILE_DIR:my_application>
    COMMAND_EXPAND_LISTS)
endif()
