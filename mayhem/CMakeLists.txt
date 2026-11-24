# Mayhem fuzz build for Torque2D.
#
# Upstream's root CMakeLists.txt only produces the monolithic `Torque2D` executable. This file
# reuses upstream's own source lists (cmake/EngineSources.cmake + cmake/PlatformSources.cmake) and
# its vendored third-party libs (engine/lib), but builds the engine as a STATIC library with the
# platform `main` renamed, so fuzz harnesses (and the unit-test runner) can boot the engine
# headless (dedicated mode) and call its parsers through their public class APIs.
#
#   cmake -S mayhem -B mayhem/build-fuzz -DT2D_FUZZ=ON    # sanitized fuzzers
#   cmake -S mayhem -B mayhem/build-test -DT2D_FUZZ=OFF   # plain unit-test runner
cmake_minimum_required(VERSION 3.24)
project(Torque2DMayhem LANGUAGES CXX C)

option(T2D_FUZZ "Build the libFuzzer harnesses (else only the unit-test runner)" ON)
set(T2D_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
get_filename_component(T2D_ROOT "${T2D_ROOT}" ABSOLUTE)
set(TORQUE_SRC "${T2D_ROOT}/engine/source")
set(TORQUE_LIB "${T2D_ROOT}/engine/lib")

add_subdirectory(${TORQUE_LIB} ${CMAKE_BINARY_DIR}/thirdparty)
# The vendored 1990s-era libjpeg/libpng/zlib/ogg/vorbis left-shift negative values and overflow
# signed ints on VALID images (jdhuff.c:357, png.c:2185 on the repo's own assets), which would
# halt every input. Relax only those two UBSan checks, only in the vendored libs; ASan stays on.
foreach(_tp ljpeg lpng zlib libogg libvorbis)
    target_compile_options(${_tp} PRIVATE -fno-sanitize=shift,signed-integer-overflow)
endforeach()

include(${T2D_ROOT}/cmake/EngineSources.cmake)
include(${T2D_ROOT}/cmake/PlatformSources.cmake)

add_library(t2d_engine STATIC ${TORQUE_ENGINE_SOURCES} ${TORQUE_PLATFORM_SOURCES_LINUX})
target_compile_features(t2d_engine PUBLIC cxx_std_17)
# The platform entry point; harnesses provide their own.
set_source_files_properties(${TORQUE_SRC}/platformX86UNIX/x86UNIXWindow.cc
    PROPERTIES COMPILE_DEFINITIONS "main=t2d_platform_main")

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_library(SDL12_LIBRARY NAMES SDL SDL-1.2 REQUIRED)
find_path(SDL12_INCLUDE_DIR NAMES SDL/SDL.h REQUIRED)

target_include_directories(t2d_engine PUBLIC
    ${TORQUE_SRC}
    ${TORQUE_LIB}
    ${TORQUE_SRC}/persistence/rapidjson
    ${TORQUE_SRC}/persistence/rapidjson/include
    ${TORQUE_SRC}/testing/googleTest
    ${TORQUE_SRC}/testing/googleTest/include
    ${SDL12_INCLUDE_DIR}
)
# Same Linux 64-bit definitions as upstream's root CMakeLists (its "Release" configuration:
# no TORQUE_DEBUG, so assertion dialogs never block the fuzzer).
target_compile_definitions(t2d_engine PUBLIC
    UNICODE _UNICODE TORQUE_UNICODE TORQUE_DEBUG_GUARD TORQUE_NET_STATS LINUX TORQUE_64)
target_compile_options(t2d_engine PUBLIC -Wno-register -w)
target_link_libraries(t2d_engine PUBLIC
    libogg libvorbis lpng ljpeg zlib
    Threads::Threads OpenGL::GL Freetype::Freetype m dl rt X11 Xft fontconfig
    ${SDL12_LIBRARY} openal)

add_library(t2d_boot STATIC harnesses/t2d_boot.cc)
# Whole archive: the console class registrations (IMPLEMENT_CONOBJECT), console functions and
# GoogleTest cases are static initializers nothing references directly.
target_link_libraries(t2d_boot PUBLIC "$<LINK_LIBRARY:WHOLE_ARCHIVE,t2d_engine>")

# Unit-test runner (upstream's GoogleTest suite in engine/source/testing/tests, booted headless).
if(T2D_FUZZ)
    set(T2D_TEST_EXCLUDE EXCLUDE_FROM_ALL)
endif()
add_executable(t2d_unittests ${T2D_TEST_EXCLUDE} harnesses/t2d_unittests.cc)
target_link_libraries(t2d_unittests PRIVATE t2d_boot)

if(T2D_FUZZ)
    set(T2D_FUZZERS taml_xml taml_json taml_binary script_compile dso_read zip
                    bitmap_png bitmap_bmp gfont_uft bitmap_font)
    set(T2D_LSAN_OFF "${CMAKE_CURRENT_SOURCE_DIR}/lsan_off.cc" CACHE FILEPATH "LSan-off TU")
    add_library(lsan_off OBJECT ${T2D_LSAN_OFF})
    add_library(standalone_main OBJECT $ENV{STANDALONE_FUZZ_MAIN})
    set_source_files_properties($ENV{STANDALONE_FUZZ_MAIN} PROPERTIES LANGUAGE C)
    foreach(f ${T2D_FUZZERS})
        add_executable(fuzz_${f} harnesses/fuzz_${f}.cc $<TARGET_OBJECTS:lsan_off>)
        target_link_libraries(fuzz_${f} PRIVATE t2d_boot)
        target_link_options(fuzz_${f} PRIVATE $ENV{LIB_FUZZING_ENGINE})
        add_executable(fuzz_${f}-standalone harnesses/fuzz_${f}.cc
            $<TARGET_OBJECTS:lsan_off> $<TARGET_OBJECTS:standalone_main>)
        target_link_libraries(fuzz_${f}-standalone PRIVATE t2d_boot)
    endforeach()
endif()
