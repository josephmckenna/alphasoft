#include <iostream>
#include <cstdlib>

#include <TMath.h>
#include <TString.h>

#include "Garfield/MediumMagboltz.hh"


int makeGas(double CO2frac = 10, int nEF = 20, double EFmax = 200000, int nBF = 6, double BFmax = 1.25, int nTh = 5, double thetaMax = 180)
{
    // Make a medium
    Garfield::MediumMagboltz* gas = new Garfield::MediumMagboltz();
    gas->SetComposition("ar", 100.-CO2frac, "co2", CO2frac);
    gas->SetTemperature(293.15); // K
    //gas->SetTemperature(303.15); // K
    //    gas->SetPressure(760.); // Torr @ TRIUMF
    gas->SetPressure(725.); // Torr @ CERN, it may be 735 Torr = 980 hPa, altitude 432 m

    // Set electric field (EF) range covered by gas table.
    double EFmin =    100.; // V/cm
    // Flag to request logarithmic spacing.
     bool useLog = true;
    if(nEF==1){
        useLog = false;
        EFmin = EFmax;
    }
    // Set magnetic field (BF) range covered by gas table.
    double BFmin = 0.; // T
    if(BFmax == 0 && nBF > 1){
        std::cout << "Asking for multiple magnetic fields between 0 and 0 doesn't make sense. Reducing to one field." << std::endl;
        nBF = 1;
    }
    if(nBF == 1){
        BFmin = BFmax;
        if(BFmax == 0){
            thetaMax = 90;
            nTh = 1;
        }
    }
    // Set angle between BF and EF
    double thetaMin = 0;


    thetaMax *= Garfield::Pi/180.;
    if(nTh == 1) thetaMin = thetaMax;
    gas->SetFieldGrid(EFmin, EFmax, nEF, useLog,
                      BFmin, BFmax, nBF,
                      thetaMin, thetaMax, nTh);

    std::cout << nEF << " E fields from   " << EFmin << " to " << EFmax << std::endl;
    std::cout << nBF << " B fields from   " << BFmin << " to " << BFmax << std::endl;
    std::cout << nTh << " E-B angles from " << thetaMin << " to " << thetaMax << std::endl << std::endl;

    std::cout << "gas->SetFieldGrid(" << EFmin << ", " << EFmax << ", " <<  nEF << ", " << useLog << ", " <<
        BFmin << ", " <<  BFmax << ", " <<  nBF << ", " <<  thetaMin << ", " << thetaMax << ", " << nTh << ")" << std::endl;
    //  gas->SetFieldGrid(EFmin, EFmax, nEF, useLog);


    gas->SetMaxElectronEnergy(1.e3);

    // To see the full Magboltz output
    gas->EnableDebugging();
    // Specify number of collisions (in multiples of 10^7)
    // over which the electron is traced by Magboltz
    const int ncoll = 10;
    const bool verbose = true;
    gas->GenerateGasTable(ncoll, verbose);
    //  gas->GenerateGasTable(10);

    //gas->DisableDebugging();
    // Save the table.
    TString filename = TString::Format("ar_%.0f_co2_%.0f_NTP_%dE%.0f_%dB%.2f.gas",100-CO2frac,CO2frac,nEF,EFmax,nBF,BFmax);
    //    TString filename = TString::Format("ar_64_co2_26_cf4_10_NTP_%dE%.0f_%dB%.0f.gas",nEF,EFmax,nBF,BFmax);
    //    TString filename = TString::Format("ar_xx_co2_xx_air_0_NTP_%dE%.0f_%dB%.0f.gas",nEF,EFmax,nBF,BFmax);
    gas->WriteGasFile(filename.Data());

    return 0;
}

