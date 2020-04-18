cmake_minimum_required(VERSION 3.10)

project(steering VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

find_package(Allegro5 REQUIRED)

include_directories(${ALLEGRO5_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(SOURCE_FILES src/main.cpp)

set(LIBS ${ALLEGRO5_LIBRARIES})

add_executable(steering ${SOURCE_FILES})
target_link_libraries(steering ${LIBS})
