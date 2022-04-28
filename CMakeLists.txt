cmake_minimum_required(VERSION 3.12)
project(wib_hdf5_compress VERSION 0.0.1)

find_package(daq-cmake REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package(detdataformats REQUIRED)
find_package(CLI11 REQUIRED)
find_package(HighFive REQUIRED)

daq_setup_environment()


##############################################################################
# Main library

daq_add_library(LINK_LIBRARIES detdataformats::detdataformats HighFive)


##############################################################################
# Plugins

##############################################################################
# Integration tests

##############################################################################
# Unit Tests

##############################################################################
# Applications
daq_add_application(write_wib_hdf5 write_wib_hdf5.cxx TEST LINK_LIBRARIES wib_hdf5_compress CLI11::CLI11)
daq_add_application(write_wib_fragment_style write_wib_fragment_style.cxx TEST LINK_LIBRARIES wib_hdf5_compress CLI11::CLI11)

daq_install()
