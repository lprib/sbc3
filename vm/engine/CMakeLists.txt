add_library(engine)

target_sources(engine
PRIVATE
    engine.cpp
    engine.hpp
)

target_include_directories(engine PUBLIC .)