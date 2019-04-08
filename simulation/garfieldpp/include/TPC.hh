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

class TPC: public Garfield::ComponentBmap, public TPCBase
{
public:
  TPC(double V_c=-5325., double V_a=2050., double V_f=-227.);

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

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
