add_library(engine)

target_sources(engine
PRIVATE
    vm.cpp
    vm.hpp
)

target_include_directories(engine PUBLIC .)