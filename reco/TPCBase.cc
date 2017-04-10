#include "TPCBase.hh"
#include <cmath>
#include <map>
#include <sstream>
#include <fstream>
#include <cassert>

#include "Alpha16.h"

using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::string;

int TPCBase::MapElectrodes(short run, std::vector<electrode> &anodes, std::vector<electrode> &pads, double &phi0){
    anodes.clear();
    pads.clear();

    if(run < 0){
        for(unsigned int i = 0; i < 2*TPCBase::NanodeWires; i++){
            int ii = i % TPCBase::NanodeWires;
            short us_ds = (i / TPCBase::NanodeWires);
            anodes.emplace_back(us_ds, ii);
        }
        for(unsigned int s = 0; s < TPCBase::npadsec; s++)
            for(unsigned int i = 0; i < TPCBase::npads; i++){
                pads.emplace_back(s, i);
            }
        phi0 = 0;
    } else {
        map<short, double> gainMap;
        // std::ifstream gainFile("preamp_a16_gain.dat");
        // assert(gainFile.is_open());
        // if(gainFile.is_open()){
        //     while(gainFile.good()){
        //         if(gainFile.peek() == '#'){
        //             char buf[1024];
        //             gainFile.getline(buf,1023);
        //             continue;
        //         }
        //         short chan;
        //         double gain, err;
        //         // gainFile >> chan >> gain >> err;
        //         gainFile >> chan >> gain;
        //         if(gainFile.good()){
        //             if(gainMap.find(chan) != gainMap.end()){
        //                 std::cerr << "Duplicate entries for channel " << chan << " in preamp_a16_gain.dat!" << endl;
        //                 return -1;
        //             }
        //             gainMap[chan] = gain;
        //             gainFile.peek();
        //         }
        //     }
        // }
        // gainFile.close();
        map<short, double> againMap;
        std::ifstream againFile("anode_gain.dat");
        assert(againFile.is_open());
        if(againFile.is_open()){
            while(againFile.good()){
                if(againFile.peek() == '#'){
                    char buf[1024];
                    againFile.getline(buf,1023);
                    continue;
                }
                short an;
                double gain, err;
                againFile >> an >> gain;
                if(againFile.good()){
                    if(againMap.find(an) != againMap.end()){
                        std::cerr << "Duplicate entries for channel " << an << " in anode_gain.dat!" << endl;
                        return -1;
                    }
                    againMap[an] = gain;
                    againFile.peek();
                }
            }
        }
        againFile.close();
               // gainMap.clear();
        map<short, vector<short> > moduleMap;
        std::ifstream mMapFile("alpha16.map");
        if(mMapFile.is_open()){
            while(mMapFile.good()){
                if(mMapFile.peek() == '#'){
                    char buf[1024];
                    mMapFile.getline(buf,1023);
                    continue;
                }
                short runN;
                mMapFile >> runN;
                if(mMapFile.good()){
                    if(moduleMap.find(runN) != moduleMap.end()){
                        std::cerr << "Duplicate entries for run " << runN << " in alpha16.map!" << endl;
                        return -1;
                    }
                    string line;
                    getline(mMapFile,line);
                    std::istringstream iss(line);
                    short sec;
                    iss >> phi0;
                    phi0 *= M_PI/180.;
                    while(iss >>  sec) moduleMap[runN].push_back(sec);
                }
            }
        } else {
	  printf("Signals::MapElectrodes: cannot read from alpha16.map!\n");
	  return -1;
	}

        vector<short> vec;
        for(auto it = moduleMap.rbegin(); it != moduleMap.rend(); it++){
            if(run >= it->first){
                cout << "run " << run << " >= " << it->first << endl;
                vec = it->second;
                break;
            }
        }
        for(unsigned short mod = 0; mod < MAX_ALPHA16 && mod < vec.size(); mod++){
            short sec = vec[mod];
            short tb = sec<0;
            sec %= 16;
            cout << "Module " << mod << " -> AWC " << sec << '\t' << (tb?"bottom":"top") << endl;
            for(int i = 0; i < NUM_CHAN_ALPHA16; i++){
                short chan = mod*NUM_CHAN_ALPHA16+i;
                double anode = abs(sec)*16+i;
                double gain = 1.;
                auto it = gainMap.find(chan);
                if(it != gainMap.end())
                    gain = it->second;
                else
                    cout << "No gain listed for channel " << chan << ", assuming 1.0" << endl;
                if(tb){
                    if(anode==0){
                        it = againMap.find(-256);
                    } else {
                        it = againMap.find(-anode);
                    }
                } else {
                    it = againMap.find(anode);
                }
                if(it != againMap.end())
                    gain *= it->second;
                else
                    cout << "No gain listed for anode " << anode << ", assuming 1.0" << endl;
                anodes.emplace_back(tb, anode, gain);
            }
        }
    }
    cout << "Electrode map set: " << anodes.size()+pads.size() << endl;
    // for(unsigned int i = 0; i < anodes.size(); i++) cout << i << '\t' << (anodes[i].sec?'b':'t') << '\t' << anodes[i].i << endl;
    return anodes.size()+pads.size();
}

