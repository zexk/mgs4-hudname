cmake_minimum_required(VERSION 3.20)
project(hudname VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Log lines carry file paths, which the default codepage of an MSVC build mangles.
if(MSVC)
  add_compile_options(/utf-8)
endif()

if(NOT WIN32)
  message(FATAL_ERROR "ASI target only: build with `nix build` or a mingw-w64 toolchain")
endif()

include(FetchContent)
set(MINHOOK_DIR "$ENV{minhook}" CACHE PATH "Path to MinHook sources")
if(NOT MINHOOK_DIR)
  FetchContent_Declare(minhook_source
    GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
    GIT_TAG v1.3.4
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(minhook_source)
  set(MINHOOK_DIR "${minhook_source_SOURCE_DIR}")
endif()

add_library(minhook STATIC
  ${MINHOOK_DIR}/src/buffer.c
  ${MINHOOK_DIR}/src/hook.c
  ${MINHOOK_DIR}/src/trampoline.c
  ${MINHOOK_DIR}/src/hde/hde64.c)
target_include_directories(minhook PUBLIC ${MINHOOK_DIR}/include)
target_include_directories(minhook PRIVATE ${MINHOOK_DIR}/src ${MINHOOK_DIR}/src/hde)

if(MINGW)
  # The game dir has no libstdc++-6.dll, libgcc_s_seh-1.dll or libmcfgthread-2.dll:
  # without full -static the ASI fails to load with a missing-DLL error.
  # PE auto-export of every global bloats .edata and pins dead code; DllMain is the
  # only export needed.
  add_compile_options(-ffunction-sections -fdata-sections)
  add_link_options(-s -static -static-libgcc -static-libstdc++
                   -Wl,--exclude-all-symbols -Wl,--gc-sections)
endif()

add_library(hudname SHARED
  src/dllmain.cpp
  src/config.cpp
  src/hooks.cpp
  src/log.cpp)
set_target_properties(hudname PROPERTIES PREFIX "" SUFFIX ".asi")
target_include_directories(hudname PRIVATE src)
target_link_libraries(hudname PRIVATE minhook)
target_compile_definitions(hudname PRIVATE MOD_NAME="${PROJECT_NAME}" MOD_VERSION="${PROJECT_VERSION}")

install(TARGETS hudname RUNTIME DESTINATION .)
install(FILES hudname.ini DESTINATION .)
