/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2013 Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <PluginInfo.h>
#include "SleepScorer/SleepScorerProcessor.h"
#include "ClusterDecoder/ClusterDecoderProcessor.h"
#include "PositionDecoder/PositionDecoderProcessor.h"
#include "SpikeSorterWithStim/SpikeSorterWithStim.h"
#include <string>
#ifdef WIN32
#include <Windows.h>
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

using namespace Plugin;
#define NUM_PLUGINS 4

extern "C" EXPORT void getLibInfo(Plugin::LibraryInfo* info)
{
	info->apiVersion = PLUGIN_API_VER;
	info->name = "Online Decoding";
	info->libVersion = 1;
	info->numPlugins = NUM_PLUGINS;
}

extern "C" EXPORT int getPluginInfo(int index, Plugin::PluginInfo* info)
{
	switch (index)
	{
	case 0:
		info->type = Plugin::PLUGIN_TYPE_PROCESSOR;
		info->processor.name = "Sleep Scorer";
		info->processor.type = Plugin::FilterProcessor;
		info->processor.creator = &(Plugin::createProcessor<SleepScorerProcessor>);
		break;
	case 1:
		info->type = Plugin::PLUGIN_TYPE_PROCESSOR;
		info->processor.name = "Cluster Decoder";
		info->processor.type = Plugin::FilterProcessor;
		info->processor.creator = &(Plugin::createProcessor<ClusterDecoderProcessor>);
		break;
	case 2:
		info->type = Plugin::PLUGIN_TYPE_PROCESSOR;
		info->processor.name = "Position Decoder";
		info->processor.type = Plugin::FilterProcessor;
		info->processor.creator = &(Plugin::createProcessor<PositionDecoderProcessor>);
		break;
	case 3:
		info->type = Plugin::PLUGIN_TYPE_PROCESSOR;
		info->processor.name = "Spk Srt + Stim";
		info->processor.type = Plugin::FilterProcessor;
		info->processor.creator = &(Plugin::createProcessor<SpikeSorterWithStim>);
		break;
	default:
		return -1;
		break;
	}
	return 0;
}

#ifdef WIN32
BOOL WINAPI DllMain(IN HINSTANCE hDllHandle,
	IN DWORD     nReason,
	IN LPVOID    Reserved)
{
	return TRUE;
}

#endif
