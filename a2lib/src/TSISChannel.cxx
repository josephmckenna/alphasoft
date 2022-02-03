#include "TSISChannel.h"




TSISChannel::TSISChannel()
{
   fChannel = -1;
   fModule = -1;
}

TSISChannel::TSISChannel(const int chan, const int mod)
{
   fChannel = chan;
   fModule = mod;
}

TSISChannel::TSISChannel(const int CombinedChannelAndModule)
{
   fModule = 0;
   fChannel = CombinedChannelAndModule;
   while (  fChannel >= NUM_SIS_CHANNELS)
   {
     fModule++;
     fChannel -= NUM_SIS_CHANNELS;
   }
   
}

TSISChannel::TSISChannel(const TSISChannel& c): fChannel(c.fChannel), fModule(c.fModule)
{
      
}

TSISChannel& TSISChannel::operator=(const TSISChannel& rhs)
{
    fChannel = rhs.fChannel;
    fModule = rhs.fModule;
    return *this;
}

bool operator==(const TSISChannel& lhs, const TSISChannel& rhs)
{
   return ( lhs.fChannel == rhs.fChannel && lhs.fModule == rhs.fModule);
}

std::ostream& operator<< (std::ostream& os, const TSISChannel& ch)
{
    os <<"[ " << ch.fChannel << ": " << ch.fModule << " ]";
    return os;
}

TString& operator+=(TString& s, const TSISChannel& lhs)
{
    s += "[ ";
    s += lhs.fChannel;
    s += ", ";
    s += lhs.fModule;
    s += "]";
    return s;
}