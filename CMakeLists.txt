cmake_minimum_required(VERSION 3.24)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

project(cmake-vcpkg-template)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

SET(EXECUTABLE_TARGET_NAME cmake-vcpkg-template-app)

add_executable(${EXECUTABLE_TARGET_NAME}
    main.cpp
)
