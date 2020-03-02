#plugin build file
cmake_minimum_required(VERSION 3.5.0)

#include common rules
include(../PluginRules.cmake)

#add sources, not including OpenEphysLib.cpp
add_sources(${PLUGIN_NAME}
	ClusterDecoder/ClusterDecoderProcessor.cpp
	ClusterDecoder/ClusterDecoderProcessor.h
	ClusterDecoder/ClusterDecoderProcessorContentComponent.cpp
	ClusterDecoder/ClusterDecoderProcessorContentComponent.h
	ClusterDecoder/ClusterDecoderProcessorEditor.cpp
	ClusterDecoder/ClusterDecoderProcessorEditor.h
	ClusterDecoder/ClusterDecoderProcessorVisualizer.cpp
	ClusterDecoder/ClusterDecoderProcessorVisualizer.h
	ClusterDecoder/ClusterDecoderProcessorVisualizerContentComponent.cpp
	ClusterDecoder/ClusterDecoderProcessorVisualizerContentComponent.h
	ClusterDecoder/ClusterDecoderNetwork.cpp
	ClusterDecoder/ClusterDecoderNetwork.h
	json.h
	jsoncpp.cpp
	SleepScorer/SleepScorerProcessor.cpp
	SleepScorer/SleepScorerProcessor.h
	SleepScorer/SleepScorerProcessorContentComponent.cpp
	SleepScorer/SleepScorerProcessorContentComponent.h
	SleepScorer/SleepScorerProcessorEditor.cpp
	SleepScorer/SleepScorerProcessorEditor.h
	SleepScorer/SleepScorerProcessorVisualizer.cpp
	SleepScorer/SleepScorerProcessorVisualizer.h
	SleepScorer/SleepScorerProcessorVisualizerContentComponent.cpp
	SleepScorer/SleepScorerProcessorVisualizerContentComponent.h
	SleepScorer/SleepScorerFilter.cpp
	SleepScorer/SleepScorerFilter.h
	SpikeSorterWithStim/SpikeSorterWithStim.cpp
	SpikeSorterWithStim/SpikeSorterWithStim.h
	SpikeSorterWithStim/SpikeSortBoxesWithStim.cpp
	SpikeSorterWithStim/SpikeSortBoxesWithStim.h
	SpikeSorterWithStim/SpikeSorterEditorWithStim.cpp
	SpikeSorterWithStim/SpikeSorterEditorWithStim.h
	SpikeSorterWithStim/SpikeSorterCanvasWithStim.cpp
	SpikeSorterWithStim/SpikeSorterCanvasWithStim.h
	PositionDecoder/PositionDecoderProcessor.cpp
	PositionDecoder/PositionDecoderProcessor.h
	PositionDecoder/PositionDecoderProcessorContentComponent.cpp
	PositionDecoder/PositionDecoderProcessorContentComponent.h
	PositionDecoder/PositionDecoderProcessorEditor.cpp
	PositionDecoder/PositionDecoderProcessorEditor.h
	PositionDecoder/PositionDecoderProcessorVisualizer.cpp
	PositionDecoder/PositionDecoderProcessorVisualizer.h
	PositionDecoder/PositionDecoderProcessorVisualizerContentComponent.cpp
	PositionDecoder/PositionDecoderProcessorVisualizerContentComponent.h
	PositionDecoder/PositionDecoderNetwork.cpp
	PositionDecoder/PositionDecoderNetwork.h
	)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/../.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/fir1/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

add_subdirectory(fir1)
add_subdirectory(fftw-3.3.8)
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/fir1")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/fftw-3.3.8/api")
LINK_DIRECTORIES(/usr/local/lib)
TARGET_LINK_LIBRARIES(OnlineDecoding fir fftw3 tensorflow_cc tensorflow_framework)

#optional: create IDE groups
#plugin_create_filters()
