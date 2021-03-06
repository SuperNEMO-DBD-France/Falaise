# CMAKE_COMPILER
# - Basic CMake setup
# Check version meets ou requirements
# Declare project, which will configure compiler for us
cmake_minimum_required(VERSION 2.8)
project(FalaiseDigitizationPlugin)
set(FalaiseDIGITIZATION_VERSION_MAJOR 0)
set(FalaiseDIGITIZATION_VERSION_MINOR 1)
set(FalaiseDIGITIZATION_VERSION_PATCH 0)
set(FalaiseDIGITIZATION_VERSION_REVISION 0)

# - ROOT
# find_package(ROOT 5.34.00 REQUIRED Gui Geom GeomPainter Physics)
set(ROOT_COMPONENTS Gui Gpad Geom GeomPainter Physics Graf Graf3d)
find_package(ROOT 5.34.00 QUIET COMPONENTS ${ROOT_COMPONENTS} NO_MODULE)
find_package(ROOT 5.34.00 REQUIRED ${ROOT_COMPONENTS})

# Ensure our code can see the Falaise headers
#include_directories(${Falaise_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/source)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/source/falaise)
include_directories(${Falaise_BUILDPRODUCT_DIR}/include)
include_directories(${Falaise_BUILDPRODUCT_DIR}/include/falaise)
include_directories(${Bayeux_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source)
############################################################################################
# - Digitization modules:
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/falaise)

# - Prepare files from skelettons:
configure_file(source/falaise/snemo/digitization/version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/digitization/version.h
  @ONLY)

# - Headers:

list(APPEND FalaiseDigitizationPlugin_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/source/falaise/snemo/digitization/version.h
  source/falaise/snemo/digitization/electronic_mapping.h
  source/falaise/snemo/digitization/clock_utils.h
  source/falaise/snemo/digitization/ID_convertor.h
  source/falaise/snemo/digitization/mapping.h
  source/falaise/snemo/digitization/trigger_structures.h
  source/falaise/snemo/digitization/trigger_info.h
  source/falaise/snemo/digitization/trigger_display_manager.h
  source/falaise/snemo/digitization/tracker_zone.h
  source/falaise/snemo/digitization/tracker_sliding_zone.h

  source/falaise/snemo/digitization/calo_signal.h
  source/falaise/snemo/digitization/calo_signal_simple_shape.h
  source/falaise/snemo/digitization/calo_feb_data.h
  source/falaise/snemo/digitization/calo_tp.h
  source/falaise/snemo/digitization/calo_tp_data.h
  source/falaise/snemo/digitization/calo_ctw.h
  source/falaise/snemo/digitization/calo_ctw_data.h
  source/falaise/snemo/digitization/calo_tp_to_ctw_algo.h
  source/falaise/snemo/digitization/calo_tp.ipp
  source/falaise/snemo/digitization/calo_tp_data.ipp
  source/falaise/snemo/digitization/calo_ctw.ipp
  source/falaise/snemo/digitization/calo_ctw_data.ipp

  source/falaise/snemo/digitization/geiger_signal.h
  source/falaise/snemo/digitization/geiger_tp.h
  source/falaise/snemo/digitization/geiger_tp_data.h
  source/falaise/snemo/digitization/geiger_tp.h
  source/falaise/snemo/digitization/tracker_feb_data.h
  source/falaise/snemo/digitization/geiger_tp_data.h
  source/falaise/snemo/digitization/geiger_ctw.h
  source/falaise/snemo/digitization/geiger_ctw_data.h
  source/falaise/snemo/digitization/geiger_tp_to_ctw_algo.h
  source/falaise/snemo/digitization/geiger_tp.ipp
  source/falaise/snemo/digitization/geiger_tp_data.ipp
  source/falaise/snemo/digitization/geiger_ctw.ipp
  source/falaise/snemo/digitization/geiger_ctw_data.ipp

  source/falaise/snemo/digitization/signal_data.h

  source/falaise/snemo/digitization/sd_to_calo_signal_algo.h
  source/falaise/snemo/digitization/sd_to_geiger_signal_algo.h
  source/falaise/snemo/digitization/signal_to_geiger_tp_algo.h
  source/falaise/snemo/digitization/signal_to_calo_tp_algo.h
  source/falaise/snemo/digitization/tracker_trigger_algorithm.h
  source/falaise/snemo/digitization/tracker_trigger_algorithm_test_time.h
  source/falaise/snemo/digitization/calo_trigger_algorithm.h
  source/falaise/snemo/digitization/calo_trigger_algorithm_test_time.h
  source/falaise/snemo/digitization/coincidence_trigger_algorithm.h
  source/falaise/snemo/digitization/coincidence_trigger_algorithm_test_time.h
  source/falaise/snemo/digitization/trigger_algorithm.h
  source/falaise/snemo/digitization/trigger_algorithm_test_time.h

  source/falaise/snemo/digitization/fake_trigger_algo.h
  source/falaise/snemo/digitization/memory.h
  source/falaise/snemo/digitization/memory-inl.h
  source/falaise/snemo/digitization/tracker_trigger_mem_maker.h

  source/falaise/snemo/digitization/fake_trigger_module.h
  source/falaise/snemo/digitization/digitization_module.h
  source/falaise/snemo/digitization/digitization_driver.h
  source/falaise/snemo/digitization/fake_trigger_cut.h

  source/falaise/snemo/digitization/the_serializable.ipp
  source/falaise/snemo/digitization/the_serializable.h
  )