int makeContaminatedGas(double N2frac = 1.0, int nEF = 25, double EFmax = 1000000,
                        int nBF = 1, double BFmax = 0., int nTh = 0, double thetaMax = 0.)
{
   double CO2frac=30.;
    // Make a medium
    Garfield::MediumMagboltz* gas = new Garfield::MediumMagboltz();
    gas->SetComposition("ar", 100.-CO2frac, "co2", CO2frac-N2frac, "n2", N2frac);
    gas->SetTemperature(293.15); // K
    //    gas->SetPressure(760.); // Torr @ TRIUMF
    gas->SetPressure(725.); // Torr @ CERN, it may be 735 Torr = 980 hPa, altitude 432 m

    // Set electric field (EF) range covered by gas table.
    double EFmin =    100.; // V/cm
    // Flag to request logarithmic spacing.
     bool useLog = true;
    if(nEF==1){
        useLog = false;
        EFmin = EFmax;
    }
    // Set magnetic field (BF) range covered by gas table.
    double BFmin = 0.; // T
    if(BFmax == 0 && nBF > 1){
        std::cout << "Asking for multiple magnetic fields between 0 and 0 doesn't make sense. Reducing to one field." << std::endl;
        nBF = 1;
    }
    if(nBF == 1){
        BFmin = BFmax;
        if(BFmax == 0){
            thetaMax = 0.0;
            nTh = 1;
        }
    }
    // Set angle between BF and EF
    double thetaMin = 0.;


    thetaMax *= Garfield::Pi/180.;
    if(nTh == 1) thetaMin = thetaMax;
    gas->SetFieldGrid(EFmin, EFmax, nEF, useLog,
                      BFmin, BFmax, nBF,
                      thetaMin, thetaMax, nTh);

    std::cout << nEF << " E fields from   " << EFmin << " to " << EFmax << std::endl;
    std::cout << nBF << " B fields from   " << BFmin << " to " << BFmax << std::endl;
    std::cout << nTh << " E-B angles from " << thetaMin << " to " << thetaMax << std::endl << std::endl;

    std::cout << "gas->SetFieldGrid(" << EFmin << ", " << EFmax << ", " <<  nEF << ", " << useLog << ", " <<
        BFmin << ", " <<  BFmax << ", " <<  nBF << ", " <<  thetaMin << ", " << thetaMax << ", " << nTh << ")" << std::endl;
    //  gas->SetFieldGrid(EFmin, EFmax, nEF, useLog);


    gas->SetMaxElectronEnergy(1.e3);

    // To see the full Magboltz output
    gas->EnableDebugging();
    // Specify number of collisions (in multiples of 10^7)
    // over which the electron is traced by Magboltz
    const int ncoll = 10;
    const bool verbose = true;
    gas->GenerateGasTable(ncoll, verbose);
    //  gas->GenerateGasTable(10);

    //gas->DisableDebugging();
    // Save the table.
    TString filename = TString::Format("ar_%.0f_co2_%.0f_n2_%.0f_NTP_%dE%.0f_%dB%.2f.gas",100.-CO2frac,CO2frac-N2frac,N2frac,nEF,EFmax,nBF,BFmax);
    //    TString filename = TString::Format("ar_64_co2_26_cf4_10_NTP_%dE%.0f_%dB%.0f.gas",nEF,EFmax,nBF,BFmax);
    //    TString filename = TString::Format("ar_xx_co2_xx_air_0_NTP_%dE%.0f_%dB%.0f.gas",nEF,EFmax,nBF,BFmax);
    gas->WriteGasFile(filename.Data());

    return 0;
}


int main(int argc, char * argv[]){
    double CO2frac = 30.;
    int nEF = 20;
    double EFmax = 200000.;
    int nBF = 4;
    double BFmax = 1.2;
    int nTh = 5;
    double thetaMax = 180.;

    for(int i = 1; i < argc; i++){
        switch(i){
        case 1: CO2frac = atof(argv[i]); break;
        case 2: nEF = atoi(argv[i]); break;
        case 3: EFmax = atof(argv[i]); break;
        case 4: nBF = atoi(argv[i]); break;
        case 5: BFmax = atof(argv[i]); break;
        case 6: nTh = atoi(argv[i]); break;
        case 7: thetaMax = atof(argv[i]); break;
        }
    }
    makeGas(CO2frac, nEF, EFmax, nBF, BFmax, nTh, thetaMax);
    //makeGas(CO2frac, 25, 1000000., 1,0.,0.,0.);
    //makeContaminatedGas(3.0);
    return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
