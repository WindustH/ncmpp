
cmake_minimum_required(VERSION 3.20)

project(ncmpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find dependencies
find_package(OpenSSL REQUIRED)
find_package(RapidJSON REQUIRED)

# --- ncmlib ---
set(NCMLIB_SRC
    ncmlib/src/ncmdump.cpp
    ncmlib/src/NcmFile.cpp
    ncmlib/src/utils.cpp
    ncmlib/src/base64.cpp
    ncmlib/src/pkcs7.cpp
)
add_library(ncmlib ${NCMLIB_SRC})

# ncmlib needs to know where its own headers are and where rapidjson is.
target_include_directories(ncmlib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ncmlib/include>
    PRIVATE
        ${RapidJSON_INCLUDE_DIRS}
)

# Link ncmlib to its dependencies
target_link_libraries(ncmlib PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
)


# --- ncmpp executable ---
file(GLOB NCMPP_SRC "ncmpp/src/*.cpp")
add_executable(ncmpp ${NCMPP_SRC})

# ncmpp needs to know where its own headers are.
target_include_directories(ncmpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ncmpp/src)

# Link ncmpp to ncmlib. This transitively links to dependencies.
target_link_libraries(ncmpp PRIVATE ncmlib)

# The <filesystem> library should be automatically linked with C++17 and later.
# If you encounter linker errors related to std::filesystem on older compilers,
# you might need to explicitly link against it. For example:
# target_link_libraries(ncmpp PRIVATE stdc++fs)
