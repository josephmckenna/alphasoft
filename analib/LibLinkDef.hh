#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#ifdef BUILD_AG
#pragma link C++ class  TStoreEvent+;
#pragma link C++ class  TStoreHelix+;
#pragma link C++ class  TStoreLine+;
#endif
#pragma link C++ class  TSeq_Event+;
#pragma link C++ class  TSequencerState+;
#pragma link C++ class  DigitalOut+;
#pragma link C++ class  AnalogueOut+;
#pragma link C++ class  TriggerIn+;
#pragma link C++ class  TChrono_Event+;
#pragma link C++ class  TChronoChannelName+;
#pragma link C++ class  TSeq_Dump+;
#ifdef BUILD_AG
#pragma link C++ class  TBarEvent+;
#pragma link C++ class  EndHit+;
#pragma link C++ class  BarHit+;
#endif
//Spill parent classes
#pragma link C++ class  TSpill+;
#pragma link C++ class  TSpillScalerData+;
#pragma link C++ class  TSpillSequencerData+;
//Experiment specific Spill classes (child classses)
#ifdef BUILD_AG
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

#pragma link C++ class TStoreGEMEventHeader+;
#pragma link C++ class TLVTimestamp+;
#pragma link C++ class TStoreGEMData<double>+;
#pragma link C++ class TStoreGEMData<float>+;
#pragma link C++ class TStoreGEMData<bool>+;
#pragma link C++ class TStoreGEMData<int32_t>+;
#pragma link C++ class TStoreGEMData<uint32_t>+;
#pragma link C++ class TStoreGEMData<uint16_t>+;
#pragma link C++ class TStoreGEMData<char>+;

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
