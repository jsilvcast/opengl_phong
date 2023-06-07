cmake_minimum_required (VERSION 3.16)
project (gl_test)
set(library_name lib)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin) #set binary dir to bin folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)


set(LIBS
        ${LIBS}
        GLEW::GLEW
        glfw
        GL
        )

set(HEADER_FILES ${HEADER_FILES} textfile/textfile.h textfile/textfile_ALT.h)
set(SOURCE_FILES ${SOURCE_FILES} textfile/textfile.c)

add_library(${library_name} ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${library_name} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SRC_DIR}>")
target_link_libraries(${library_name} PUBLIC ${LIBS})

add_executable(phong spinningcube_withlight_SKEL.cpp)
target_link_libraries (phong ${library_name})
