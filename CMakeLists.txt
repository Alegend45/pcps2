cmake_minimum_required (VERSION 3.8)
set( CMAKE_VERBOSE_MAKEFILE ON )

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project (emotionless)

#properly disable insource building, as CMAKE_DISABLE_IN_SOURCE_BUILD never works for me
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(SEND_ERROR "Insource building is disabled.")
endif()

set(SOURCES
src/attotime.cpp
src/attotime.h
src/common.cpp
src/common.h
src/ee.cpp
src/ee.h
src/eminline.h
src/emucore.h
src/iop_dma.cpp
src/iop_dma.h
src/iop_intc.cpp
src/iop_intc.h
src/iop.cpp
src/iop.h
src/main.cpp
src/scph10000.cpp
src/scph10000.h
src/scph15000.cpp
src/scph15000.h
src/scph30000.cpp
src/scph30000.h
)

add_executable(emotionless ${SOURCES})