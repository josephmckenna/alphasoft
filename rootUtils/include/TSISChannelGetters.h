
#ifndef _TSISCHANNELGETTERS_
#define _TSISCHANNELGETTERS_

#ifdef BUILD_A2
#include <vector>
#include "TSISChannel.h"
#include "TSISChannels.h"


TSISChannel GetSISChannel(int runNumber, const char* ChannelName);
std::vector<TSISChannel> GetSISChannels(int runNumber, const std::vector<std::string>& ChannelNames);

#endif


#endif