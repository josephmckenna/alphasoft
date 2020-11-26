///< ###############################################
///< Developed for the Alpha experiment  [Nov. 2020]
///< germano.bonomi@cern.ch [developed by D. Pagano]
///< ###############################################

#include "a2mcMuonGen.h"

a2mcMuonGen::a2mcMuonGen() : 
    mYOffset(0.),
    mZOffset(0.),
    mCylRadius(0.),
    mCylHeight(0.),
    mGenMomentum(0.),
    mTheta(0.),
    mPhi(0.),
    mxSky(0.),
    mySky(0.),
    mzSky(0.),
    mDoHorGen(false),
    mDoSphGen(false),
    mDoCylGen(false)
{
    Init();
}

////////////////////////////////////////////////////////
a2mcMuonGen::~a2mcMuonGen() {

}

////////////////////////////////////////////////////////
void a2mcMuonGen::SetHorizontalGeneration(bool horgen) {
    mDoHorGen = horgen;
}

////////////////////////////////////////////////////////
void a2mcMuonGen::SetSphericalGeneration(bool sphgen) {
    mDoSphGen = sphgen;
}

////////////////////////////////////////////////////////
void a2mcMuonGen::SetCylindricalGeneration(bool cylindr) {
    mDoCylGen = cylindr;
}

// ---------------------------> Vertical generation

////////////////////////////////////////////////////////
void a2mcMuonGen::SetCylinderRadius(double radius) {
    mCylRadius = radius;
}

//////////////////////////////////////////////////////// 
void a2mcMuonGen::SetCylinderHeight(double height) {
    mCylHeight = height;
}

//////////////////////////////////////////////////////// 
void a2mcMuonGen::SetCylinderRadiusAndHeight(double radius, double height) {
    mCylRadius = radius;
    mCylHeight = height;
}

////////////////////////////////////////////////////////  
void a2mcMuonGen::SetYOffset(double offset) {
  mYOffset = offset;
}

////////////////////////////////////////////////////////  
void a2mcMuonGen::SetZOffset(double offset) {
    mZOffset = offset;
}


// ---------------------------> Horizontal generation

////////////////////////////////////////////////////////
void a2mcMuonGen::SetSkyXYZ(double xs, double ys, double zs, int icoord=1) {
//pay attention that it has been modified with respect to Antonietta's method
//now you can generate muons on each of the three upper borders
//of the world (depending on icoord: icoord = 1 means the "usual" world roof)
//mzWorld = zw;
    mxSky = xs;
    mySky = ys;
    mzSky = zs;
    iSky= icoord;
}


// ---------------------------> Spherical generation

////////////////////////////////////////////////////////
void a2mcMuonGen::SetSphRadius(double ys) {
    sphere_radius=ys;
}

