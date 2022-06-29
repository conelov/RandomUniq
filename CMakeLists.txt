cmake_minimum_required(VERSION 3.16)
project(RandomUniq CXX)
set(CMAKE_CXX_STANDARD 20)

set(ENABLE_TESTING CACHE ON BOOL "")

add_subdirectory(3rdparty/gsl)

add_library(${PROJECT_NAME}_impl STATIC
  include/randomUniq/util/RandomDevice.hpp

  include/randomUniq/UniformIntDistributionUniq.hpp include/randomUniq/src/UniformIntDistributionUniq.cpp
  include/randomUniq/UniformIntDistribution.hpp
  )
target_link_libraries(${PROJECT_NAME}_impl PUBLIC
  Microsoft.GSL::GSL
  )
target_include_directories(${PROJECT_NAME}_impl PUBLIC
  include
  3rdparty/range/include
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

if (ENABLE_TESTING)
  add_executable(${PROJECT_NAME}_test
    test/general.cpp
    )
  add_subdirectory(3rdparty/gtest)
  target_link_libraries(${PROJECT_NAME}_test PUBLIC
    ${PROJECT_NAME}_impl
    GTest::gtest_main
    )
endif ()

set(QT_VERSION_MAJOR 5)
set(${PROJECT_NAME}_plot_REQUIRED_LIBS Core Widgets PrintSupport)
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
  target_include_directories(qcustomplot PUBLIC
    3rdparty/qcustomplot
    )
  target_compile_definitions(qcustomplot PUBLIC
    QT_USE_QSTRINGBUILDER
    )
  set_target_properties(qcustomplot PROPERTIES
    AUTOMOC ON
    INCLUDE_CURRENT_DIR ON
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
    AUTOUIC ON
    AUTOUIC_SEARCH_PATHS example
    INCLUDE_CURRENT_DIR ON
    )
endif ()