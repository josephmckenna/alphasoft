#ifndef TPC_H
#define TPC_H

#include <iostream>
#include <vector>
#include <string>

#include <cmath>

#include <GeometrySimple.hh>
#include <Medium.hh>
#include <SolidTube.hh>
#include "ComponentBmap.hh"

#include "TPCBase.hh"

using namespace std;
using namespace Garfield;

class TPC: public Garfield::ComponentBmap, public TPCBase{
public:

  TPC(float V_c=-5325., float V_a=2050., float V_f=-227.): Garfield::ComponentBmap(true), 
							   TPCBase(true),
							   CathodeVoltage(V_c), 
							   AnodeVoltage(V_a), 
							   FieldVoltage(V_f), 
							   medium(0), chamber(0)
    {
        init();
    };

    vector<string> GetAnodeReadouts(){ return anodes;};
    vector<string> GetPadReadouts(){ return pads;};
    vector<string> GetOtherReadouts(){ return readouts;};

    void SetGas(Medium *m);

    double CathodeVoltage, AnodeVoltage, FieldVoltage;
protected:
    void init();
    vector<string> anodes, pads, readouts;
    GeometrySimple geo;
    Medium *medium;
    SolidTube* chamber;
};

#endif
