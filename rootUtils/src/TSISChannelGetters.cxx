
#include "TSISChannelGetters.h"

#ifdef BUILD_A2

TSISChannel GetSISChannel(int runNumber, const char* ChannelName)
{
   TSISChannel chan=-1;
   TSISChannels sisch(runNumber);
   chan=sisch.GetChannel(ChannelName);
   return chan;
}
std::vector<TSISChannel> GetSISChannels(int runNumber, const std::vector<std::string>& ChannelNames)
{
    std::vector<TSISChannel> channels;
    TSISChannels sisch(runNumber);
    for (auto& name: ChannelNames)
    {
        channels.push_back(sisch.GetChannel(name.c_str()));
    }
    return channels;
}

#endif