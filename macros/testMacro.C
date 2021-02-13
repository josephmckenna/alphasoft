//#include "TA2Plot.h"
//Plot something basic... lets see vertices between 0 and 350 seconds in run 45000
TA2Plot* basic = new TA2Plot();
//Set the time window we want
basic->AddTimeGate(
    45000, //runNumber
    0.,    //Start time of our plot
    350.); //End time of our plot

//With everything set... load data from disk
basic->LoadData();
//Render TA2Plot
TCanvas* BasicCanvas=basic->DrawCanvas("Example plot of run 45000");
//Draw canvas
BasicCanvas->Draw();