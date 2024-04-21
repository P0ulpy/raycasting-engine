cmake_minimum_required(VERSION 3.21)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

project(raycasting-engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

SET(APP_TARGET_NAME raycasting-engine-app)

find_package(raylib CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_path(RAYGUI_INCLUDE_DIRS "raygui.h")

file(GLOB_RECURSE APP_SRC_FILES
    "src/*.h"
    "src/*.hpp"
    "src/*.cpp"
)

add_executable(${APP_TARGET_NAME}
    ${APP_SRC_FILES}
)

set_target_properties(${APP_TARGET_NAME}
    PROPERTIES LINKER_LANGUAGE CXX
)

add_subdirectory(src/vendors)

target_include_directories(${APP_TARGET_NAME}
    PRIVATE ${RAYGUI_INCLUDE_DIRS}
)

target_link_libraries(${APP_TARGET_NAME}
    PRIVATE raylib
    PRIVATE imgui::imgui
)