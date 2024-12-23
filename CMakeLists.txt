cmake_minimum_required(VERSION 3.17)
project(testForDiploma)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE_FILES main.cpp)

message(STATUS "start running cmake...")

set(BOOST_ROOT "C:/Program Files/boost/boost_1_87_0")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")


# Если библиотека найдена, то
find_package(Boost REQUIRED)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif ()

add_executable(testForDiploma ${SOURCE_FILES})

if(BOOST_FOUND)
    target_link_libraries(testForDiploma ${Boost_LIBRARIES}) # подключаем библиотеку
endif()
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(testForDiploma PRIVATE nlohmann_json::nlohmann_json)

find_path(MAPBOX_GEOJSON_CPP_INCLUDE_DIRS "mapbox/geojson.hpp")
target_include_directories(testForDiploma PRIVATE ${MAPBOX_GEOJSON_CPP_INCLUDE_DIRS})