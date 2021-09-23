cmake_minimum_required(VERSION 3.0.0)
project(chaiscript VERSION 0.1.0)

set(CMAKE_CXX_STANDARD_REQUIRED 20)

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_run(REQUIRES chaiscript/6.1.0@
        GENERATORS cmake cmake_paths cmake_find_package
        BUILD_TYPE Debug
        BASIC_SETUP CMAKE_TARGETS
)

include(CTest)
enable_testing()

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::chaiscript)

set(EX_EASY_CHAIN_TEST easy_chain_test)
add_executable(${EX_EASY_CHAIN_TEST} easy-chain-test.cpp)
target_link_libraries(${EX_EASY_CHAIN_TEST} CONAN_PKG::chaiscript)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${PROJECT_SOURCE_DIR}/resource/"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
