cmake_minimum_required(VERSION 3.1.0)

project(fft-ifft-2d VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)

set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/dependencies/opencv-4.11.0/build)
find_package(OpenCV REQUIRED)

include_directories(
    include/
    ${OpenCV_INCLUDE_DIRS}
)

file(GLOB SOURCES src/*.cpp)

add_executable(${PROJECT_NAME} 
    ${SOURCES} 
    ui/widget.ui include/widget.hpp
)

target_link_libraries(${PROJECT_NAME} 
    Qt5::Widgets Qt5::Core Qt5::Gui
    ${OpenCV_LIBS}
)