void TPCBase::GetAnodePosition(int i, double &x, double &y, bool polar, double phi0){
    double AngleAnodeWires = GetAnodePitch();
    double AngleOffsetAnodeWires = 0.5*AngleAnodeWires;
    double phi = AngleAnodeWires * i + AngleOffsetAnodeWires + phi0;
    if(polar){
        x = AnodeWiresR;
        y = phi;
    } else {
        x = AnodeWiresR*cos(phi);
        y = AnodeWiresR*sin(phi);
    }
}

void TPCBase::GetWirePosition(int i, double &x, double &y, bool polar, double phi0){
    double AngleFieldWires = 2.*M_PI / double(NfieldWires);
    double phi = AngleFieldWires * i + phi0;
    if(polar){
        x = FieldWiresR;
        y = phi;
    } else {
        x = FieldWiresR*cos(phi);
        y = FieldWiresR*sin(phi);
    }
}

void TPCBase::GetPadPosition(int i, double &z){  // Currently no phi-segmentation
    z = (((double) npads) * 0.5 - (i + 0.5))* PadSideZ;
}

void TPCBase::GetPadPosition(int i, double &z, double &phi)
{
  double ring_d = double(i)/double(npadsec);
  double intpart;
  double frac = modf(ring_d,&intpart);
  //  assert(intpart<gNpadsZ);
  z = ( intpart + 0.5 ) * PadSideZ;
  z = HalfWidthZ - z;
  phi = frac * 2.*M_PI;
}

void TPCBase::GetPadPosition(int i, int s, double &z, double &phi)
{
  double phi_c = 2.*M_PI*PadWidthPhi;
  double phi_ch = M_PI*PadWidthPhi;
  if( s >= 0 && s < npadsec )
    //    phi = double(s) * phi_c;
    phi = double(s) * phi_c + phi_ch;
  else
    {
      phi=-9999999.;
      std::cerr<<"TPCBase::GetPadPosition Error: Sector Out Of Range: "<<s<<"[0,"<<npadsec<<")\n";
      return;
    }

  if( i >= 0 && i < npads)
    {
      z = ( double(i) + 0.5 ) * PadSideZ;
      z = HalfWidthZ - z;
    }
  else
    {
      z=-9999999.;
      std::cerr<<"TPCBase::GetPadPosition Error: Pads Out Of Range: "<<i<<"[0,"<<npads<<"31)\n";
      return;
    }
}

std::pair<int,int> TPCBase::FindPad(const double zed, const double phi)
{  // // Currently no phi-segmentation
   //  return (0.5*(npads - 1) - z/PadSideZ);
  double z = HalfWidthZ - zed;
  double f = phi;
  if(f<0.) f+=2.*M_PI;
  //  std::cout<<"z: "<<zed<<"\tphi: "<<phi<<std::endl;

  std::pair<int,int> pad;
  pad.first = int(f/2.*M_PI*npadsec);  // pad sector
  pad.second = int(z/(2*HalfWidthZ)*npads);         // pad index in sector
  if(pad.first >= npadsec) pad.first = -1;
  if(pad.second >= npads) pad.second = -1;
  return pad;
}

int TPCBase::SectorAndPad2Index(const int ps, const int pi){
    return (ps+pi*npadsec);
}

int TPCBase::SectorAndPad2Index(std::pair<int,int> p){
  int ps = p.first, pi = p.second;
  return (ps+pi*npadsec);
}

std::pair<int,int> TPCBase::Index2SectorAndPad(const int padindex){
    int pi = padindex/npadsec;
    int ps = padindex%npadsec;
    return std::pair<int,int>(ps,pi);
}

unsigned int TPCBase::FindAnode(const double phi, double phi0){
    double AngleAnodeWires = GetAnodePitch();
    int anode((phi-phi0)/AngleAnodeWires);
    if(anode < 0) anode += NanodeWires;
    return anode;
}
