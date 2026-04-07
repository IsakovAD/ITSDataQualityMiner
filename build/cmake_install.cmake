# Install script for directory: /home/its/QC_QA_Team/A_Isakov/dev/ITSDataQualityMiner

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/its/QC_QA_Team/A_Isakov/dev/ITSDataQualityMiner/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "RelWithDebInfo")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/home/its/QC_QA_Team/sw/slc9_x86-64/GCC-Toolchain/v14.2.0-alice2-1/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libITSDataQualityMinerLib.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libITSDataQualityMinerLib.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libITSDataQualityMinerLib.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/its/QC_QA_Team/A_Isakov/dev/ITSDataQualityMiner/build/libITSDataQualityMinerLib.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libITSDataQualityMinerLib.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libITSDataQualityMinerLib.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libITSDataQualityMinerLib.so"
         OLD_RPATH "/home/its/QC_QA_Team/sw/slc9_x86-64/O2/dev-local5/lib:/home/its/QC_QA_Team/sw/slc9_x86-64/QualityControl/master-local7/lib:/home/its/QC_QA_Team/sw/slc9_x86-64/JAliEn-ROOT/0.7.14-58/lib:/home/its/QC_QA_Team/sw/slc9_x86-64/ROOT/v6-32-06-alice9-3/lib:/home/its/QC_QA_Team/sw/slc9_x86-64/boost/v1.83.0-alice2-44/lib:/home/its/QC_QA_Team/sw/slc9_x86-64/FairMQ/v1.10.0-3/lib:/home/its/QC_QA_Team/sw/slc9_x86-64/FairLogger/v2.1.0-10/lib:/home/its/QC_QA_Team/sw/slc9_x86-64/fmt/11.1.2-7/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/its/QC_QA_Team/sw/slc9_x86-64/GCC-Toolchain/v14.2.0-alice2-1/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libITSDataQualityMinerLib.so")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/its-dqm" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/its-dqm")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/its-dqm"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "/home/its/QC_QA_Team/A_Isakov/dev/ITSDataQualityMiner/build/its-dqm")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/its-dqm" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/its-dqm")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/its-dqm"
         OLD_RPATH "/home/its/QC_QA_Team/sw/slc9_x86-64/O2/dev-local5/lib:/home/its/QC_QA_Team/sw/slc9_x86-64/QualityControl/master-local7/lib:/home/its/QC_QA_Team/sw/slc9_x86-64/JAliEn-ROOT/0.7.14-58/lib:/home/its/QC_QA_Team/A_Isakov/dev/ITSDataQualityMiner/build:/home/its/QC_QA_Team/sw/slc9_x86-64/ROOT/v6-32-06-alice9-3/lib:/home/its/QC_QA_Team/sw/slc9_x86-64/boost/v1.83.0-alice2-44/lib:/home/its/QC_QA_Team/sw/slc9_x86-64/FairMQ/v1.10.0-3/lib:/home/its/QC_QA_Team/sw/slc9_x86-64/FairLogger/v2.1.0-10/lib:/home/its/QC_QA_Team/sw/slc9_x86-64/fmt/11.1.2-7/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/its/QC_QA_Team/sw/slc9_x86-64/GCC-Toolchain/v14.2.0-alice2-1/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/its-dqm")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE DIRECTORY FILES "/home/its/QC_QA_Team/A_Isakov/dev/ITSDataQualityMiner/include/")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/its/QC_QA_Team/A_Isakov/dev/ITSDataQualityMiner/build/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/its/QC_QA_Team/A_Isakov/dev/ITSDataQualityMiner/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
