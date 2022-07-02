cmake_minimum_required(VERSION 3.16)
project(RandomUniq CXX)
set(CMAKE_CXX_STANDARD 20)

set(ENABLE_TESTING CACHE ON BOOL "")

add_subdirectory(3rdparty/gsl)
add_subdirectory(3rdparty/range)
if (ENABLE_TESTING)
  add_subdirectory(3rdparty/gtest)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-fconcepts)
endif ()

add_library(${PROJECT_NAME}_impl STATIC
  include/randomUniq/util/RandomDevice.hpp
  include/randomUniq/util/minmaxException.hpp

  include/randomUniq/UniformIntDistributionUniq.hpp
  include/randomUniq/UniformIntDistribution.hpp
  )
target_link_libraries(${PROJECT_NAME}_impl PUBLIC
  Microsoft.GSL::GSL
  range-v3::range-v3
  )
target_include_directories(${PROJECT_NAME}_impl PUBLIC
  include
  )
set_target_properties(${PROJECT_NAME}_impl PROPERTIES
  LINKER_LANGUAGE CXX
  )
target_compile_definitions(${PROJECT_NAME}_impl PUBLIC
  PROJECT_NAME_RandomUniq=${PROJECT_NAME}
  )

if (ENABLE_TESTING)
  add_executable(${PROJECT_NAME}_impl_ut
    ut/smoke.cpp
    )
  target_link_libraries(${PROJECT_NAME}_impl_ut PRIVATE
    GTest::gtest_main
    ${PROJECT_NAME}_impl
    )
endif ()

set(QT_VERSION_MAJOR 5)
set(${PROJECT_NAME}_plot_REQUIRED_LIBS Core Widgets PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${${PROJECT_NAME}_plot_REQUIRED_LIBS})
if (Qt${QT_VERSION_MAJOR}_FOUND)
  message(STATUS "Qt ${Qt${QT_VERSION_MAJOR}_VERSION} found")

  add_library(${PROJECT_NAME}_plot_impl STATIC
    example/util/rangeScaleView.hpp
    example/util/rangeIntegrate.hpp
    example/util/generator.hpp
    example/util/SharedFromInstance.hpp
    example/util/RangeScaler.hpp example/util/src/RangeScaler.cpp
    )
  target_link_libraries(${PROJECT_NAME}_plot_impl PUBLIC
    range-v3::range-v3
    )
  target_include_directories(${PROJECT_NAME}_plot_impl PUBLIC
    ${PROJECT_SOURCE_DIR}
    )

  set(${PROJECT_NAME}_plot_REQUIRED_LIBS_QUALIFIED)
  foreach (lib ${${PROJECT_NAME}_plot_REQUIRED_LIBS})
    list(APPEND ${PROJECT_NAME}_plot_REQUIRED_LIBS_QUALIFIED Qt${QT_VERSION_MAJOR}::${lib})
  endforeach ()

  find_package(Boost REQUIRED)

  include(FetchContent)
  set(FETCHCONTENT_QUIET OFF)
  FetchContent_Declare(qcustomplot
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
  target_compile_options(qcustomplot PRIVATE
    -Wno-deprecated-declarations
    -Wno-deprecated-enum-enum-conversion
    )
  set_target_properties(qcustomplot PROPERTIES
    AUTOMOC ON
    )

  add_executable(${PROJECT_NAME}_plot
    example/plot.cpp
    )
  target_link_libraries(${PROJECT_NAME}_plot PRIVATE
    qcustomplot
    Boost::boost
    ${PROJECT_NAME}_impl
    ${PROJECT_NAME}_plot_impl
    )
  target_compile_definitions(${PROJECT_NAME}_plot
    PUBLIC
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:BOOST_MULTI_INDEX_ENABLE_SAFE_MODE BOOST_MULTI_INDEX_INVARIANT_ASSERT>
    PRIVATE
    BOOST_HANA_CONFIG_ENABLE_STRING_UDL
    )
  set_target_properties(${PROJECT_NAME}_plot PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTOUIC_SEARCH_PATHS example
    INCLUDE_CURRENT_DIR ON
    $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:INTERPROCEDURAL_OPTIMIZATION ON>
    )
  if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    include(cmake/strip_binary.cmake)
    strip_output_binary(${PROJECT_NAME}_plot)

    include(cmake/compress_7z_output_binary.cmake)
    compress_7z_output_binary(${PROJECT_NAME}_plot)
  endif ()

  if (ENABLE_TESTING)
    add_executable(${PROJECT_NAME}_plot_impl_ut
      example/util/ut/ut.cpp
      )
    target_link_libraries(${PROJECT_NAME}_plot_impl_ut PUBLIC
      GTest::gtest_main
      ${PROJECT_NAME}_impl
      ${PROJECT_NAME}_plot_impl
      )
  endif ()
endif ()