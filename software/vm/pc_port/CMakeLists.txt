add_subdirectory(raylib)

add_executable(pc_port)

target_sources(pc_port
PRIVATE
    main.cpp
    RaylibGraphicsModule.hpp
    RaylibGraphicsModule.cpp
)

target_link_libraries(pc_port
PRIVATE
    engine
    raylib
)


add_executable(font_test font_test.cpp)
target_link_libraries(font_test PRIVATE raylib)