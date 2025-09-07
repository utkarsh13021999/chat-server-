cmake_minimum_required(VERSION 3.20)

project(ChatServerCpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find packages
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

# Add uWS include directory
set(UWS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/uws)
if(NOT EXISTS ${UWS_DIR})
    message(FATAL_ERROR "uWS directory not found. Run: git clone https://github.com/uNetworking/uWebSockets.git uws")
endif()

add_executable(chat_server
  src/main.cpp
  src/MessageStore.cpp
)

target_include_directories(chat_server PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${UWS_DIR}/src
  ${UWS_DIR}/uSockets/src
)

target_link_libraries(chat_server PRIVATE
  nlohmann_json::nlohmann_json
  Threads::Threads
  OpenSSL::SSL
  OpenSSL::Crypto
  ZLIB::ZLIB
  ${UWS_DIR}/uSockets/libusockets.a
)

# Copy static assets
add_custom_command(TARGET chat_server POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:chat_server>/static
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/static $<TARGET_FILE_DIR:chat_server>/static
)