////////////////////////////////////////////////////////
void a2mcMuonGen::Generate() {

    ///< Checking which generation have been selected
    Int_t ngen = 0;
    if(mDoHorGen) ngen++;
    if(mDoCylGen) ngen++;
    if(mDoSphGen) ngen++;
    
    if(ngen!=1) {
        std::cout << "a2mcMuonGen::Generate ==> ERROR -> please check the generation settings" << std::endl;
        std::cout << "\t mDoHorGen = " << mDoHorGen << std::endl;
        std::cout << "\t mDoCylGen = " << mDoCylGen << std::endl;
        std::cout << "\t mDoSphGen = " << mDoSphGen << std::endl;
        return;
    }
    
    //// Position ////
    Double_t  phi0     = 0.;
    Double_t  theta0   = 0.;
    
    if  (mDoHorGen) {   //// Generation point on the sky (flat) ////
        mGenPoint[0] = gRandom->Uniform()*mxSky - mxSky/2.;                 // x coordinate
        mGenPoint[1] = gRandom->Uniform()*mySky - mySky/2. + mYOffset;      // y coordinate (vertical axis)
        mGenPoint[2] = gRandom->Uniform()*mzSky - mzSky/2. + mZOffset;      // z coordinate (along the beam axis)
        double gen_fixed[3] = {mxSky,mySky,mzSky};
        mGenPoint[iSky] = gen_fixed[iSky];
    }
    
    if  (mDoCylGen) {   //// Generation point on the cylinder ////
        phi0         = gRandom->Uniform() * 2*M_PI;                  // flat angle in xy projection
        mGenPoint[0] = mCylRadius*cos(phi0);                         // x coordinate
        mGenPoint[1] = mCylRadius*sin(phi0);                         // y coordinate
        mGenPoint[2] = gRandom->Uniform(0.,mCylHeight) + mZOffset;   // z coordinate
    }
/////////////////////////////////////////////////////////////////////

//// Momentum and Theta angle ////                                                                                               
    bool accepted = false;
    double min_theta = 0;
    double max_theta = M_PI/2;
    double min_p = 0;
    double max_p = 1000;
    // 2D sampling
    if(mDoHorGen ||mDoCylGen)
        while (!accepted) {
            double r1t = gRandom->Uniform();
            double r1p = gRandom->Uniform();
            double r2  = gRandom->Uniform();
            mTheta = (max_theta - min_theta)*r1t + min_theta;
            mGenMomentum = (max_p - min_p)*r1p + min_p;
            double n = 2.856-0.655*TMath::Log(mGenMomentum);
            if (n < 0.1) n = 0.1;
            if (mDoHorGen) {
                double ftheta = 1600*TMath::Power(mGenMomentum+2.68, -3.175)*TMath::Power(mGenMomentum, 3.175-2.896)*TMath::Power(TMath::Cos(mTheta), n)*TMath::Sin(mTheta)*TMath::Cos(mTheta);
                if (6.*r2 < ftheta) accepted = true;
            }
    
            if(mDoCylGen)  {
                double ftheta = 1600*TMath::Power(mGenMomentum+2.68, -3.175)*TMath::Power(mGenMomentum, 3.175-2.896)*TMath::Power(TMath::Cos(mTheta), n)*TMath::Power(TMath::Sin(mTheta), 2);
                if (6*r2 < ftheta) accepted = true;
            }
        }
    
    // final reversion of the theta angle
    mTheta = M_PI - mTheta;
    
/////////////////////////////////////////////////////////////////////
    
  
//// Phi angle //// -> new version from Antonietta 6/4/2016
    accepted = false;
    double min_phi = 0;
    double max_phi = 2*M_PI;
    if (mDoHorGen) {
        double r1  = gRandom->Uniform();
        mPhi = (max_phi - min_phi)*r1 + min_phi;
    }
       
    if (mDoCylGen) {
        while (!accepted) {
            double r1 = gRandom->Uniform();
            double r2 = gRandom->Uniform();
            mPhi = (max_phi - min_phi)*r1 + min_phi;
            double fphi = fabs(cos(mPhi));
            if (r2 < fphi) accepted = true;
        }
        mPhi = mPhi + phi0;
        if (mPhi >= 2*M_PI) mPhi -= 2*M_PI;
    }
    
    //Spherical case (warning: in this case, all the variables but phi0 are coupled!!!)
    if (mDoSphGen) {
        //// Generation point on the (half-)sphere
        phi0         = gRandom->Uniform() * 2*M_PI;
        // theta is added to the subsequent 4D generator
        
        //// Momentum and Theta angle and Phi angle////
        // (thetam is taken with respect to the main reference system)
        // (phi0 is added to the end)
        // 4D sampling
        accepted = false;
        min_theta = 0;
        max_theta = M_PI/2;
        min_p = 0;
        max_p = 1000;
        min_phi = 0;
        max_phi = 2*M_PI;

        while (!accepted) {

            // as a first approximation, the flux on the sphere is supposed to be constant ( -> uniformly distrubuted points)
            Double_t auxiliary_var = gRandom->Uniform();
            theta0       = TMath::ACos(auxiliary_var);
            //but: the following instrutions will modify the previous distribution according to the cosmic flux which is function of theta0

            double r1t = gRandom->Uniform();
            mTheta = (max_theta - min_theta)*r1t + min_theta;

            double r1p = gRandom->Uniform();
            mGenMomentum = (max_p - min_p)*r1p + min_p;
            
            double r1  = gRandom->Uniform();
            mPhi = (max_phi - min_phi)*r1 + min_phi;

            double r2  = gRandom->Uniform();

            double n = 2.856-0.655*TMath::Log(mGenMomentum);
            if (n < 0.1) n = 0.1;
  
            double ftheta = 1600*TMath::Power(mGenMomentum+2.68, -3.175)*TMath::Power(mGenMomentum, 3.175-2.896)*TMath::Power(TMath::Cos(mTheta), n)*
            TMath::Abs(
                 TMath::Sin(mTheta)*TMath::Sin(theta0)*TMath::Cos(mPhi)+
//                  TMath::Sin(mTheta)*TMath::Sin(theta0)*TMath::Sin(mPhi)+
                    TMath::Cos(mTheta)*TMath::Cos(theta0)
            )*TMath::Sin(mTheta);
           if (11*r2 < ftheta) accepted = true;
        }
        
        mGenPoint[2] = sphere_radius * TMath::Sin(theta0) * TMath::Cos(phi0) + mZOffset;
        mGenPoint[0] = sphere_radius * TMath::Sin(theta0) * TMath::Sin(phi0);
        mGenPoint[1] = sphere_radius * TMath::Cos(theta0) + mYOffset;
//        mGenPoint[1] -= sphere_radius/2.;  // traslation downward by half of the radius

        mTheta = M_PI - mTheta;

        mPhi = mPhi + phi0;
        if (mPhi >= 2*M_PI) mPhi -= 2*M_PI;

        mPhi+= M_PI;
        if (mPhi >= 2*M_PI) mPhi -= 2*M_PI;
    } 
}

       

