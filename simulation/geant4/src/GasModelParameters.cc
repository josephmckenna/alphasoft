#include "GasModelParameters.hh"

#include "HeedInterfaceModel.hh"
#include "HeedOnlyModel.hh"
#include "GasModelParametersMessenger.hh"

GasModelParameters::GasModelParameters(): driftElectrons(true),createAval(true),
					  driftRKF(false),trackMicro(true),
					  generateSignals(true),
					  awnoise(1.),pdnoise(1.),
					  fVisualizeChamber(false),
					  fVisualizeSignal(false),
					  fVisualizeField(false),
					  vAnode(3.1e3),
					  vCathode(-4.e3),
					  vField(-99.)
{ 
  gasFile = G4String(getenv("AGRELEASE"))+"/simulation/common/gas_files/ar_70_co2_30_725Torr_20E200000_4B1.10.gas";
  ionMobFile = "IonMobility_Ar+_Ar.txt";
  fMessenger = new GasModelParametersMessenger(this);
}


void GasModelParameters::AddParticleNameHeedOnly(const G4String particleName,double ekin_min_keV,double ekin_max_keV){
    if (ekin_min_keV >= ekin_max_keV) {
        return;
    }
    fMapParticlesEnergyHeedOnly.insert(
                                std::make_pair(particleName, std::make_pair(ekin_min_keV, ekin_max_keV)));
    G4cout << "HeedOnly: " << particleName << " added: " << ekin_min_keV << " " << ekin_max_keV << G4endl;
}

void GasModelParameters::AddParticleNameHeedInterface(const G4String particleName,double ekin_min_keV,double ekin_max_keV){
    if (ekin_min_keV >= ekin_max_keV) {
        return;
    }
    fMapParticlesEnergyHeedInterface.insert(
                                std::make_pair(particleName, std::make_pair(ekin_min_keV, ekin_max_keV)));
    G4cout << "HeedInterface: " << particleName << " added: " << ekin_min_keV << " " << ekin_max_keV << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
