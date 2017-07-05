// fectrl.cxx
//
// MIDAS frontend for ESPER frontend boards - ALPHA16, FEAM
//

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>

#include <vector>

#include "tmfe.h"

#include "KOtcp.h"

#include "midas.h"
#include "mjson.h"

#define C(x) ((x).c_str())

#if 0
static int odbReadArraySize(TMFE* mfe, const char*name)
{
   int status;
   HNDLE hdir = 0;
   HNDLE hkey;
   KEY key;

   status = db_find_key(mfe->fDB, hdir, (char*)name, &hkey);
   if (status != DB_SUCCESS)
      return 0;

   status = db_get_key(mfe->fDB, hkey, &key);
   if (status != DB_SUCCESS)
      return 0;

   return key.num_values;
}

static int odbResizeArray(TMFE* mfe, const char*name, int tid, int size)
{
   int oldSize = odbReadArraySize(mfe, name);

   if (oldSize >= size)
      return oldSize;

   int status;
   HNDLE hkey;
   HNDLE hdir = 0;

   status = db_find_key(mfe->fDB, hdir, (char*)name, &hkey);
   if (status != SUCCESS) {
      mfe->Msg(MINFO, "odbResizeArray", "Creating \'%s\'[%d] of type %d", name, size, tid);
      
      status = db_create_key(mfe->fDB, hdir, (char*)name, tid);
      if (status != SUCCESS) {
         mfe->Msg(MERROR, "odbResizeArray", "Cannot create \'%s\' of type %d, db_create_key() status %d", name, tid, status);
         return -1;
      }
         
      status = db_find_key (mfe->fDB, hdir, (char*)name, &hkey);
      if (status != SUCCESS) {
         mfe->Msg(MERROR, "odbResizeArray", "Cannot create \'%s\', db_find_key() status %d", name, status);
         return -1;
      }
   }
   
   mfe->Msg(MINFO, "odbResizeArray", "Resizing \'%s\'[%d] of type %d, old size %d", name, size, tid, oldSize);

   status = db_set_num_values(mfe->fDB, hkey, size);
   if (status != SUCCESS) {
      mfe->Msg(MERROR, "odbResizeArray", "Cannot resize \'%s\'[%d] of type %d, db_set_num_values() status %d", name, size, tid, status);
      return -1;
   }
   
   return size;
}

double OdbGetValue(TMFE* mfe, const std::string& eqname, const char* varname, int i, int nch)
{
   std::string path;
   path += "/Equipment/";
   path += eqname;
   path += "/Settings/";
   path += varname;

   char bufn[256];
   sprintf(bufn,"[%d]", nch);

   double v = 0;
   int size = sizeof(v);

   int status = odbResizeArray(mfe, C(path), TID_DOUBLE, nch);

   if (status < 0) {
      return 0;
   }

   char bufi[256];
   sprintf(bufi,"[%d]", i);

   status = db_get_value(mfe->fDB, 0, C(path + bufi), &v, &size, TID_DOUBLE, TRUE);

   return v;
}
#endif

class Alpha16ctrl: public TMFeRpcHandlerInterface
{
public:
   TMFE* mfe = NULL;
   TMFeEquipment* eq = NULL;
   KOtcpConnection* s = NULL;

#if 0
   static std::vector<std::string> split(const std::string& s)
   {
      std::vector<std::string> v;
      
      std::string::size_type p = 0;
      while (1) {
         std::string::size_type pp = s.find(";", p);
         //printf("p %d, pp %d\n", p, pp);
         if (pp == std::string::npos) {
            v.push_back(s.substr(p));
            return v;
         }
         v.push_back(s.substr(p, pp-p));
         p = pp + 1;
      }
      // not reached
   }

   static std::vector<double> D(std::vector<std::string>& v)
   {
      std::vector<double> vv;
      for (unsigned i=0; i<v.size(); i++) {
         //printf("v[%d] is [%s]\n", i, C(v[i]));
         vv.push_back(atof(C(v[i])));
      }
      return vv;
   }
#endif

