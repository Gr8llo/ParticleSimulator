cmake_minimum_required(VERSION 3.27)
project(Particles)

set(CMAKE_CXX_STANDARD 20)

# === Build type ===
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif()

# === Custom flags for Release ===
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto -DNDEBUG")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto")
endif()

# === SDL2 ===
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# === GLM ===
find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

# === Source files ===
file(GLOB SOURCES "current/src/*.cpp")

# === Executable ===
add_executable(Particles
        current/mainV3.cpp
        current/include/Constants.h
        ${SOURCES}
)

# === Link libraries ===
target_link_libraries(Particles ${SDL2_LIBRARIES})

# === Show info ===
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")


# Add Flags to Release Mode
##if(CMAKE_BUILD_TYPE STREQUAL "Release")
##    add_compile_options(
##            -O3                  # Ottimizzazione massima per la velocità di esecuzione
##            -march=native        # Usa le istruzioni specifiche della CPU locale
##            -flto                # Link Time Optimization(LTO), permette ottimizzazioni tra unità di compilazione
##            -fno-rtti            # Disabilita Runtime Type Information (RTTI), risparmia spazio e tempo se non serve, niente cast dinamico a runtime
##            -DNDEBUG             # Definisce la macro NDEBUG per disabilitare assert() e controlli debug in runtime
###            -ffast-math          # Ottimizzazioni aggressive su operazioni in virgola mobile, può violare gli standard IEEE (meglio evitare con multithread)
###            -mtune=native        # Ottimizza per la microarchitettura della CPU locale (implicito in -march=native)
##    )
##    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto")
##    # Flag per il linker: abilitano LTO e ottimizzazioni specifiche CPU anche durante il linking
##endif()