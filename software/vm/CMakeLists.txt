add_subdirectory(engine)

if(NOT CMAKE_CROSSCOMPILING)
   # Don't try to build these for baremetal
   add_subdirectory(pc_port)
   add_subdirectory(tests)
endif()