cmake_minimum_required(VERSION 3.8)

project(TINCExamples)

include(../cmake/BuildExamples.cmake)

BuildExamples("${CMAKE_CURRENT_SOURCE_DIR}" "tinc_examples" tinc)
