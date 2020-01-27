#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class  TStoreEvent+;
#pragma link C++ class  TStoreHelix+;
#pragma link C++ class  TStoreLine+;
#pragma link C++ class  TSeq_Event+;
#pragma link C++ class  TSequencerState+;
#pragma link C++ class  DigitalOut+;
#pragma link C++ class  AnalogueOut+;
#pragma link C++ class  TriggerIn+;
#pragma link C++ class  TChrono_Event+;
#pragma link C++ class  TChronoChannelName+;
#pragma link C++ class  TSeq_Dump+;
#pragma link C++ class  TBarEvent+;
#pragma link C++ class  BarHit+;

//Spill parent classes
#pragma link C++ class  TSpill+;
#pragma link C++ class  TSpillScalerData+;
#pragma link C++ class  TSpillSequencerData+;
//Experiment specific Spill classes (child classses)
#pragma link C++ class  TA2Spill+;
#pragma link C++ class  TA2SpillScalerData+;
#pragma link C++ class  TA2SpillSequencerData+;

#pragma link C++ class  TAGSpill+;
#pragma link C++ class  TAGSpillScalerData+;

#pragma link C++ class  AnaSettings+;
#pragma link C++ function  RootUtils+;
#pragma link C++ class TAGPlot+;

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

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
