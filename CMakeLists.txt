cmake_minimum_required(VERSION 3.20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(fps_geometry_engine VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "Build tests" ON)

include(FetchContent)


# nlohmann/json
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# raylib
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.0
)
FetchContent_MakeAvailable(raylib)



add_library(engine
  src/core/Map.cpp
  src/io/SceneIO.cpp
  src/analysis/ReachabilityAnalyzer.cpp
  src/analysis/ExposureAnalyzer.cpp
  src/analysis/VisibilityAnalyzer.cpp
  src/analysis/SceneAnalyzer.cpp
)

target_include_directories(engine PUBLIC include)
target_link_libraries(engine PUBLIC nlohmann_json::nlohmann_json)

add_executable(fps_engine src/main.cpp)
add_executable(example_open examples/example_open.cpp)
target_link_libraries(example_open PRIVATE engine)

add_executable(example_wall examples/example_wall.cpp)
target_link_libraries(example_wall PRIVATE engine)

target_link_libraries(fps_engine PRIVATE engine)

add_executable(fps_viewer viewer/main.cpp)
target_link_libraries(fps_viewer PRIVATE engine raylib)


if(BUILD_TESTING)
  enable_testing()

  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
  )
  FetchContent_MakeAvailable(Catch2)

add_executable(unit_tests
  tests/test_reachability.cpp
  tests/test_exposure.cpp
  tests/test_visibility.cpp
  tests/test_scene_analyzer.cpp
)

  target_link_libraries(unit_tests PRIVATE engine Catch2::Catch2WithMain)

  include(Catch)
  catch_discover_tests(unit_tests)
endif()
