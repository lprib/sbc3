add_library(engine)

target_sources(engine
PRIVATE
    BaseModuleCommon.hpp
    BytecodeModule.cpp
    BytecodeModule.hpp
    GraphicsModuleCommon.hpp
    GraphicsModuleCommon.cpp
    IPlatform.hpp
    ISystemModule.hpp
    Machine.cpp
    Machine.hpp
    Stack.hpp
    engine_common.cpp
    engine_common.hpp
)

target_include_directories(engine PUBLIC .)

target_compile_options(engine
PRIVATE
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:RelWithDebInfo>:-O3>
)