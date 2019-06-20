# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(${CMAKE_C_FLAGS}, "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall -Werror")
set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             main

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             main.cpp)

target_include_directories(main PRIVATE
        engine/public)

target_include_directories(main PRIVATE
        include)

add_library(engine STATIC

        engine/private/engine.cpp
        engine/private/engine_helper.cpp
        engine/private/engine_debug.cpp
        engine/private/engine_buffer.cpp
        engine/private/engine_physics.cpp
        engine/private/engine_texture.cpp
        engine/private/engine_geometry.cpp
        engine/private/engine_material.cpp
        engine/private/engine_resource.cpp)

target_include_directories(engine PRIVATE
        engine/public)
target_include_directories(engine PRIVATE
        include)
target_include_directories(engine PRIVATE
        mdlcg/public)

set(CGAL_SOURCE_DIR C:/Users/andys/Documents/repos/cgal)
set(CGAL_BUILD_DIR C:/Users/andys/Documents/repos/cgal_android)
add_subdirectory(${CGAL_SOURCE_DIR} ${CGAL_BUILD_DIR})

add_library(mdlcg STATIC
        mdlcg/private/mdlcg.cpp)
target_include_directories(mdlcg PRIVATE mdlcg/public)
target_include_directories(mdlcg PRIVATE ${CGAL_INCLUDE_DIRS})
target_include_directories(mdlcg PRIVATE C:/Users/andys/Documents/repos/boost_1.69.0_prebuilt_ndk_19/include)
set_target_properties(mdlcg PROPERTIES COMPILE_FLAGS "-w -std=c++14")

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        main

                       # Links the target library to the log library
                       # included in the NDK.
        android
        vulkan
        CGAL
        CGAL_Core
        CGAL_ImageIO
        engine
        mdlcg
        ${log-lib} )