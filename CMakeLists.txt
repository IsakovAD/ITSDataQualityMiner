cmake_minimum_required(VERSION 3.18)
project(ITSDataQualityMiner CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Find dependencies ────────────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH $ENV{O2_ROOT}/lib/cmake/O2)
#include($ENV{O2_ROOT}/lib/cmake/O2/O2Targets.cmake )


find_package(ROOT REQUIRED COMPONENTS Core Hist Graf)
find_package(fmt REQUIRED)  
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(Boost 1.58 REQUIRED COMPONENTS container program_options system log)
find_package(FairLogger REQUIRED)
find_package(FairMQ REQUIRED)   # restored
find_package(JAliEnROOT MODULE)
#find_package(Vc REQUIRED)
#find_package(FairRoot REQUIRED)
#find_package(Boost REQUIRED)
#find_package(MC REQUIRED)



#set(CMAKE_PREFIX_PATH "/home/its/QC_QA_Team/sw/slc9_x86-64/O2/dev-local5")
#set(CMAKE_PREFIX_PATH "/home/its/QC_QA_Team/sw/slc9_x86-64/O2/install")
#find_package(O2 REQUIRED COMPONENTS ITSBase)

#find_package(Gandiva MODULE)
# ── Collect sources ─────────────────────────────────────────────────
set(LIB_SOURCES
    src/ITSGeometry.cpp
    src/DeadMapDecoder.cpp
    src/CCDBServer.cpp 
    src/DataQualityProcessor.cxx
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
    FairMQ::FairMQ          
    JAliEn::JAliEn
    -lO2CCDB
    -lO2DataFormatsITSMFT
    #-lO2CommonConstants
    -lO2DetectorsBase
    -lO2ITSBase
    -lO2ITSMFTBase
    #O2::ITSBase

 
)

# ── Build executable ───────────────────────────────────────────────
add_executable(its-dqm apps/main.cxx)
target_link_libraries(its-dqm PRIVATE ITSDataQualityMinerLib)

# ── Install rules ──────────────────────────────────────────────────
install(TARGETS ITSDataQualityMinerLib its-dqm
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