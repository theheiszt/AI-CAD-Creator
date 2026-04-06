cmake_minimum_required(VERSION 3.21)
project(AiCadCreator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

option(AICAD_ENABLE_OCCT "Enable Open CASCADE integration" ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(OCCT_INCLUDE_DIR "" CACHE PATH "Path to OCCT include directory")
set(OCCT_LIBRARY_DIR "" CACHE PATH "Path to OCCT library directory")

add_executable(AiCadCreator
  src/main.cpp
  src/app/CadApplication.cpp
  src/document/OcafDocument.cpp
  src/viewer/ViewerWindow.cpp
  src/viewer/OcctViewport.cpp
  src/features/FeatureBase.cpp
  src/features/ExtrudeFeature.cpp
  src/ai/CadIrDocument.cpp
  src/geom/GeometryOps.cpp
  src/selectors/SelectorTypes.cpp
  src/selectors/ShapeSelectors.cpp
)

target_include_directories(AiCadCreator PRIVATE
  src
)

target_link_libraries(AiCadCreator PRIVATE Qt6::Widgets)

if (AICAD_ENABLE_OCCT)
  if (NOT EXISTS "${OCCT_INCLUDE_DIR}")
    message(FATAL_ERROR "OCCT_INCLUDE_DIR is not set or does not exist")
  endif()

  if (NOT EXISTS "${OCCT_LIBRARY_DIR}")
    message(FATAL_ERROR "OCCT_LIBRARY_DIR is not set or does not exist")
  endif()

  target_include_directories(AiCadCreator PRIVATE "${OCCT_INCLUDE_DIR}")
  target_link_directories(AiCadCreator PRIVATE "${OCCT_LIBRARY_DIR}")

  # Adjust this list to match the OCCT libraries available on your system.
  target_link_libraries(AiCadCreator PRIVATE
    TKernel
    TKMath
    TKG2d
    TKG3d
    TKTopAlgo
    TKGeomBase
    TKGeomAlgo
    TKBRep
    TKPrim
    TKBO
    TKBool
    TKFillet
    TKMesh
    TKXCAF
    TKCAF
    TKLCAF
    TKV3d
    TKOpenGl
    TKService
    TKXSBase
    TKSTEP
    TKSTEP209
    TKSTEPAttr
    TKSTEPBase
    TKShHealing
    TKMeshVS
    TKVCAF
    TKStdL
    TKStd
    TKBin
    TKBinL
    TKBinXCAF
  )

  target_compile_definitions(AiCadCreator PRIVATE AICAD_WITH_OCCT=1)
endif()
