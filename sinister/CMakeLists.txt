# Sinister - DWARF stack machine tooling.
project(sinister)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(SINISTER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SINISTER_INCLUDE_DIR "${SINISTER_SOURCE_DIR}/include")
set(LLDB_INCLUDE_DIR "${SINISTER_SOURCE_DIR}/../lldb/include")

# Must go below project(..)
include(GNUInstallDirs)

include_directories(BEFORE ${SINISTER_INCLUDE_DIR} ${LLDB_INCLUDE_DIR})

# set(LINK_LIBRARIES LLDB LLVM)

add_llvm_executable(sinister
    lib/sinister/DwarfExecutor.cpp
    lib/sinister/sinister.cpp
    lib/sinister/TextToDwarf.cpp
)
target_link_libraries(sinister PRIVATE lldbCore)

# target_link_libraries(sinister LINK_LIBS LLDB)
