/*
 * HeedInterfaceModel.h
 *
 *  Created on: Apr 9, 2014
 *      Author: dpfeiffe
 */

#ifndef HEEDINTERFACEMODEL_H_
#define HEEDINTERFACEMODEL_H_

#include "HeedModel.hh"


class G4VPhysicalVolume;
class DetectorConstruction;
class HeedInterfaceMessenger;
class GasModelParameters;
class TPCSD;

typedef std::pair<double, double> EnergyRange_keV;
typedef std::multimap<const G4String, EnergyRange_keV> MapParticlesEnergy;

class HeedInterfaceModel : public HeedModel {
 public:
  //-------------------------
  // Constructor, destructor
  //-------------------------
  HeedInterfaceModel(GasModelParameters*, G4String, 
		     G4Region* ,DetectorConstruction*, TPCSD*);
  ~HeedInterfaceModel();
 
  // //This method is called after each event, to record the relevant data
  // virtual void ProcessEvent();
  // //This method is called at the beginning of an event to reset some variables of the class
  // virtual void Reset();

  virtual bool Readout();
 
 private:
  virtual void Run(G4String particleName, double ekin_keV, double t, double x_cm,
            double y_cm, double z_cm, double dx, double dy, double dz);
};

#endif /* HeedInterfaceModel_H_ */
