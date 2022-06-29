cmake_minimum_required(VERSION 3.16)
project(RandomUniq CXX)
set(CMAKE_CXX_STANDARD 20)

add_library(${PROJECT_NAME}_impl STATIC
  include/randomUniq/UniformIntDistributionUniq.hpp
  include/randomUniq/util/RandomDevice.hpp
  )
target_include_directories(${PROJECT_NAME}_impl PUBLIC
  include
  )
set_target_properties(${PROJECT_NAME}_impl PROPERTIES
  LINKER_LANGUAGE CXX
  )

add_executable(${PROJECT_NAME}_touch
  example/touch.cpp
  )
target_link_libraries(${PROJECT_NAME}_touch PUBLIC
  ${PROJECT_NAME}_impl
  )

set(QT_VERSION_MAJOR 5)
set(${PROJECT_NAME}_plot_REQUIRED_LIBS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${${PROJECT_NAME}_plot_REQUIRED_LIBS})
if (Qt${QT_VERSION_MAJOR}_FOUND)
  message(STATUS "Qt${QT_VERSION_MAJOR} Found")

  set(${PROJECT_NAME}_plot_REQUIRED_LIBS_QUALIFIED)
  foreach (lib ${${PROJECT_NAME}_plot_REQUIRED_LIBS})
    list(APPEND ${PROJECT_NAME}_plot_REQUIRED_LIBS_QUALIFIED Qt${QT_VERSION_MAJOR}::${lib})
  endforeach ()

  include(FetchContent)
  set(FETCHCONTENT_QUIET OFF)
  FetchContent_Declare(
    qcustomplot
    URL https://www.qcustomplot.com/release/2.1.0fixed/QCustomPlot-source.tar.gz
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/3rdparty/qcustomplot
  )
  FetchContent_MakeAvailable(qcustomplot)
  add_library(qcustomplot STATIC
    ${PROJECT_SOURCE_DIR}/3rdparty/qcustomplot/qcustomplot.h
    ${PROJECT_SOURCE_DIR}/3rdparty/qcustomplot/qcustomplot.cpp
    )
  target_link_libraries(qcustomplot PUBLIC
    ${${PROJECT_NAME}_plot_REQUIRED_LIBS_QUALIFIED}
    )
  set_target_properties(qcustomplot PROPERTIES
    AUTOMOC ON
    INCLUDE_CURRENT_DIR ON
    INTERPROCEDURAL_OPTIMIZATION ON
    )
  target_compile_definitions(qcustomplot PUBLIC
    QT_USE_QSTRINGBUILDER
    )

  add_executable(${PROJECT_NAME}_plot
    example/plot.cpp
    )
  target_link_libraries(${PROJECT_NAME}_plot PUBLIC
    ${PROJECT_NAME}_impl
    qcustomplot
    )
  target_include_directories(${PROJECT_NAME}_plot PUBLIC
    3rdparty
    )
  set_target_properties(${PROJECT_NAME}_plot PROPERTIES
    AUTOMOC ON
    INCLUDE_CURRENT_DIR ON
    INTERPROCEDURAL_OPTIMIZATION ON
    )
endif ()