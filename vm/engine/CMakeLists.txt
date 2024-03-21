add_library(engine)

target_sources(engine
PRIVATE
    Module.cpp
    Module.hpp
    Machine.cpp
    Machine.hpp
    engine_common.hpp
)

target_include_directories(engine PUBLIC .)