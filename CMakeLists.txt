cmake_minimum_required(VERSION 3.20)
project(Terminal)

set(CMAKE_CXX_STANDARD 14)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/whoshuu/cpr.git GIT_TAG c8d33915dbd88ad6c92b258869b03aba06587ff9) # the commit hash for 1.5.0
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json
        GIT_TAG v3.7.3)
FetchContent_GetProperties(json)
if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

find_package(Boost)
include_directories(${Boost_INCLUDE_DIR})

add_executable(Terminal main.cpp)
target_link_libraries(Terminal PRIVATE cpr::cpr)
target_link_libraries(Terminal PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(Terminal PRIVATE ${Boost_INCLUDE_DIR})