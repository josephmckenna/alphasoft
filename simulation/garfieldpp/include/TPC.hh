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
    //   init();
  };

  std::vector<std::string> GetAnodeReadouts(){ return anodes;};
  std::vector<std::string> GetPadReadouts(){ return pads;};
  std::vector<std::string> GetOtherReadouts(){ return readouts;};

  void SetGas(Medium *m);
  void SetVoltage(double &vc, double& vaw, double& vfw);

  void init();

  double CathodeVoltage, AnodeVoltage, FieldVoltage;
protected:

  vector<string> anodes, pads, readouts;
  GeometrySimple geo;
  Medium *medium;
  SolidTube* chamber;
};

#endif
