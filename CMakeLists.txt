cmake_minimum_required(VERSION 2.6)

# Name of project
set(PROJ_NAME wind)
project(${PROJ_NAME})


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(COMPILER_FLAGS "-g")

set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})

# Specify project files: header files and source files
set(HDRS
	include/application.h
	include/view.h
	include/simulation.h
	include/mesh.h
	include/camera.h
	include/freecamera.h
	include/random.h
	include/shader.h
	include/shapes.h
    include/perlin.h
    include/aircraft.h
    include/camera.h
	include/paths.h
    include/defs.h)
 
set(SRCS
	src/main.cc
	src/engine/application.cc
	src/engine/view.cc
	src/engine/simulation.cc
	src/engine/mesh.cc
	src/engine/camera.cc
	src/engine/freecamera.cc
	src/engine/shader.cc
	src/engine/random.cc
	src/engine/shapes.cc
    src/engine/aircraft.cc)

# Add path name to configuration file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/path_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/paths.h)

# Add executable based on the source files
add_definitions(${COMPILER_FLAGS})
add_executable(${PROJ_NAME} ${HDRS} ${SRCS} )

# Directories to include for header files, so that the compiler can find
# path_config.h
target_include_directories(${PROJ_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJ_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# Require OpenGL library
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
target_link_libraries(${PROJ_NAME} ${OPENGL_gl_LIBRARY})

# Other libraries needed
set(LIBRARY_PATH "" CACHE PATH "Folder with GLEW, GLFW, GLM, and SOIL libraries")
include_directories(${LIBRARY_PATH}/include)
if(NOT WIN32)
    find_library(GLEW_LIBRARY GLEW)
    find_library(GLFW_LIBRARY glfw)
elseif(WIN32)
    find_library(GLEW_LIBRARY glew32s HINTS ${LIBRARY_PATH}/lib)
    find_library(GLFW_LIBRARY glfw3 HINTS ${LIBRARY_PATH}/lib)
endif(NOT WIN32)
target_link_libraries(${PROJ_NAME} ${GLEW_LIBRARY})
target_link_libraries(${PROJ_NAME} ${GLFW_LIBRARY})

# The rules here are specific to Windows Systems
if(WIN32)
    # Avoid ZERO_CHECK target in Visual Studio
    set(CMAKE_SUPPRESS_REGENERATION TRUE)
 
    # This will use the proper libraries in debug mode in Visual Studio
    set_target_properties(${PROJ_NAME} PROPERTIES DEBUG_POSTFIX _d)
endif(WIN32)