   void WR(const char* name, const char* v)
   {
      if (mfe->fShutdown)
         return;
      
      std::string path;
      path += "/Equipment/";
      path += eq->fName;
      path += "/Readback/";
      path += name;
      //printf("Write ODB %s Readback %s: %s\n", C(path), name, v);
      int status = db_set_value(mfe->fDB, 0, C(path), v, strlen(v)+1, 1, TID_STRING);
      if (status != DB_SUCCESS) {
         printf("WR: db_set_value status %d\n", status);
      }
   }
         
   void WVD(const char* name, const std::vector<double> &v)
   {
      if (mfe->fShutdown)
         return;
      
      std::string path;
      path += "/Equipment/";
      path += eq->fName;
      path += "/Variables/";
      path += name;
      //printf("Write ODB %s Variables %s: %s\n", C(path), name, v);
      int status = db_set_value(mfe->fDB, 0, C(path), &v[0], sizeof(double)*v.size(), v.size(), TID_DOUBLE);
      if (status != DB_SUCCESS) {
         printf("WVD: db_set_value status %d\n", status);
      }
   }

   void Read()
   {
      s->fReadTimeout = 5*1000;
      s->fHttpKeepOpen = false;

      KOtcpError e;

      std::vector<std::string> headers;
      //headers.push_back("Accept: vdn.dac.v1");
      
      std::vector<std::string> reply_headers;
      std::string reply_body;

      e = s->HttpGet(headers, "/", &reply_headers, &reply_body);

      if (e.error) {
         eq->SetStatus("http error", "red");
         mfe->Msg(MERROR, "Read", "HttpGet() error %s", e.message.c_str());
         return;
      }

      printf("reply headers:\n");
      for (unsigned i=0; i<reply_headers.size(); i++)
         printf("%d: %s\n", i, reply_headers[i].c_str());

      //printf("json: %s\n", reply_body.c_str());

#if 0
      WR("DI", reply_body_di.c_str());

      MJsonNode* di = MJsonNode::Parse(reply_body_di.c_str());
      //di->Dump();

      std::vector<double> di_counter_value;

      di_counter_value.push_back(di->FindObjectNode("io")->FindObjectNode("di")->GetArray()->at(0)->FindObjectNode("diCounterValue")->GetInt());

      printf("di_counter_value %f\n", di_counter_value[0]);

      delete di;

      WVD("di_counter_value", di_counter_value);
#endif

      eq->SetStatus("Ok", "#00FF00");
   }

   std::string HandleRpc(const char* cmd, const char* args)
   {
      mfe->Msg(MINFO, "HandleRpc", "RPC cmd [%s], args [%s]", cmd, args);
      return "OK";
   }
};

int main(int argc, char* argv[])
{
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   signal(SIGPIPE, SIG_IGN);

   TMFE* mfe = TMFE::Instance();

   TMFeError err = mfe->Connect("fectrl");
   if (err.error) {
      printf("Cannot connect, bye.\n");
      return 1;
   }

   //mfe->SetWatchdogSec(0);

   TMFeCommon *eqc = new TMFeCommon();
   eqc->EventID = 5;
   eqc->FrontendName = "fectrl";
   eqc->LogHistory = 1;
   
   TMFeEquipment* eq = new TMFeEquipment("CTRL");
   eq->Init(eqc);
   eq->SetStatus("Starting...", "white");

   mfe->RegisterEquipment(eq);

   KOtcpConnection* s = new KOtcpConnection("mod8", "http");

   class Alpha16ctrl* a16 = new Alpha16ctrl;

   a16->mfe = mfe;
   a16->eq = eq;
   a16->s = s;

   mfe->RegisterRpcHandler(a16);

   while (!mfe->fShutdown) {

      a16->Read();
         
      for (int i=0; i<5; i++) {
         mfe->PollMidas(1000);
         if (mfe->fShutdown)
            break;
      }
   }

   if (s->fConnected)
      s->Close();
   delete s;
   s = NULL;

   mfe->Disconnect();

   return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
