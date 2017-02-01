/********************************************************************\

  Name:         tmfe.h
  Created by:   Konstantin Olchanski - TRIUMF

  Contents:     C++ MIDAS frontend

\********************************************************************/

#ifndef TMFE_H
#define TMFE_H

#include <string>
//#include <vector>
//#include "midas.h"

// from midas.h

#define TID_BYTE      1       /**< unsigned byte         0       255    */
#define TID_SBYTE     2       /**< signed byte         -128      127    */
#define TID_CHAR      3       /**< single character      0       255    */
#define TID_WORD      4       /**< two bytes             0      65535   */
#define TID_SHORT     5       /**< signed word        -32768    32767   */
#define TID_DWORD     6       /**< four bytes            0      2^32-1  */
#define TID_INT       7       /**< signed dword        -2^31    2^31-1  */
#define TID_BOOL      8       /**< four bytes bool       0        1     */
#define TID_FLOAT     9       /**< 4 Byte float format                  */
#define TID_DOUBLE   10       /**< 8 Byte float format                  */
#define TID_BITFIELD 11       /**< 32 Bits Bitfield      0  111... (32) */
#define TID_STRING   12       /**< zero terminated string               */
#define TID_ARRAY    13       /**< array with unknown contents          */
#define TID_STRUCT   14       /**< structure with fixed length          */
#define TID_KEY      15       /**< key in online database               */
#define TID_LINK     16       /**< link in online database              */
#define TID_LAST     17       /**< end of TID list indicator            */

class TMFeError
{
 public:
   int error;
   std::string error_string;

 public:
   TMFeError() { // default ctor for success
      error = 0;
      error_string = "success";
   }

   TMFeError(int status, const std::string& str) { // ctor
      error = status;
      error_string = str;
   }
};

class TMFeCommon
{
 public:
   int EventID;
   int TriggerMask;
   std::string Buffer;
   int Type;
   int Source;
   std::string Format;
   bool Enabled;
   int ReadOn;
   int Period;
   double EventLimit;
   int NumSubEvents;
   int LogHistory;
   std::string FrontendHost;
   std::string FrontendName;
   std::string FrontendFileName;
   std::string Status;
   std::string StatusColor;
   bool Hidden;

 public:
   TMFeCommon(); // ctor
};

TMFeError WriteToODB(const char* odbpath, const TMFeCommon* common);
TMFeError ReadFromODB(const char* odbpath, const TMFeCommon* defaults, TMFeCommon* common);

class TMFeEquipment
{
 public:
   std::string fName;
   TMFeCommon *fCommon;
   TMFeCommon *fDefaultCommon;
   int fBuffer;
   int fSerial;

 public:
   double fStatEvents;
   double fStatBytes;
   double fStatEpS; // events/sec
   double fStatKBpS; // kbytes/sec (factor 1000, not 1024)

   double fStatLastTime;
   double fStatLastEvents;
   double fStatLastBytes;

 public:
   TMFeEquipment(const char* name); // ctor
   TMFeError Init(TMFeCommon* defaults); // ctor
   TMFeError SendData(const char* data, int size);
   TMFeError ComposeEvent(char* pevent, int size);
   TMFeError BkInit(char* pevent, int size);
   void* BkOpen(char* pevent, const char* bank_name, int bank_type);
   TMFeError BkClose(char* pevent, void* ptr);
   int BkSize(const char* pevent);
   TMFeError SendEvent(const char* pevent);
   TMFeError WriteStatistics();
};

class TMFE
{
 public:
   
   //typedef void (*TransitionHandler)(int transition,int run_number,int trans_time);
   //typedef void (*EventHandler)(const void*header,const void*data,int length);
   //std::vector<TMHandlerInterface*> fHandlers;
   
 public:
   
   std::string fHostname; ///< hostname where the mserver is running, blank if using shared memory
   std::string fExptname; ///< experiment name, blank if only one experiment defined in exptab
   
 private:
   /// TMFE is a singleton class: only one
   /// instance is allowed at any time
   static TMFE* gfMFE;
   
   TMFE(); ///< default constructor is private for singleton classes
   virtual ~TMFE(); ///< destructor is private for singleton classes
   
 public:
   
   /// TMidasOnline is a singleton class. Call instance() to get a reference
   /// to the one instance of this class.
   static TMFE* Instance();
   
   TMFeError Connect(const char* progname, const char*hostname = NULL, const char*exptname = NULL);
   TMFeError Disconnect();
   TMFeError SetWatchdogSec(int sec);
   TMFeError RegisterEquipment(TMFeEquipment*eq);
   
   /// Check for MIDAS events (run transitions, data requests)
   //bool poll(int mdelay);
   
   // run transitions functions
   
   /// Ask MIDAS to tell us about run transitions
   //void registerTransitions();
   
   /// Specify user handlers for run transitions
   //void setTransitionHandlers(TransitionHandler start,TransitionHandler stop,TransitionHandler pause,TransitionHandler resume);
   
   /// Check for pending transitions, call user handlers. Returns "true" if there were transitions.
   //bool checkTransitions();
};

#endif
/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
