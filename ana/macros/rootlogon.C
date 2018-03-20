{
  cout<<"ROOT "<<gROOT->GetVersion()<<" on "<<gSystem->HostName()<<endl;

  gSystem->Load("libMinuit");
  gSystem->Load("libGeom");
  gSystem->Load("libAGTPC");
  gSystem->Load("libAGUTILS");
  gSystem->Load("libAGDAQ");
  gSystem->Load("libALPHAG4");

  gSystem->AddIncludePath(" -I$AGTPC_ANALYSIS -I$SOURCE_TPC/include -I$ANALYSIS_TPC/include -I$GARFIELDPP -I$AGANA");

  gSystem->AddDynamicPath(" $ANALYSIS_TPC/ ");
//  gSystem->AddLinkedLibs(" $ANALYSIS_TPC/libAGTPC.so $ANALYSIS_TPC/libAGDAQ.so $ANALYSIS_TPC/libAGUTILS.so $ANALYSIS_TPC/libALPHAG4.so ");

  //  cout<<gSystem->Now().AsString()<<endl;
}


