cmake_minimum_required(VERSION 2.8)

Project(To3mx)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# c++ 17
if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

# osg
find_package(OpenSceneGraph 2.0.0 REQUIRED osgDB osgUtil)
include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})

# liblzma
file (GLOB LIBLZMA_SRC ./OpenCTM/liblzma/*.c)
file (GLOB LIBLZMA_H ./OpenCTM/liblzma/*.h)
source_group("liblzma" FILES ${LIBLZMA_SRC} ${LIBLZMA_H} )
include_directories("./OpenCTM/liblzma/")

# openctm
file (GLOB OPENCTM_SRC ./OpenCTM/*.c)
file (GLOB OPENCTM_H ./OpenCTM/*.h)
source_group("openCTM" FILES ${OPENCTM_SRC} ${OPENCTM_H} )
include_directories("./openCTM/")
add_definitions(-DOPENCTM_STATIC)

# cJsonObject
set(CJSONOBJECT_SRC 
	./cJsonObject/CJsonObject.cpp
	./cJsonObject/cJSON.c
)

set(CJSONOBJECT_H
	./cJsonObject/CJsonObject.hpp
    ./cJsonObject/cJSON.h
)
source_group("cJsonObject" FILES ${CJSONOBJECT_SRC} ${CJSONOBJECT_H} )
include_directories("./cJsonObject/")


# src
file(GLOB HEADER ./src/*.h)
file(GLOB SRC ./src/*.cpp)

set(TARGET_SRC 
	${SRC}
	${CJSONOBJECT_SRC}
	${LIBLZMA_SRC}
	${OPENCTM_SRC}
)

set(TARGET_H
	${HEADER}
	${CJSONOBJECT_H}
	${LIBLZMA_H}
	${OPENCTM_H}
)

add_executable(${CMAKE_PROJECT_NAME} ${TARGET_SRC} ${TARGET_H})
target_link_libraries(${CMAKE_PROJECT_NAME} ${OPENSCENEGRAPH_LIBRARIES})