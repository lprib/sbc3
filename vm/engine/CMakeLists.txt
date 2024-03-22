add_library(engine)

target_sources(engine
PRIVATE
    BytecodeModule.cpp
    BytecodeModule.hpp
    IPlatform.hpp
    ISystemModule.hpp
    Machine.cpp
    Machine.hpp
    Stack.hpp
    engine_common.hpp
)

target_include_directories(engine PUBLIC .)