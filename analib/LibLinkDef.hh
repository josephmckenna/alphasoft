#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class  TSeq_Event+;
#pragma link C++ class  TSequencerState+;
#pragma link C++ class  TSequencerDigitalOut+;
#pragma link C++ class  TSequencerAnalogueOut+;
#pragma link C++ class  TSequencerTriggerIn+;


#pragma link C++ class TChronoChannel+;


#pragma link C++ class TCbFIFOEvent+;
#pragma link C++ class TChronoChannelName+;

#pragma link C++ class TChronoBoardCounter+;

#pragma link C++ class  TSeq_Dump+;
//Spill parent classes
#pragma link C++ class  TSpill+;
#pragma link C++ class  TSpillScalerData+;
#pragma link C++ class  TSpillSequencerData+;
//Generic child class
#pragma link C++ class  TInfoSpill+;
//Experiment specific Spill classes (child classses)
#if BUILD_A2
  #pragma link C++ class  TA2Spill+;
  #pragma link C++ class  TA2SpillScalerData+;
  #pragma link C++ class  TA2SpillSequencerData+;
#endif
#if BUILD_AG
  #pragma link C++ class  TAGSpill+;
  #pragma link C++ class  TAGSpillScalerData+;
  #pragma link C++ class  TAGSpillSequencerData+;
#endif
#pragma link C++ class Seq+;
#pragma link C++ class Seq_DriverConsts+;
#pragma link C++ class TSequencerDriver;
#pragma link C++ class SeqXML_Obj+;
#pragma link C++ class SeqXML+;
#pragma link C++ class SeqXML_DriverConsts+;
#pragma link C++ class SeqXML_AOChn+;
#pragma link C++ class SeqXML_HVElec+;
#pragma link C++ class SeqXML_AOConfig+;
#pragma link C++ class SeqXML_State+;
#pragma link C++ class SeqXML_Event+;
#pragma link C++ class SeqXML_ChainLink+;

#pragma link C++ class  TStoreLabVIEWEvent+;

#pragma link C++ class TAnalysisReport+;

#if BUILD_A2
#pragma link C++ class TA2AnalysisReport+;
#endif
#if BUILD_AG
#pragma link C++ class TAGAnalysisReport+;
#endif
#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
