cmake_minimum_required(VERSION 3.18)
project(ITSDataQualityMiner CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Find dependencies ────────────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH $ENV{O2_ROOT}/lib/cmake/O2)


find_package(ROOT REQUIRED COMPONENTS Core Hist Graf)
find_package(fmt REQUIRED)  
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(Boost 1.58 REQUIRED COMPONENTS container program_options system log)
find_package(FairLogger REQUIRED)
find_package(FairMQ REQUIRED)   # restored
find_package(JAliEnROOT MODULE)
find_package(SQLite3 REQUIRED)




# ── Collect sources ─────────────────────────────────────────────────
set(LIB_SOURCES
    src/ITSGeometry.cpp
    src/DeadMapDecoder.cpp
    src/CCDBServer.cpp 
    src/DataQualityProcessor.cxx
    src/AssyncProcessor.cxx
    src/Helpers.cxx
)

# ── Build shared library ────────────────────────────────────────────
add_library(ITSDataQualityMinerLib SHARED ${LIB_SOURCES})

target_include_directories(ITSDataQualityMinerLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    $ENV{O2_ROOT}/include
    $ENV{O2_ROOT}/include/GPU
)

target_link_directories(ITSDataQualityMinerLib PUBLIC
    $ENV{O2_ROOT}/lib
    $ENV{QUALITYCONTROL_ROOT}/lib
)


target_link_libraries(ITSDataQualityMinerLib PUBLIC
    ROOT::Core ROOT::Hist ROOT::Graf
    fmt::fmt
    Boost::container
    Boost::program_options
    Boost::system
    Microsoft.GSL::GSL
    FairLogger::FairLogger
    SQLite::SQLite3
    FairMQ::FairMQ          
    JAliEn::JAliEn
    -lO2CCDB
    -lO2DataFormatsITSMFT
    -lO2DetectorsBase
    -lO2ITSBase
    -lO2ITSMFTBase

 
)

# ── Build executable ───────────────────────────────────────────────
add_executable(its-dqm apps/main.cxx)
add_executable(its-ccdb-db-test apps/test_database.cxx)
add_executable(its-qa-qc apps/AssyncQA.cxx)
target_link_libraries(its-dqm PRIVATE ITSDataQualityMinerLib)
target_link_libraries(its-ccdb-db-test PRIVATE ITSDataQualityMinerLib)
target_link_libraries(its-qa-qc PRIVATE ITSDataQualityMinerLib)
# ── Install rules ──────────────────────────────────────────────────
install(TARGETS ITSDataQualityMinerLib its-dqm its-qa-qc
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)

# ── Print summary ──────────────────────────────────────────────────
message(STATUS "")
message(STATUS "ITSDataQualityMiner configuration:")
message(STATUS "  Build type    : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  C++ standard  : ${CMAKE_CXX_STANDARD}")
message(STATUS "")