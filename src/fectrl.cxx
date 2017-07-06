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
#include <map>

#include "tmfe.h"

#include "KOtcp.h"

#include "midas.h"
#include "mjson.h"

#define C(x) ((x).c_str())

static std::string toString(int value)
{
   char buf[256];
   sprintf(buf, "%d", value);
   return buf;
}

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

int OdbGetInt(TMFE* mfe, const char* path)
{
   int v = 0;
   int size = sizeof(v);

   int status = db_get_value(mfe->fDB, 0, path, &v, &size, TID_INT, FALSE);

   if (status != DB_SUCCESS) {
      return 0;
   }

   return v;
}

struct EsperModuleData
{
   std::map<std::string,int> t; // type
   std::map<std::string,std::string> s; // string variables
   std::map<std::string,int> i; // integer variables
   std::map<std::string,bool> b; // boolean variables
   std::map<std::string,std::vector<int>> ia; // integer array variables
   std::map<std::string,std::vector<bool>> ba; // boolean array variables
};

typedef std::map<std::string,EsperModuleData> EsperNodeData;

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

   KOtcpError GetModules(std::vector<std::string>* mid)
   {
      std::vector<std::string> headers;
      std::vector<std::string> reply_headers;
      std::string reply_body;

      KOtcpError e = s->HttpGet(headers, "/read_node?includeMods=y", &reply_headers, &reply_body);

      if (e.error) {
         eq->SetStatus("http error", "red");
         mfe->Msg(MERROR, "GetModules", "HttpGet() error %s", e.message.c_str());
         return e;
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
                     mid->push_back(maek->GetString());
                  }
               }
            }
         }
      }

      delete jtree;

      return KOtcpError();
   }

   KOtcpError ReadVariables(const std::string& mid, EsperModuleData* vars)
   {
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
         return e;
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
                     vars->t[vid] = type;
                     if (fVerbose)
                        printf("mid [%s] vid [%s] type %d json value %s\n", mid.c_str(), vid.c_str(), type, vaed->Stringify().c_str());
                     if (type == 1 || type == 2 || type == 3 || type == 4 || type == 5 || type == 6) {
                        std::vector<int> val = JsonToIntArray(vaed);
                        if (val.size() == 1)
                           vars->i[vid] = val[0];
                        else
                           vars->ia[vid] = val;
                        WRI(mid.c_str(), vid.c_str(), val);
                     } else if (type == 11) {
                        std::string val = vaed->GetString();
                        vars->s[vid] = val;
                        WR(mid.c_str(), vid.c_str(), val.c_str());
                     } else if (type == 12) {
                        std::vector<bool> val = JsonToBoolArray(vaed);
                        if (val.size() == 1)
                           vars->b[vid] = val[0];
                        else
                           vars->ba[vid] = val;
                        WRB(mid.c_str(), vid.c_str(), val);
                     } else {
                        WR(mid.c_str(), vid.c_str(), vaed->Stringify().c_str());
                     }
                     //variables.push_back(vid);
                  }
               }
            }
         }
      }

      delete jtree;

      return KOtcpError();
   }

   bool Read(EsperNodeData* data)
   {
      s->fReadTimeout = 5*1000;
      s->fHttpKeepOpen = false;

      if (fVerbose)
         printf("Reading %s\n", fOdbName.c_str());

      std::vector<std::string> modules;

      KOtcpError e = GetModules(&modules);

      if (e.error)
         return false;

      for (unsigned i=0; i<modules.size(); i++) {
         if (modules[i] == "sp32wv")
            continue;
         if (modules[i] == "sp16wv")
            continue;
         e = ReadVariables(modules[i], &(*data)[modules[i]]);
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
      delete jtree;
#endif

      return true;
   }

   bool Write(const char* mid, const char* vid, const char* json)
   {
      s->fWriteTimeout = 5*1000;
      s->fHttpKeepOpen = false;

      std::string url;
      url += "/write_var?";
      url += "mid=";
      url += mid;
      url += "&";
      url += "vid=";
      url += vid;
      url += "&";
      url += "offset=";
      url += "0";

      //printf("URL: %s\n", url.c_str());

      std::vector<std::string> headers;
      std::vector<std::string> reply_headers;
      std::string reply_body;

      KOtcpError e = s->HttpPost(headers, url.c_str(), json, &reply_headers, &reply_body);

      if (e.error) {
         eq->SetStatus("http error", "red");
         mfe->Msg(MERROR, "Write", "HttpGet() error %s", e.message.c_str());
         return false;
      }

#if 0
      printf("reply headers:\n");
      for (unsigned i=0; i<reply_headers.size(); i++)
         printf("%d: %s\n", i, reply_headers[i].c_str());

      printf("json: %s\n", reply_body.c_str());
#endif

      return true;
   }

   std::map<std::string,bool> fLogOnce;

   bool LogOnce(const char*s)
   {
      bool v = fLogOnce[s];
      fLogOnce[s] = true;
      return !v;
   }

   void LogOk(const char*s)
   {
      fLogOnce[s] = false;
   }

   bool Check(EsperNodeData data)
   {
      int run_state = OdbGetInt(mfe, "/Runinfo/State");
      bool running = (run_state == 3);

      bool ok = true;

      int freq_esata = data["board"].i["freq_esata"];
      bool force_run = data["board"].b["force_run"];
      bool nim_ena = data["board"].b["nim_ena"];
      bool nim_inv = data["board"].b["nim_inv"];
      bool esata_ena = data["board"].b["esata_ena"];
      bool esata_inv = data["board"].b["esata_inv"];
      int trig_nim_cnt = data["board"].i["trig_nim_cnt"];
      int trig_esata_cnt = data["board"].i["trig_esata_cnt"];
      bool udp_enable = data["udp"].b["enable"];
      int  udp_tx_cnt = data["udp"].i["tx_cnt"];

      printf("freq_esata: %d, run %d, nim %d %d, esata %d %d, trig %d %d, udp %d, tx_cnt %d\n", freq_esata, force_run, nim_ena, nim_inv, esata_ena, esata_inv, trig_nim_cnt, trig_esata_cnt, udp_enable, udp_tx_cnt);

      if (!udp_enable) {
         if (LogOnce("udp.enable"))
            mfe->Msg(MERROR, "Check", "ALPHA16 %s: udp.enable is false", fOdbName.c_str());
         ok = false;
      } else {
         LogOk("udp.enable");
      }

      if (force_run != running) {
         if (LogOnce("board.force_run"))
            mfe->Msg(MERROR, "Check", "ALPHA16 %s: board.force_run is wrong", fOdbName.c_str());
         ok = false;
      } else {
         LogOk("board.force_run");
      }

      return ok;
   }

   bool Configure()
   {
      int run_state = OdbGetInt(mfe, "/Runinfo/State");
      bool running = (run_state == 3);

      int udp_port = OdbGetInt(mfe, "/Equipment/UDP/Settings/udp_port");

      printf("Configure %s: run_state %d, running %d, udp_port %d\n", fOdbName.c_str(), run_state, running, udp_port);

      bool ok = true;

      ok &= Write("board", "force_run", "false");
      ok &= Write("udp", "enable", "false");

      int udp_ip = 0;
      udp_ip |= (192<<24);
      udp_ip |= (168<<16);
      udp_ip |= (1<<8);
      udp_ip |= (1<<0);

      ok &= Write("udp", "dst_ip", toString(udp_ip).c_str());
      ok &= Write("udp", "dst_port", toString(udp_port).c_str());
      ok &= Write("udp", "enable", "true");

      if (running) {
         ok &= Write("board", "nim_ena", "true");
         ok &= Write("board", "sata_ena", "true");
      } else {
         ok &= Write("board", "nim_ena", "false");
         ok &= Write("board", "sata_ena", "false");
      }

      if (running) {
         ok &= Write("board", "force_run", "true");
      } else {
         ok &= Write("board", "force_run", "false");
      }

      return ok;
   }

   bool SoftTrigger()
   {
      printf("SoftTrigger!\n");
      bool ok = true;
      ok &= Write("board", "nim_inv", "true");
      printf("Here!\n");
      ok &= Write("board", "nim_inv", "false");
      printf("SoftTrigger done!\n");
      return ok;
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

   if (1) {
      int countOk = 0;
      int countBad = 0;
      for (unsigned i=0; i<a16ctrl.size(); i++) {
         bool ok = a16ctrl[i]->Configure();
         if (ok) {
            countOk += 1;
         }
         if (!ok)
            countBad += 1;
      }

      {
         char buf[256];
         if (countBad == 0) {
            sprintf(buf, "Configure: %d A16 Ok", countOk);
            eq->SetStatus(buf, "#00FF00");
         } else {
            sprintf(buf, "Configure: %d A16 Ok, %d bad", countOk, countBad);
            eq->SetStatus(buf, "yellow");
         }
      }
   }

   if (1) {
      for (unsigned i=0; i<a16ctrl.size(); i++) {
         bool ok = a16ctrl[i]->SoftTrigger();
      }
   }

   printf("init done!\n");

   while (!mfe->fShutdown) {

      int countOk = 0;
      int countBad = 0;
      for (unsigned i=0; i<a16ctrl.size(); i++) {
         EsperNodeData e;
         bool ok = a16ctrl[i]->Read(&e);
         if (ok) {
            ok = a16ctrl[i]->Check(e);
            if (ok)
               countOk += 1;
         }
         if (!ok)
            countBad += 1;
      }

      {
         char buf[256];
         if (countBad == 0) {
            sprintf(buf, "%d A16 Ok", countOk);
            eq->SetStatus(buf, "#00FF00");
         } else {
            sprintf(buf, "%d A16 Ok, %d bad", countOk, countBad);
            eq->SetStatus(buf, "yellow");
         }
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
