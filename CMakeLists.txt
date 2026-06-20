cmake_minimum_required(VERSION 3.20)
project(CineCPP VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=x86-64-v3")

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(SDL2 sdl2 QUIET)
  pkg_check_modules(FFMPEG libavformat libavcodec libavutil QUIET)
endif()

add_library(cinecpp_core
  src/input_manager.cpp
  src/timeline_engine.cpp
  src/keybinding_engine.cpp
  src/ffmpeg_reader.cpp
)
target_include_directories(cinecpp_core PUBLIC include)

if(NOT FFMPEG_FOUND)
  target_compile_definitions(cinecpp_core PUBLIC CINECPP_NO_FFMPEG)
endif()

add_executable(cinecpp src/main.cpp)
target_link_libraries(cinecpp cinecpp_core)

if(SDL2_FOUND)
  find_package(OpenGL REQUIRED)

  set(IMGUI_SRC
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/imgui_demo.cpp
    third_party/imgui/backends/imgui_impl_sdl2.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
  )

  add_executable(cinecpp-gui src/main.cpp src/gui.cpp ${IMGUI_SRC})
  target_compile_definitions(cinecpp-gui PRIVATE HAS_SDL2)
  target_include_directories(cinecpp-gui PRIVATE third_party/imgui)
  target_include_directories(cinecpp-gui PRIVATE ${SDL2_INCLUDE_DIRS})
  target_link_directories(cinecpp-gui PRIVATE ${SDL2_LIBRARY_DIRS})
  target_link_libraries(cinecpp-gui cinecpp_core ${SDL2_LIBRARIES} OpenGL::GL)

  if(FFMPEG_FOUND)
    target_compile_definitions(cinecpp-gui PRIVATE HAS_FFMPEG)
    target_include_directories(cinecpp-gui PRIVATE ${FFMPEG_INCLUDE_DIRS})
    target_link_directories(cinecpp-gui PRIVATE ${FFMPEG_LIBRARY_DIRS})
    target_link_libraries(cinecpp-gui PRIVATE ${FFMPEG_LIBRARIES})
  endif()
endif()

add_executable(test_input_manager tests/test_input_manager.cpp src/input_manager.cpp src/keybinding_engine.cpp)
target_include_directories(test_input_manager PRIVATE include)

add_executable(test_timeline tests/test_timeline.cpp src/timeline_engine.cpp)
target_include_directories(test_timeline PRIVATE include)
