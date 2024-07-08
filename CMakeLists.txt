cmake_minimum_required(VERSION 3.4)

#Setting Project
project(BIT-TORRENT-APP VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

#Build Library and Executable path
#Scheme 1
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>")

#Setting Directory
set(HEADER_DIR ${CMAKE_SOURCE_DIR}/include)
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/utils)


#Setting Library and executable name
set(TORRENT_LIB torrent)
set(TEST_EXE test_main)


#Setting Fetch content function directory
include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/utils")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include(AddGitSubmodule)


#OPENSSL LIBRARY
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL)

if( OPENSSL_FOUND )
        message(STATUS "OpenSSL include found: ${OPENSSL_INCLUDE_DIR}")
        message(STATUS "OpenSSL crypto lib found: ${OPENSSL_CRYPTO_LIBRARY}")
        message(STATUS "OpenSSL ssl lib found: ${OPENSSL_SSL_LIBRARY}")
        message(STATUS "OpenSSL all lib found: ${OPENSSL_LIBRARIES}")
else()
        message("System OpenSSL not found")
endif()


#eventpp
FetchContent_Declare(
    eventpp
    GIT_REPOSITORY https://github.com/wqking/eventpp.git
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(eventpp)

#set(EVENTPP_HEADER_DIR 
#${CMAKE_SOURCE_DIR}/utils/eventpp-src/include)

#CURL LIBRARY

# Find the CURL package
find_package(CURL REQUIRED)

#get_target_property(CURL_INCLUDE_DIRS CURL::libcurl INTERFACE_INCLUDE_DIRECTORIES)
#get_target_property(CURL_LIBRARIES CURL::libcurl INTERFACE_LINK_LIBRARIES)
if( CURL_FOUND )
        message(STATUS "CURL include found: ${OPENSSL_INCLUDE_DIR}")
        message(STATUS "CURL lib found: ${CURL_LIBRARIES}")
else()
        message("System CURL not found")
endif()


#Adding subdirectory CMakeLists.txt
add_subdirectory(src)
add_subdirectory(test)


#add_subdirectory(utils/eventpp-src)
