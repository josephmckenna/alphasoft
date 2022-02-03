#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#ifdef BUILD_AG
#pragma link C++ class  TStoreEvent+;
#pragma link C++ class  TStoreHelix+;
#pragma link C++ class  TStoreLine+;
#endif

#ifdef BUILD_AG
#pragma link C++ class  TBarEvent+;
#pragma link C++ class  TBarEndHit+;
#pragma link C++ class  TBarHit+;
#pragma link C++ class  TBarSimpleTdcHit+;
#endif

#endif

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
