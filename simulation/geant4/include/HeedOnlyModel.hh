/*
 * HeedOnlyModel.h
 *
 *  Created on: Apr 9, 2014
 *      Author: dpfeiffe
 */

#ifndef HEEDONLYMODEL_H_
#define HEEDONLYMODEL_H_

#include "G4ThreeVector.hh"

#include "AvalancheMicroscopic.hh"
#include "AvalancheMC.hh"
#include "DriftLineRKF.hh"

//Visualization
#include "TCanvas.h"
#include "ViewCell.hh"
#include "ViewDrift.hh"
#include "ViewSignal.hh"
#include "ViewField.hh"

#include "G4VFastSimulationModel.hh"

#include "HeedModel.hh"


class G4VPhysicalVolume;
class DetectorConstruction;
class HeedOnlyMessenger;
class GasModelParameters;
class TPCSD;

typedef std::pair<double, double> EnergyRange_keV;
typedef std::multimap<const G4String, EnergyRange_keV> MapParticlesEnergy;

class HeedOnlyModel : public HeedModel {
public:
  //-------------------------
  // Constructor, destructor
  //-------------------------
  HeedOnlyModel(GasModelParameters*,G4String, G4Region*,DetectorConstruction*, TPCSD*);
  ~HeedOnlyModel();

  // //This method is called after each event, to record the relevant data
  // virtual void ProcessEvent();
  // //This method is called at the beginning of an event to reset some variables of the class
  // virtual void Reset();

  virtual bool Readout();

private:
  virtual void Run(G4String particleName, double ekin_keV, double t, double x_cm,
		   double y_cm, double z_cm, double dx, double dy, double dz);
};

#endif /* HeedOnlyModel_H_ */

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
