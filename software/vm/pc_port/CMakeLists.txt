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
