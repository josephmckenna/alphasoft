
#ifdef __CINT__
//#pragma link off all globals;
//#pragma link off all classes;
//#pragma link off all functions;

#pragma link C++ class TAPlot+;

#pragma link C++ class TAPlotTimeWindows;
#pragma link C++ class TAPlotVertexEvents;

#pragma link C++ class TAPlotEnvDataPlot;
#pragma link C++ class TAPlotEnvData;
#pragma link C++ class TAPlotFELabVIEWData;
#pragma link C++ class TAPlotFEGEMData;



#ifdef BUILD_AG
#pragma link C++ class TAGPlot+;
#endif

#ifdef BUILD_A2
#pragma link C++ class TA2Plot+;
#pragma link C++ class TA2Plot_Filler;
#pragma link C++ class TA2PlotSISPlotEvents;
#endif

#ifdef BUILD_AG
#pragma link C++ function RootUtils+;
#endif
#ifdef BUILD_A2
#pragma link C++ function A2RootUtils+;
#endif

#pragma link C++ class HistoStacker;

#endif
