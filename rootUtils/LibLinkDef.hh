
#ifdef __CINT__
//#pragma link off all globals;
//#pragma link off all classes;
//#pragma link off all functions;

#pragma link C++ class TAPlot+;

#ifdef BUILD_AG
#pragma link C++ class TAGPlot+;
#endif

#ifdef BUILD_A2
#pragma link C++ class TA2Plot+;
#pragma link C++ class TA2Plot_Filler;
#endif

#ifdef BUILD_AG
#pragma link C++ function RootUtils+;
#endif
#ifdef BUILD_A2
#pragma link C++ function A2RootUtils+;
#endif

#endif
