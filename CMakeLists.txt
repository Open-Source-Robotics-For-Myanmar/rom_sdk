cmake_minimum_required(VERSION 3.16...3.28)

project(rom
  VERSION 0.1.2
  DESCRIPTION "ROM Robotics C++17 SDK"
  LANGUAGES CXX)

# ---- Options ----------------------------------------------------------------
option(ROM_BUILD_TESTS "Build unit tests" ON)
option(BUILD_SHARED_LIBS "Build libraries as shared (.so)" ON)

# Reserved for future per-CPU builds (intel, rockchip). Not used yet.
set(ROM_TARGET_CPU "" CACHE STRING "Target CPU: intel or rockchip")

# ---- Global settings --------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# The SDK is always installed under /opt/rom:
#   /opt/rom/include/rom/...   public headers
#   /opt/rom/lib/              librom_*.so and lib/cmake/rom (find_package)
set(CMAKE_INSTALL_PREFIX "/opt/rom" CACHE PATH "Install prefix (fixed)" FORCE)
set(CMAKE_INSTALL_INCLUDEDIR "include" CACHE PATH "" FORCE)
set(CMAKE_INSTALL_LIBDIR "lib" CACHE PATH "" FORCE)
include(GNUInstallDirs)

# ---- Libraries --------------------------------------------------------------
find_package(Threads REQUIRED)

# rom_add_library(<name> <sources>...) creates target <name>, alias
# rom::<name> and output file librom_<name>.so with shared SDK settings.
function(rom_add_library name)
  add_library(${name} ${ARGN})
  add_library(rom::${name} ALIAS ${name})
  target_compile_features(${name} PUBLIC cxx_std_17)
  target_compile_options(${name} PRIVATE -Wall -Wextra -Wpedantic)
  # Include path so that <rom/...> works from both the build and install tree.
  target_include_directories(${name} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  set_target_properties(${name} PROPERTIES
    OUTPUT_NAME rom_${name}
    SOVERSION ${PROJECT_VERSION_MAJOR})
endfunction()

# rom::tools - namespace rom::tools (cout, cin, Color, Style).
rom_add_library(tools
  rom/tools/InStream.cc
  rom/tools/Line.cc
  rom/tools/OutStream.cc
  rom/tools/Style.cc)
target_link_libraries(tools PUBLIC Threads::Threads)

# rom::robotics - namespace rom::robotics.
rom_add_library(robotics
  rom/robotics/control/pid/PidController.cc
  rom/robotics/guidance/png/PngGuidance.cc)

# rom::intelligence - namespace rom::intelligence.
rom_add_library(intelligence
  rom/intelligence/model/rl/bc_mlp_v4_ppo/BcMlpV4Ppo.cc)

# ---- Install ----------------------------------------------------------------
install(TARGETS tools robotics intelligence
  EXPORT romTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install public headers one by one, preserving the rom/... layout. Installing
# per file (instead of install(DIRECTORY)) avoids creating empty namespace
# directories and skips private headers under internal/.
file(GLOB_RECURSE ROM_PUBLIC_HEADERS
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rom/*.h)
list(FILTER ROM_PUBLIC_HEADERS EXCLUDE REGEX "/internal/")
foreach(header IN LISTS ROM_PUBLIC_HEADERS)
  get_filename_component(header_dir ${header} DIRECTORY)
  install(FILES ${header} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${header_dir})
endforeach()

install(EXPORT romTargets
  FILE romTargets.cmake
  NAMESPACE rom::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rom)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/romConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/romConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rom)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/romConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/romConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/romConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rom)

# ---- Uninstall (make uninstall) ---------------------------------------------
if(NOT TARGET uninstall)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    @ONLY)
  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

# ---- Tests ------------------------------------------------------------------
if(ROM_BUILD_TESTS)
  enable_testing()
  find_package(GTest REQUIRED)
  include(GoogleTest)

  add_executable(rom_tests
    rom/tools/InStreamTest.cc
    rom/tools/OutStreamTest.cc
    rom/tools/StyleTest.cc
    rom/robotics/control/pid/PidControllerTest.cc
    rom/robotics/guidance/png/PngGuidanceTest.cc
    rom/intelligence/model/rl/bc_mlp_v4_ppo/BcMlpV4PpoTest.cc)
  target_link_libraries(rom_tests PRIVATE rom::tools rom::robotics rom::intelligence GTest::gtest_main)
  gtest_discover_tests(rom_tests)
endif()
