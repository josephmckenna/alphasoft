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

class Alpha16ctrl // : public TMFeRpcHandlerInterface
{
public:
   TMFE* mfe = NULL;
   TMFeEquipment* eq = NULL;
   KOtcpConnection* s = NULL;

   std::string fOdbName;

   bool fVerbose = false;

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

   void WR(const char* mid, const char* vid, const char* v)
   {
      if (mfe->fShutdown)
         return;
      
      std::string path;
      path += "/Equipment/";
      path += eq->fName;
      path += "/Readback/";
      path += fOdbName;
      path += "/";
      path += mid;
      path += "/";
      path += vid;
      //printf("Write ODB %s : %s\n", C(path), v);
      int status = db_set_value(mfe->fDB, 0, C(path), v, strlen(v)+1, 1, TID_STRING);
      if (status != DB_SUCCESS) {
         printf("WR: db_set_value status %d\n", status);
      }
   }
         
   void WRI(const char* mid, const char* vid, const std::vector<int>& v)
   {
      if (mfe->fShutdown)
         return;
      
      std::string path;
      path += "/Equipment/";
      path += eq->fName;
      path += "/Readback/";
      path += fOdbName;
      path += "/";
      path += mid;
      path += "/";
      path += vid;
      //printf("Write ODB %s : %s\n", C(path), v);
      int status = db_set_value(mfe->fDB, 0, C(path), &v[0], sizeof(int)*v.size(), v.size(), TID_INT);
      if (status != DB_SUCCESS) {
         printf("WR: db_set_value status %d\n", status);
      }
   }
         
