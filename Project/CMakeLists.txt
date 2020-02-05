cmake_minimum_required(VERSION 3.14.5)
project(Skeleton)

set(CMAKE_CXX_STANDARD 17)
set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)
set(GLFW_DIR ${LIBS_DIR}/glfw)
set(GLAD_DIR ${LIBS_DIR}/glad)
set(KHR_DIR ${LIBS_DIR}/KHR)
set(Eigen_DIR ${LIBS_DIR}/Eigen)
set(SOURCE_FILES ${GLAD_DIR}/src/glad.c)
set(THREADS_PREFER_PTHREAD_FLAG ON)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

find_package(Threads REQUIRED)

add_subdirectory(${GLFW_DIR})

include_directories($ENV{KINECTSDK10_DIR}/inc ${GLAD_DIR}/inc ${KHR_DIR})
link_directories($ENV{KINECTSDK10_DIR}/lib/amd64)

add_executable(Skeleton main.cpp ${HEADER_FILES} ${SOURCE_FILES} ${SOURCES})
target_link_libraries(Skeleton Kinect10 glfw Threads::Threads)
target_include_directories(Skeleton PUBLIC $ENV{KINECTSDK10_DIR}/inc ${GLAD_DIR}/inc ${KHR_DIR} ${Eigen_DIR})