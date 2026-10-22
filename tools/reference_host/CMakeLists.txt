cmake_minimum_required(VERSION 3.22)
project(DigiMDOriginalReference LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DIGIMD_MACHINEMODULE_ROOT "" CACHE PATH "Pinned sd88me/mpc-vst-machinedrum checkout")
if(NOT EXISTS "${DIGIMD_MACHINEMODULE_ROOT}/engine/VoiceEngine.cpp" OR
   NOT EXISTS "${DIGIMD_MACHINEMODULE_ROOT}/libs/dsp56300/source/dsp56kEmu/dsp.h" OR
   NOT EXISTS "${DIGIMD_MACHINEMODULE_ROOT}/libs/gearmulator-md-mm/source/mc68k/CMakeLists.txt")
  message(FATAL_ERROR "Machinemodule or emulator submodules absent; see tools/build_md_reference.py")
endif()

# Reuse upstream's exact original-firmware loader, MachineRunner and VoiceEngine.
# This does NOT create a synthetic or approximate audio implementation.
add_subdirectory("${DIGIMD_MACHINEMODULE_ROOT}" "${CMAKE_BINARY_DIR}/machinemodule" EXCLUDE_FROM_ALL)

foreach(name IN ITEMS mdhost mdvoice)
  add_executable(digimd_${name} "${DIGIMD_MACHINEMODULE_ROOT}/tools/${name}/${name}.cpp")
  target_include_directories(digimd_${name} PRIVATE
    "${DIGIMD_MACHINEMODULE_ROOT}/libs/dsp56300/source")
  target_link_libraries(digimd_${name} PRIVATE mdcore)
  if(UNIX)
    target_link_libraries(digimd_${name} PRIVATE pthread ${CMAKE_DL_LIBS})
  endif()
endforeach()

add_executable(digimd_mdsequence "${CMAKE_CURRENT_SOURCE_DIR}/mdsequence.cpp")
target_include_directories(digimd_mdsequence PRIVATE
  "${DIGIMD_MACHINEMODULE_ROOT}"
  "${DIGIMD_MACHINEMODULE_ROOT}/libs/dsp56300/source")
target_link_libraries(digimd_mdsequence PRIVATE mdcore)
if(UNIX)
  target_link_libraries(digimd_mdsequence PRIVATE pthread ${CMAKE_DL_LIBS})
endif()

add_executable(digimd_mdsine "${CMAKE_CURRENT_SOURCE_DIR}/mdsine.cpp")
target_include_directories(digimd_mdsine PRIVATE
  "${DIGIMD_MACHINEMODULE_ROOT}"
  "${DIGIMD_MACHINEMODULE_ROOT}/libs/dsp56300/source")
target_link_libraries(digimd_mdsine PRIVATE mdcore)
if(UNIX)
  target_link_libraries(digimd_mdsine PRIVATE pthread ${CMAKE_DL_LIBS})
endif()

# Independent TRX-SD executable oracle: original CPU + full 64-word DSP slot.
add_executable(digimd_mdsdsequence "${CMAKE_CURRENT_SOURCE_DIR}/mdsdsequence.cpp")
target_include_directories(digimd_mdsdsequence PRIVATE
  "${DIGIMD_MACHINEMODULE_ROOT}" "${MD_DSP56300_DIR}/source")
target_link_libraries(digimd_mdsdsequence PRIVATE mdcore)
if(UNIX)
  target_link_libraries(digimd_mdsdsequence PRIVATE pthread ${CMAKE_DL_LIBS})
endif()
