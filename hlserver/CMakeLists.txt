cmake_minimum_required(VERSION 3.5)
project(HLServer)

find_package(Boost 1.60.0 REQUIRED COMPONENTS system thread)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
set(Boost_DEBUG OFF)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include)

file(GLOB hlserver_SRC "src/*.cpp")

add_executable(hlserver ${hlserver_SRC})
target_link_libraries(hlserver ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${OPENSSL_CRYPTO_LIBRARY})
