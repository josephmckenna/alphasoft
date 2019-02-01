/*
 * HeedModel.h
 *
 *  Created on: Apr 9, 2014
 *      Author: dpfeiffe
 */

#ifndef HEEDMODEL_H_
#define HEEDMODEL_H_

#include "GasModelParameters.hh"

#include "ComponentAnalyticField.hh"  //Garfield field

#include "TrackHeed.hh"

#include "DriftLineRKF.hh"
#include "AvalancheMicroscopic.hh"
#include "AvalancheMC.hh"

#include "TCanvas.h"
#include "ViewCell.hh"      //Visualization
#include "ViewDrift.hh"
#include "ViewSignal.hh"
#include "ViewField.hh"

#include "G4VFastSimulationModel.hh"
#include "G4ThreeVector.hh"
#include "TPCSD.hh"


class G4VPhysicalVolume;
class DetectorConstruction;
class HeedMessenger;

class HeedModel : public G4VFastSimulationModel 
{
public:
  //-------------------------
  // Constructor, destructor
  //-------------------------
  HeedModel(G4String, G4Region*, DetectorConstruction*, TPCSD*);
  ~HeedModel();

  virtual G4bool IsApplicable(const G4ParticleDefinition&);
  virtual G4bool ModelTrigger(const G4FastTrack&);
  virtual void DoIt(const G4FastTrack&, G4FastStep&);
  
  /*The following public methods are user-dependent*/

  //This method is called after each event, to record the relevant data
  virtual void ProcessEvent();
  //This method is called at the beginning of an event to reset some variables of the class
  virtual void Reset();
  G4bool FindParticleName(G4String name);
  G4bool FindParticleNameEnergy(G4String name,double ekin_keV);
  
  void SetBinWidth(double w)  { fBinWidth = w; }
  void SetNumberOfBins(int n) { fNbins = n; }
  double GetBinWith() const   { return fBinWidth; } 
  int GetNumberOfBins() const { return fNbins; }
  
protected:
  void InitialisePhysics();
  virtual void Run(G4String particleName, double ekin_keV, double t, 
		   double x_cm, double y_cm, double z_cm,
		   double dx, double dy, double dz) = 0;
  void PlotTrack(G4String fileName="PrimaryTrack.pdf");
  void PlotSignal(G4String electrode, G4String fileName);
  void Drift(double,double, double, double);
  void AddTrajectories();

  DetectorConstruction* fDet;
  TPCSD* fTPCSD;

  MapParticlesEnergy fMapParticlesEnergy;
  
  bool driftElectrons;
  bool createAval;

  bool driftRKF;
  bool trackMicro;

  bool generateSignals;

  bool fVisualizeChamber;
  bool fVisualizeSignal;
  bool fVisualizeField;

  Garfield::TrackHeed* fTrackHeed;
  Garfield::Sensor* fSensor;

  // model's name
  const char* fName;

  // number of ions to track to produce a signal
  int fNions;
  // the higher this number is the more accurate is the signal generation
  // the lower this number is the faster is the simulation

  // drift region boundaries [potentially unused]
  double fMaxRad, fMinRad, fLen; // in cm for Garfiled++ will

  // signal readout
  double fBinWidth; // ns
  int fNbins;  

  /*The following private methods and variables are user-dependent*/
private:
  void AddSensor();
  void TestSensor();
  void SetTracking();
  void CreateChamberView();
  void CreateSignalView();
  void CreateFieldView();

  Garfield::AvalancheMC* fDrift;
  Garfield::DriftLineRKF* fDriftRKF;
  Garfield::AvalancheMicroscopic* fAvalanche;

  Garfield::AvalancheMC* fIonDrift;

  TCanvas* fChamber;
  TCanvas* fSignal;
  TCanvas* fField;
  Garfield::ViewCell* cellView;
  Garfield::ViewDrift* viewDrift;
  Garfield::ViewSignal* viewSignal;
  Garfield::ViewField* viewField;
  
};

#endif /* HeedModel_H_ */
