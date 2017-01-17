#set(Boost_USE_STATIC_LIBS ON)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME ON)

SET(CMAKE_CXX_FLAGS_PROFILE "-pg")
SET(CMAKE_CXX_FLAGS "-std=c++11 -D_WEBSOCKETPP_CPP11_STL_ -Wfatal-errors")
#SET(BOOST_LIBRARYDIR ${PROJECT_SOURCE_DIR}/src/libs)
#SET(BOOST_INCLUDEDIR ${PROJECT_SOURCE_DIR}/include)

find_package(Boost 1.53.0 REQUIRED COMPONENTS system program_options)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories (${PROJECT_SOURCE_DIR}/include/websocketpp
    ${PROJECT_SOURCE_DIR}/src/common
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS})

add_subdirectory(common)
add_subdirectory(wschat)