   void WRB(const char* mid, const char* vid, const std::vector<bool>& v)
   {
      if (mfe->fShutdown)
         return;
      
      std::string path;
      path += "/Equipment/";
      path += eq->fName;
      path += "/Readback/";
      path += fOdbName;
      path += "/";
      path += mid;
      path += "/";
      path += vid;
      //printf("Write ODB %s : %s\n", C(path), v);

      BOOL *bb = new BOOL[v.size()];
      for (unsigned i=0; i<v.size(); i++) {
         bb[i] = v[i];
      }

      int status = db_set_value(mfe->fDB, 0, C(path), bb, sizeof(BOOL)*v.size(), v.size(), TID_BOOL);
      if (status != DB_SUCCESS) {
         printf("WR: db_set_value status %d\n", status);
      }

      delete bb;
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

   std::vector<int> JsonToIntArray(const MJsonNode* n)
   {
      std::vector<int> vi;
      const MJsonNodeVector *a = n->GetArray();
      if (a) {
         for (unsigned i=0; i<a->size(); i++) {
            const MJsonNode* ae = a->at(i);
            if (ae) {
               vi.push_back(ae->GetInt());
            }
         }
      }
      return vi;
   }

   std::vector<bool> JsonToBoolArray(const MJsonNode* n)
   {
      std::vector<bool> vb;
      const MJsonNodeVector *a = n->GetArray();
      if (a) {
         for (unsigned i=0; i<a->size(); i++) {
            const MJsonNode* ae = a->at(i);
            if (ae) {
               vb.push_back(ae->GetBool());
            }
         }
      }
      return vb;
   }

   std::vector<std::string> GetModules()
   {
      std::vector<std::string> modules;

      std::vector<std::string> headers;
      std::vector<std::string> reply_headers;
      std::string reply_body;

      KOtcpError e = s->HttpGet(headers, "/read_node?includeMods=y", &reply_headers, &reply_body);

      if (e.error) {
         eq->SetStatus("http error", "red");
         mfe->Msg(MERROR, "GetModules", "HttpGet() error %s", e.message.c_str());
         return modules;
      }

      MJsonNode* jtree = MJsonNode::Parse(reply_body.c_str());
      //jtree->Dump();

      const MJsonNode* m = jtree->FindObjectNode("module");
      if (m) {
         const MJsonNodeVector *ma = m->GetArray();
         if (ma) {
            for (unsigned i=0; i<ma->size(); i++) {
               const MJsonNode* mae = ma->at(i);
               if (mae) {
                  //mae->Dump();
                  const MJsonNode* maek = mae->FindObjectNode("key");
                  const MJsonNode* maen = mae->FindObjectNode("name");
                  if (maek && maen) {
                     if (fVerbose)
                        printf("module [%s] %s\n", maek->GetString().c_str(), maen->GetString().c_str());
                     modules.push_back(maek->GetString());
                  }
               }
            }
         }
      }

      delete jtree;

      return modules;
   }

   std::vector<std::string> GetVariables(const std::string& mid)
   {
      std::vector<std::string> variables;

      std::vector<std::string> headers;
      std::vector<std::string> reply_headers;
      std::string reply_body;

      std::string url;
      url += "/read_module?includeVars=y&mid=";
      url += mid.c_str();
      url += "&includeData=y";

      KOtcpError e = s->HttpGet(headers, url.c_str(), &reply_headers, &reply_body);

      if (e.error) {
         eq->SetStatus("http error", "red");
         mfe->Msg(MERROR, "GetModules", "HttpGet() error %s", e.message.c_str());
         return variables;
      }

      MJsonNode* jtree = MJsonNode::Parse(reply_body.c_str());
      //jtree->Dump();

      const MJsonNode* v = jtree->FindObjectNode("var");
      if (v) {
         //v->Dump();
         const MJsonNodeVector *va = v->GetArray();
         if (va) {
            for (unsigned i=0; i<va->size(); i++) {
               const MJsonNode* vae = va->at(i);
               if (vae) {
                  //vae->Dump();
                  const MJsonNode* vaek = vae->FindObjectNode("key");
                  const MJsonNode* vaet = vae->FindObjectNode("type");
                  const MJsonNode* vaed = vae->FindObjectNode("d");
                  if (vaek && vaet && vaed) {
                     std::string vid = vaek->GetString();
                     int type = vaet->GetInt();
                     if (fVerbose)
                        printf("mid [%s] vid [%s] type %d json value %s\n", mid.c_str(), vid.c_str(), type, vaed->Stringify().c_str());
                     if (type == 1 || type == 2 || type == 3 || type == 4 || type == 5 || type == 6) {
                        WRI(mid.c_str(), vid.c_str(), JsonToIntArray(vaed));
                     } else if (type == 11) {
                        WR(mid.c_str(), vid.c_str(), vaed->GetString().c_str());
                     } else if (type == 12) {
                        WRB(mid.c_str(), vid.c_str(), JsonToBoolArray(vaed));
                     } else {
                        WR(mid.c_str(), vid.c_str(), vaed->Stringify().c_str());
                     }
                     variables.push_back(vid);
                  }
               }
            }
         }
      }

      delete jtree;

      return variables;
   }

   bool Read()
   {
      s->fReadTimeout = 5*1000;
      s->fHttpKeepOpen = false;

      if (fVerbose)
         printf("Reading %s\n", fOdbName.c_str());

      std::vector<std::string> modules = GetModules();

      if (modules.size() < 1)
         return false;

      for (unsigned i=0; i<modules.size(); i++) {
         if (modules[i] == "sp32wv")
            continue;
         if (modules[i] == "sp16wv")
            continue;
         std::vector<std::string> variables = GetVariables(modules[i]);
      }

#if 0
      KOtcpError e;

      std::vector<std::string> headers;
      //headers.push_back("Accept: vdn.dac.v1");
      
      std::vector<std::string> reply_headers;
      std::string reply_body;

      //e = s->HttpGet(headers, "/read_var?vid=elf_build_str&mid=board&offset=0&len=0&dataOnly=y", &reply_headers, &reply_body);
      //e = s->HttpGet(headers, "/read_node?includeMods=y&includeVars=y&includeAttrs=y", &reply_headers, &reply_body);
      //e = s->HttpGet(headers, "/read_node?includeMods=y", &reply_headers, &reply_body);
      e = s->HttpGet(headers, "/read_module?includeVars=y&mid=board&includeData=y", &reply_headers, &reply_body);

      if (e.error) {
         eq->SetStatus("http error", "red");
         mfe->Msg(MERROR, "Read", "HttpGet() error %s", e.message.c_str());
         return false;
      }

      //printf("reply headers:\n");
      //for (unsigned i=0; i<reply_headers.size(); i++)
      //   printf("%d: %s\n", i, reply_headers[i].c_str());

      //printf("json: %s\n", reply_body.c_str());

      //WR("DI", reply_body_di.c_str());

      MJsonNode* jtree = MJsonNode::Parse(reply_body.c_str());
      jtree->Dump();

#if 0
      std::vector<double> di_counter_value;

      di_counter_value.push_back(di->FindObjectNode("io")->FindObjectNode("di")->GetArray()->at(0)->FindObjectNode("diCounterValue")->GetInt());

      printf("di_counter_value %f\n", di_counter_value[0]);
#endif

      delete jtree;

      //WVD("di_counter_value", di_counter_value);
#endif

      return true;
   }

#if 0
   std::string HandleRpc(const char* cmd, const char* args)
   {
      mfe->Msg(MINFO, "HandleRpc", "RPC cmd [%s], args [%s]", cmd, args);
      return "OK";
   }
#endif
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

   std::vector<Alpha16ctrl*> a16ctrl;

   std::vector<std::string> a16names;

   a16names.push_back("mod7");
   a16names.push_back("mod8");

   for (unsigned i=0; i<a16names.size(); i++) {
      KOtcpConnection* s = new KOtcpConnection(a16names[i].c_str(), "http");

      class Alpha16ctrl* a16 = new Alpha16ctrl;

      a16->mfe = mfe;
      a16->eq = eq;
      a16->s = s;
      a16->fOdbName = a16names[i];

      a16ctrl.push_back(a16);
   }

   //mfe->RegisterRpcHandler(a16);

   while (!mfe->fShutdown) {

      int countOk = 0;
      for (unsigned i=0; i<a16ctrl.size(); i++) {
         bool ok = a16ctrl[i]->Read();
         if (ok)
            countOk += 1;
      }

      {
         char buf[256];
         sprintf(buf, "%d A16 Ok", countOk);
         eq->SetStatus(buf, "#00FF00");
      }

      for (int i=0; i<5; i++) {
         mfe->PollMidas(1000);
         if (mfe->fShutdown)
            break;
      }
   }

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