////////////////////////////////////////////////////////
void a2mcMuonGen::Init() {
  p_min = {0.0000,  0.0115,  0.0195,  0.0270,  0.0345,  0.0405,  0.0475,  0.0540,  0.0600,  0.0660,  
			  0.0720,  0.0780,  0.0830,  0.0890,  0.0945,  0.1005,  0.1055,  0.1110,  0.1165,  0.1215,  
			  0.1270,  0.1325,  0.1375,  0.1430,  0.1480,  0.1535,  0.1585,  0.1640,  0.1690,  0.1740,  
			  0.1790,  0.1840,  0.1895,  0.1945,  0.1995,  0.2050,  0.2100,  0.2145,  0.2200,  0.2250,  
			  0.2300,  0.2350,  0.2400,  0.2450,  0.2500,  0.2550,  0.2600,  0.2650,  0.2705,  0.2755,  
			  0.2805,  0.2855,  0.2905,  0.2960,  0.3010,  0.3060,  0.3110,  0.3160,  0.3210,  0.3260,  
			  0.3310,  0.3360,  0.3410,  0.3465,  0.3515,  0.3570,  0.3620,  0.3670,  0.3720,  0.3770,  
			  0.3825,  0.3875,  0.3930,  0.3980,  0.4030,  0.4080,  0.4135,  0.4190,  0.4240,  0.4290,  
			  0.4345,  0.4400,  0.4450,  0.4500,  0.4555,  0.4610,  0.4660,  0.4715,  0.4770,  0.4820,  
			  0.4875,  0.4930,  0.4980,  0.5035,  0.5090,  0.5145,  0.5200,  0.5250,  0.5310,  0.5360,  
			  0.5420,  0.5470,  0.5530,  0.5580,  0.5640,  0.5695,  0.5750,  0.5805,  0.5860,  0.5920,  
			  0.5975,  0.6030,  0.6090,  0.6145,  0.6200,  0.6260,  0.6315,  0.6375,  0.6430,  0.6490,  
			  0.6550,  0.6605,  0.6665,  0.6720,  0.6780,  0.6840,  0.6900,  0.6960,  0.7020,  0.7080,  
			  0.7140,  0.7200,  0.7260,  0.7320,  0.7380,  0.7440,  0.7500,  0.7565,  0.7625,  0.7690,  
			  0.7750,  0.7810,  0.7875,  0.7935,  0.8000,  0.8060,  0.8125,  0.8190,  0.8250,  0.8320,  
			  0.8380,  0.8445,  0.8510,  0.8575,  0.8640,  0.8705,  0.8770,  0.8840,  0.8905,  0.8970,  
			  0.9040,  0.9105,  0.9170,  0.9240,  0.9305,  0.9375,  0.9440,  0.9510,  0.9580,  0.9650,  
			  0.9720,  0.9790,  0.9860,  0.9930,  1.0000,  1.0070,  1.0140,  1.0210,  1.0285,  1.0355,  
			  1.0430,  1.0500,  1.0575,  1.0650,  1.0720,  1.0800,  1.0870,  1.0945,  1.1020,  1.1095,  
			  1.1170,  1.1245,  1.1320,  1.1400,  1.1475,  1.1550,  1.1630,  1.1710,  1.1785,  1.1865,  
			  1.1945,  1.2020,  1.2100,  1.2180,  1.2260,  1.2345,  1.2425,  1.2505,  1.2590,  1.2670,  
			  1.2755,  1.2840,  1.2920,  1.3005,  1.3090,  1.3175,  1.3260,  1.3345,  1.3430,  1.3520,  
			  1.3605,  1.3690,  1.3780,  1.3870,  1.3960,  1.4045,  1.4135,  1.4225,  1.4320,  1.4410,  
			  1.4500,  1.4595,  1.4685,  1.4780,  1.4870,  1.4965,  1.5060,  1.5155,  1.5250,  1.5350,  
			  1.5445,  1.5540,  1.5640,  1.5740,  1.5835,  1.5935,  1.6035,  1.6135,  1.6240,  1.6340,  
			  1.6445,  1.6545,  1.6650,  1.6755,  1.6860,  1.6965,  1.7070,  1.7175,  1.7285,  1.7390,  
			  1.7500,  1.7610,  1.7720,  1.7830,  1.7940,  1.8055,  1.8165,  1.8280,  1.8395,  1.8510,  
			  1.8625,  1.8740,  1.8860,  1.8975,  1.9095,  1.9215,  1.9335,  1.9455,  1.9575,  1.9700,  
			  1.9820,  1.9945,  2.0070,  2.0200,  2.0325,  2.0450,  2.0580,  2.0710,  2.0840,  2.0970,  
			  2.1100,  2.1230,  2.1370,  2.1500,  2.1640,  2.1780,  2.1910,  2.2060,  2.2200,  2.2340,  
			  2.2480,  2.2620,  2.2760,  2.2910,  2.3060,  2.3200,  2.3350,  2.3500,  2.3650,  2.3800,  
			  2.3960,  2.4110,  2.4270,  2.4420,  2.4580,  2.4740,  2.4900,  2.5060,  2.5230,  2.5390,  
			  2.5560,  2.5720,  2.5890,  2.6060,  2.6240,  2.6410,  2.6580,  2.6760,  2.6940,  2.7120,  
			  2.7300,  2.7480,  2.7660,  2.7850,  2.8030,  2.8220,  2.8410,  2.8610,  2.8800,  2.9000,  
			  2.9190,  2.9390,  2.9600,  2.9800,  3.0000,  3.0210,  3.0420,  3.0630,  3.0840,  3.1065,  
			  3.1275,  3.1500,  3.1725,  3.1950,  3.2175,  3.2400,  3.2640,  3.2865,  3.3105,  3.3345,  
			  3.3585,  3.3825,  3.4080,  3.4320,  3.4575,  3.4830,  3.5085,  3.5355,  3.5610,  3.5880,  
			  3.6150,  3.6420,  3.6705,  3.6975,  3.7260,  3.7545,  3.7845,  3.8130,  3.8430,  3.8730,  
			  3.9030,  3.9345,  3.9660,  3.9975,  4.0290,  4.0620,  4.0940,  4.1280,  4.1620,  4.1960,  
			  4.2300,  4.2660,  4.3020,  4.3380,  4.3740,  4.4120,  4.4500,  4.4880,  4.5260,  4.5660,  
			  4.6060,  4.6460,  4.6880,  4.7300,  4.7720,  4.8160,  4.8590,  4.9040,  4.9520,  4.9970,  
			  5.0420,  5.0900,  5.1410,  5.1890,  5.2400,  5.2910,  5.3420,  5.3960,  5.4500,  5.5040,  
			  5.5610,  5.6180,  5.6780,  5.7350,  5.7980,  5.8580,  5.9210,  5.9870,  6.0530,  6.1190,  
			  6.1880,  6.2570,  6.3290,  6.4010,  6.4790,  6.5540,  6.6320,  6.7120,  6.7960,  6.8800,  
			  6.9650,  7.0550,  7.1450,  7.2400,  7.3350,  7.4350,  7.5350,  7.6400,  7.7500,  7.8600,  
			  7.9750,  8.0900,  8.2100,  8.3350,  8.4650,  8.6000,  8.7400,  8.8800,  9.0300,  9.1800,  
			  9.3400,  9.5100,  9.6800,  9.8600, 10.0500, 10.2400, 10.4500, 10.6600, 10.8800, 11.1100, 
			  11.360, 11.6200, 11.9000, 12.1800, 12.4800, 12.8000, 13.1500, 13.5100, 13.9100, 14.3300, 
			  14.7700 };
}
