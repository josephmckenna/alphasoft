
#ifdef __CINT__
//#pragma link off all globals;
//#pragma link off all classes;
//#pragma link off all functions;

#pragma link C++ class TAPlot+;

#pragma link C++ class TTimeWindows;
#pragma link C++ class TVertexEvents;

#pragma link C++ class TEnvDataPlot;
#pragma link C++ class TEnvData;
#pragma link C++ class TFELabVIEWData;
#pragma link C++ class TFEGEMData;



#ifdef BUILD_AG
#pragma link C++ class TAGPlot+;
#endif

#ifdef BUILD_A2
#pragma link C++ class TA2Plot+;
#pragma link C++ class TA2Plot_Filler;
#pragma link C++ class TSISPlotEvents;
#pragma link C++ class TFEGEMData;
#pragma link C++ class TFELabVIEWData;
#endif

#ifdef BUILD_AG
#pragma link C++ function RootUtils+;
#endif
#ifdef BUILD_A2
#pragma link C++ function A2RootUtils+;
#endif

#endif
