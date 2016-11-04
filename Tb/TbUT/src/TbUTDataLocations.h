/*
 * TbUTDataLocations.h
 *
 *  Created on: Oct 11, 2014
 *      Author: ADendek
 */

#pragma once
#pragma GCC diagnostic ignored "-Wunused-variable"

#include <string>

namespace TbUT
{
namespace DataLocations
{
//should be extended
static const std::string& RawTES      	 		= "Tb/TbUT/Raw";
static const std::string& HeaderTES                     = "Tb/TbUT/Header";
static const std::string& PedestalTES 	 		= "Tb/TbUT/Pedestal";
static const std::string& CMSTES      	 		= "Tb/TbUT/CMS";
static const std::string& ZS_TES      	 		= "Tb/TbUT/ZS";
static const std::string& Clusters_TES	 		= "Tb/TbUT/Clusters";
static const std::string& KeplerCluster_TES	= "Tb/TbUT/KeplerClusters";

static const std::string & MaskLocation      = "$KEPLERROOT/options/UT/ChannelMask_NO.dat";
static const std::string & PedestalLocation  = "$KEPLERROOT/options/UT/Pedestal.dat";
static const std::string & NoiseTreshold     = "$KEPLERROOT/options/UT/Noise.dat";
}
}
