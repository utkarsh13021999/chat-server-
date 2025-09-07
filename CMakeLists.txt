cmake_minimum_required(VERSION 3.20)

project(ChatServerCpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(uwebsockets CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_executable(chat_server
  src/main.cpp
  src/MessageStore.cpp
)

target_include_directories(chat_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(chat_server PRIVATE
  uwebsockets::uwebsockets
  nlohmann_json::nlohmann_json
  Threads::Threads
)

add_custom_command(TARGET chat_server POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:chat_server>/static
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/static $<TARGET_FILE_DIR:chat_server>/static
)