#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cmath>
#include <string>

#include "LookUpTable.hh"
#include "TPCconstants.hh"

int main(int argc, char* argv[])
{
   double B = 1.;
   double co2 = 0.3;
   int wire = 0;

   double tmax = 10000.;        // ns
   double dt = 1.;        // ns

   for(int i = 1; i < argc; i++){
      std::string arg = argv[i];
      if(arg == std::string("-h")){
         std::cout << "Usage: " << argv[0] << " [-h] [-B <Bfield>] [-c <CO2 frac>] [-s <time step>] [anode wire]" << std::endl;
         return 0;
      } else if(arg == std::string("-B")){
         B = atof(argv[++i]);
      } else if(arg == std::string("-c")){
         co2 = atof(argv[++i]);
      } else if(arg == std::string("-s")){
         dt = atof(argv[++i]);
      } else {
         wire = atoi(arg.c_str());
      }
   }

   std::cout << "Time step: " << dt << std::endl;
   std::cout << "Anode:     " << wire << std::endl;

   double phi0 = double(wire+0.5)*_anodepitch;
   double zed = 0.;

   LookUpTable fSTR(co2, B); // uniform field version (simulation)
   std::ofstream f("driftline.dat");
   // f << "#t\tr\tphi\tx\ty\t" << std::endl;

   for(double time = 0.; time < tmax; time += dt){
      double rad = fSTR.GetRadius( time , zed );
      if(rad < _cathradius){
         std::cout << "Maximum drift time = " << time << " ns" << std::endl;
         break;
      }
      double phi = phi0 - fSTR.GetAzimuth( time , zed );
      f << time << '\t' << rad << '\t' << phi << '\t' << rad*cos(phi) << '\t' << rad*sin(phi) << std::endl;
   }
   return 0;
}


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
