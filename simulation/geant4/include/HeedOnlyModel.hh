/*
 * HeedOnlyModel.h
 *
 *  Created on: Apr 9, 2014
 *      Author: dpfeiffe
 *
 *  Modified: Jan, 2019
 *            A Capra
 *
 */

#ifndef HEEDONLYMODEL_H_
#define HEEDONLYMODEL_H_

#include "G4ThreeVector.hh"

#include "Garfield/AvalancheMicroscopic.hh"
#include "Garfield/AvalancheMC.hh"
#include "Garfield/DriftLineRKF.hh"

//Visualization
#include "TCanvas.h"
#include "Garfield/ViewCell.hh"
#include "Garfield/ViewDrift.hh"
#include "Garfield/ViewSignal.hh"
#include "Garfield/ViewField.hh"

#include "G4VFastSimulationModel.hh"

#include "HeedModel.hh"


class G4VPhysicalVolume;
class DetectorConstruction;
class HeedOnlyMessenger;
class GasModelParameters;
class TPCSD;

class G4FastStep;
class G4FastTrack;

typedef std::pair<double, double> EnergyRange_keV;
typedef std::multimap<const G4String, EnergyRange_keV> MapParticlesEnergy;

class HeedOnlyModel : public HeedModel {
public:
  //-------------------------
  // Constructor, destructor
  //-------------------------
  HeedOnlyModel(GasModelParameters*,G4String, G4Region*,DetectorConstruction*, TPCSD*);
  ~HeedOnlyModel();

  //This method is called after each event, to record the relevant data
  virtual void ProcessEvent();
  //This method is called at the beginning of an event to reset some variables of the class
  virtual void Reset();
  
  /*Getters and Setters*/

  virtual bool Readout();

private:
   virtual void Run(G4FastStep& fastStep, const G4FastTrack& fastTrack, 
                    G4String particleName, double ekin_keV, double t, 
                    double x_cm, double y_cm, double z_cm, double dx, double dy, double dz);
};

#endif /* HeedOnlyModel_H_ */

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