# - Sources:
list(APPEND FalaiseDigitizationPlugin_SOURCES
  source/falaise/snemo/digitization/version.cc
  source/falaise/snemo/digitization/tracker_trigger_mem_maker.cc
  source/falaise/snemo/digitization/trigger_algorithm.cc
  source/falaise/snemo/digitization/trigger_algorithm_test_time.cc
  source/falaise/snemo/digitization/fake_trigger_cut.cc
  source/falaise/snemo/digitization/fake_trigger_module.cc
  source/falaise/snemo/digitization/fake_trigger_algo.cc
  source/falaise/snemo/digitization/digitization_module.cc
  source/falaise/snemo/digitization/digitization_driver.cc
  source/falaise/snemo/digitization/tracker_trigger_algorithm.cc
  source/falaise/snemo/digitization/tracker_trigger_algorithm_test_time.cc
  source/falaise/snemo/digitization/calo_trigger_algorithm.cc
  source/falaise/snemo/digitization/calo_trigger_algorithm_test_time.cc
  source/falaise/snemo/digitization/coincidence_trigger_algorithm_test_time.cc
  source/falaise/snemo/digitization/coincidence_trigger_algorithm.cc
  source/falaise/snemo/digitization/signal_to_calo_tp_algo.cc
  source/falaise/snemo/digitization/signal_to_geiger_tp_algo.cc
  source/falaise/snemo/digitization/sd_to_geiger_signal_algo.cc
  source/falaise/snemo/digitization/sd_to_calo_signal_algo.cc

  source/falaise/snemo/digitization/signal_data.cc

  source/falaise/snemo/digitization/calo_tp_to_ctw_algo.cc
  source/falaise/snemo/digitization/calo_ctw_data.cc
  source/falaise/snemo/digitization/calo_ctw.cc
  source/falaise/snemo/digitization/calo_tp_data.cc
  source/falaise/snemo/digitization/calo_tp.cc
  source/falaise/snemo/digitization/calo_feb_data.cc
  source/falaise/snemo/digitization/calo_signal.cc
  source/falaise/snemo/digitization/calo_signal_simple_shape.cc

  source/falaise/snemo/digitization/geiger_tp_to_ctw_algo.cc
  source/falaise/snemo/digitization/geiger_ctw_data.cc
  source/falaise/snemo/digitization/geiger_ctw.cc
  source/falaise/snemo/digitization/geiger_tp_data.cc
  source/falaise/snemo/digitization/geiger_tp.cc
  source/falaise/snemo/digitization/tracker_feb_data.cc
  source/falaise/snemo/digitization/geiger_signal.cc

  source/falaise/snemo/digitization/tracker_sliding_zone.cc
  source/falaise/snemo/digitization/tracker_zone.cc
  source/falaise/snemo/digitization/trigger_display_manager.cc
  source/falaise/snemo/digitization/trigger_info.cc
  source/falaise/snemo/digitization/trigger_structures.cc
  source/falaise/snemo/digitization/electronic_mapping.cc
  source/falaise/snemo/digitization/clock_utils.cc
  source/falaise/snemo/digitization/ID_convertor.cc
  source/falaise/snemo/digitization/mapping.cc

  source/falaise/snemo/digitization/the_serializable.cc

  )

############################################################################################

# Build a dynamic library from our sources
add_library(Falaise_Digitization SHARED
  ${FalaiseDigitizationPlugin_HEADERS}
  ${FalaiseDigitizationPlugin_SOURCES})

target_link_libraries(Falaise_Digitization
  Falaise)

# Apple linker requires dynamic lookup of symbols, so we
# add link flags on this platform
if(APPLE)
  set_target_properties(Falaise_Digitization
    PROPERTIES LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()

# Install it:
install(TARGETS Falaise_Digitization DESTINATION ${CMAKE_INSTALL_LIBDIR}/Falaise/modules)

# Test support:
option(FalaiseDigitizationPlugin_ENABLE_TESTING "Build unit testing system for FalaiseDigitization" ON)
if(FalaiseDigitizationPlugin_ENABLE_TESTING)
  enable_testing()
  add_subdirectory(testing)
endif()

add_subdirectory(programs)
add_subdirectory(validation)
add_subdirectory(half_commissioning_analysis)
