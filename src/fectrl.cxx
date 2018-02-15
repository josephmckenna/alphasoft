// fectrl.cxx
//
// MIDAS frontend to control the DAQ: GRIF-C trigger board, GRIF-16 ADC, PWB
//

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>

#include <vector>
#include <map>
#include <mutex>
#include <thread>

#include "tmfe.h"

#include "KOtcp.h"

#include "midas.h"
#include "mjson.h"

static TMVOdb* gEvbC = NULL;

static std::mutex gOdbLock;
#define LOCK_ODB() std::lock_guard<std::mutex> lock(gOdbLock)
//#define LOCK_ODB() TMFE_LOCK_MIDAS(mfe)

#define C(x) ((x).c_str())

static int iabs(int v)
{
   if (v>=0)
      return v;
   else
      return -v;
}

static std::string toString(int value)
{
   char buf[256];
   sprintf(buf, "%d", value);
   return buf;
}

std::vector<int> JsonToIntArray(const MJsonNode* n)
{
   std::vector<int> vi;
   const MJsonNodeVector *a = n->GetArray();
   if (a) {
      for (unsigned i=0; i<a->size(); i++) {
         const MJsonNode* ae = a->at(i);
         if (ae) {
            if (ae->GetType() == MJSON_NUMBER) {
               //printf("MJSON_NUMBER [%s] is %f is 0x%x\n", ae->GetString().c_str(), ae->GetDouble(), (unsigned)ae->GetDouble());
               vi.push_back((unsigned)ae->GetDouble());
            } else {
               vi.push_back(ae->GetInt());
            }
         }
      }
   }
   return vi;
}

std::vector<double> JsonToDoubleArray(const MJsonNode* n)
{
   std::vector<double> vd;
   const MJsonNodeVector *a = n->GetArray();
   if (a) {
      for (unsigned i=0; i<a->size(); i++) {
         const MJsonNode* ae = a->at(i);
         if (ae) {
            vd.push_back(ae->GetDouble());
         }
      }
   }
   return vd;
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

std::vector<std::string> JsonToStringArray(const MJsonNode* n)
{
   std::vector<std::string> vs;
   const MJsonNodeVector *a = n->GetArray();
   if (a) {
      for (unsigned i=0; i<a->size(); i++) {
         const MJsonNode* ae = a->at(i);
         if (ae) {
            vs.push_back(ae->GetString());
         }
      }
   }
   return vs;
}

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

void WR(TMFE*mfe, TMFeEquipment* eq, const char* mod, const char* mid, const char* vid, const char* v)
{
   if (mfe->fShutdown)
      return;
   
   std::string path;
   path += "/Equipment/";
   path += eq->fName;
   path += "/Readback/";
   path += mod;
   path += "/";
   path += mid;
   path += "/";
   path += vid;
   
   LOCK_ODB();
   
   //printf("Write ODB %s : %s\n", C(path), v);
   int status = db_set_value(mfe->fDB, 0, C(path), v, strlen(v)+1, 1, TID_STRING);
   if (status != DB_SUCCESS) {
      printf("WR: db_set_value status %d\n", status);
   }
}

void WRI(TMFE*mfe, TMFeEquipment* eq, const char* mod, const char* mid, const char* vid, const std::vector<int>& v)
{
   if (mfe->fShutdown)
      return;
   
   std::string path;
   path += "/Equipment/";
   path += eq->fName;
   path += "/Readback/";
   path += mod;
   path += "/";
   path += mid;
   path += "/";
   path += vid;
   
   LOCK_ODB();
   
   //printf("Write ODB %s : %s\n", C(path), v);
   int status = db_set_value(mfe->fDB, 0, C(path), &v[0], sizeof(int)*v.size(), v.size(), TID_INT);
   if (status != DB_SUCCESS) {
      printf("WR: db_set_value status %d\n", status);
   }
}

void WRD(TMFE*mfe, TMFeEquipment* eq, const char* mod, const char* mid, const char* vid, const std::vector<double>& v)
{
   if (mfe->fShutdown)
      return;
   
   std::string path;
   path += "/Equipment/";
   path += eq->fName;
   path += "/Readback/";
   path += mod;
   path += "/";
   path += mid;
   path += "/";
   path += vid;
   
   LOCK_ODB();
   
   //printf("Write ODB %s : %s\n", C(path), v);
   int status = db_set_value(mfe->fDB, 0, C(path), &v[0], sizeof(double)*v.size(), v.size(), TID_DOUBLE);
   if (status != DB_SUCCESS) {
      printf("WR: db_set_value status %d\n", status);
   }
}

void WRB(TMFE*mfe, TMFeEquipment* eq, const char* mod, const char* mid, const char* vid, const std::vector<bool>& v)
{
   if (mfe->fShutdown)
      return;
   
   std::string path;
   path += "/Equipment/";
   path += eq->fName;
   path += "/Readback/";
   path += mod;
   path += "/";
   path += mid;
   path += "/";
   path += vid;
   //printf("Write ODB %s : %s\n", C(path), v);
   
   BOOL *bb = new BOOL[v.size()];
   for (unsigned i=0; i<v.size(); i++) {
      bb[i] = v[i];
   }
   
   LOCK_ODB();
   
   int status = db_set_value(mfe->fDB, 0, C(path), bb, sizeof(BOOL)*v.size(), v.size(), TID_BOOL);
   if (status != DB_SUCCESS) {
      printf("WR: db_set_value status %d\n", status);
   }
   
   delete bb;
}

struct EsperModuleData
{
   std::map<std::string,int> t; // type
   std::map<std::string,std::string> s; // string variables
   std::map<std::string,std::vector<std::string>> sa; // string array variables
   std::map<std::string,int> i; // integer variables
   std::map<std::string,double> d; // double variables
   std::map<std::string,bool> b; // boolean variables
   std::map<std::string,std::vector<int>> ia; // integer array variables
   std::map<std::string,std::vector<double>> da; // double array variables
   std::map<std::string,std::vector<bool>> ba; // boolean array variables
};

typedef std::map<std::string,EsperModuleData> EsperNodeData;

class EsperComm
{
public:
   KOtcpConnection* s = NULL;
   bool fFailed = false;
   bool fVerbose = false;

public:
   double fLastHttpTime = 0;
   double fMaxHttpTime = 0;

public:
   KOtcpError GetModules(TMFE* mfe, std::vector<std::string>* mid)
   {
      std::vector<std::string> headers;
      std::vector<std::string> reply_headers;
      std::string reply_body;

      double t0 = mfe->GetTime();

      KOtcpError e = s->HttpGet(headers, "/read_node?includeMods=y", &reply_headers, &reply_body);

      double t1 = mfe->GetTime();
      fLastHttpTime = t1-t0;
      if (fLastHttpTime > fMaxHttpTime)
         fMaxHttpTime = fLastHttpTime;

      if (e.error) {
         mfe->Msg(MERROR, "GetModules", "GetModules() error: HttpGet(read_node) error %s", e.message.c_str());
         fFailed = true;
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

   KOtcpError ReadVariables(TMFE* mfe, TMFeEquipment* eq, const char* odbname, const std::string& mid, EsperModuleData* vars)
   {
      if (fFailed)
         return KOtcpError("ReadVariables", "failed flag");

      std::vector<std::string> headers;
      std::vector<std::string> reply_headers;
      std::string reply_body;

      std::string url;
      url += "/read_module?includeVars=y&mid=";
      url += mid.c_str();
      url += "&includeData=y";

      double t0 = mfe->GetTime();

      KOtcpError e = s->HttpGet(headers, url.c_str(), &reply_headers, &reply_body);

      double t1 = mfe->GetTime();
      fLastHttpTime = t1-t0;
      if (fLastHttpTime > fMaxHttpTime)
         fMaxHttpTime = fLastHttpTime;

      if (e.error) {
         mfe->Msg(MERROR, "ReadVariables", "ReadVariables() error: HttpGet(read_module %s) error %s", mid.c_str(), e.message.c_str());
         fFailed = true;
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
                     if (type == 0) {
                        WR(mfe, eq, odbname, mid.c_str(), vid.c_str(), vaed->Stringify().c_str());
                     } else if (type == 1 || type == 2 || type == 3 || type == 4 || type == 5 || type == 6) {
                        std::vector<int> val = JsonToIntArray(vaed);
                        if (val.size() == 1)
                           vars->i[vid] = val[0];
                        else
                           vars->ia[vid] = val;
                        WRI(mfe, eq, odbname, mid.c_str(), vid.c_str(), val);
                     } else if (type == 9) {
                        std::vector<double> val = JsonToDoubleArray(vaed);
                        if (val.size() == 1)
                           vars->d[vid] = val[0];
                        else
                           vars->da[vid] = val;
                        WRD(mfe, eq, odbname, mid.c_str(), vid.c_str(), val);
                     } else if (type == 11) {
                        std::string val = vaed->GetString();
                        vars->s[vid] = val;
                        WR(mfe, eq, odbname, mid.c_str(), vid.c_str(), val.c_str());
                     } else if (type == 12) {
                        std::vector<bool> val = JsonToBoolArray(vaed);
                        if (val.size() == 1)
                           vars->b[vid] = val[0];
                        else
                           vars->ba[vid] = val;
                        WRB(mfe, eq, odbname, mid.c_str(), vid.c_str(), val);
                     } else if (type == 13) {
                        WR(mfe, eq, odbname, mid.c_str(), vid.c_str(), vaed->Stringify().c_str());
                     } else {
                        printf("mid [%s] vid [%s] type %d json value %s\n", mid.c_str(), vid.c_str(), type, vaed->Stringify().c_str());
                        WR(mfe, eq, odbname, mid.c_str(), vid.c_str(), vaed->Stringify().c_str());
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

   bool Write(TMFE* mfe, const char* mid, const char* vid, const char* json, bool binaryn=false)
   {
      if (fFailed)
         return false;

      std::string url;
      url += "/write_var?";
      if (binaryn) {
         url += "binary=n";
         url += "&";
      }
      url += "mid=";
      url += mid;
      url += "&";
      url += "vid=";
      url += vid;
      //url += "&";
      //url += "offset=";
      //url += "0";

      //printf("URL: %s\n", url.c_str());

      std::vector<std::string> headers;
      std::vector<std::string> reply_headers;
      std::string reply_body;

      double t0 = mfe->GetTime();

      KOtcpError e = s->HttpPost(headers, url.c_str(), json, &reply_headers, &reply_body);

      double t1 = mfe->GetTime();
      fLastHttpTime = t1-t0;
      if (fLastHttpTime > fMaxHttpTime)
         fMaxHttpTime = fLastHttpTime;

      if (e.error) {
         mfe->Msg(MERROR, "Write", "Write() error: HttpPost(write_var %s.%s) error %s", mid, vid, e.message.c_str());
         fFailed = true;
         return false;
      }

      if (reply_body.find("error") != std::string::npos) {
         mfe->Msg(MERROR, "Write", "AJAX write %s.%s value \"%s\" error: %s", mid, vid, json, reply_body.c_str());
         return false;
      }

#if 0
      printf("reply headers for %s:\n", url.c_str());
      for (unsigned i=0; i<reply_headers.size(); i++)
         printf("%d: %s\n", i, reply_headers[i].c_str());

      printf("json: %s\n", reply_body.c_str());
#endif

      return true;
   }

   std::string Read(TMFE* mfe, const char* mid, const char* vid, std::string* last_errmsg = NULL)
   {
      if (fFailed)
         return "";

      std::string url;
      url += "/read_var?";
      url += "mid=";
      url += mid;
      url += "&";
      url += "vid=";
      url += vid;
      url += "&";
      url += "offset=";
      url += "0";
      url += "&";
      url += "len=";
      url += "&";
      url += "len=";
      url += "0";
      url += "&";
      url += "dataOnly=y";

      // "/read_var?vid=elf_build_str&mid=board&offset=0&len=0&dataOnly=y"

      //printf("URL: %s\n", url.c_str());

      std::vector<std::string> headers;
      std::vector<std::string> reply_headers;
      std::string reply_body;

      double t0 = mfe->GetTime();

      KOtcpError e = s->HttpGet(headers, url.c_str(), &reply_headers, &reply_body);

      double t1 = mfe->GetTime();
      fLastHttpTime = t1-t0;
      if (fLastHttpTime > fMaxHttpTime)
         fMaxHttpTime = fLastHttpTime;

      if (e.error) {
         if (!last_errmsg || e.message != *last_errmsg) {
            mfe->Msg(MERROR, "Read", "Read %s.%s HttpGet() error %s", mid, vid, e.message.c_str());
            if (last_errmsg) {
               *last_errmsg = e.message;
            }
         }
         fFailed = true;
         return "";
      }

#if 0
      printf("reply headers:\n");
      for (unsigned i=0; i<reply_headers.size(); i++)
         printf("%d: %s\n", i, reply_headers[i].c_str());

      printf("json: %s\n", reply_body.c_str());
#endif

      return reply_body;
   }
};

class Fault
{
public: // state
   TMFE* fMfe = NULL;
   std::string fModName;
   std::string fFaultName;
   bool fFailed = false;
   std::string fMessage;

public: // constructor
   void Setup(TMFE* mfe, const char* mod_name, const char* fault_name)
   {
      fMfe = mfe;
      fModName = mod_name;
      fFaultName = fault_name;
   }

public: //operations
   void Fail(const std::string& message)
   {
      assert(fMfe);
      if (!fFailed) {
         fMfe->Msg(MERROR, "Check", "%s: Fault: %s: %s", fModName.c_str(), fFaultName.c_str(), message.c_str());
         fFailed = true;
      }
   }

   void Ok()
   {
      assert(fMfe);
      if (fFailed) {
         fMfe->Msg(MINFO, "Fault::Ok", "%s: Fault ok now: %s", fModName.c_str(), fFaultName.c_str());
         fFailed = false;
      }
   }
};

#define ST_ABSENT 0 // empty slot
#define ST_GOOD   1 // state is good
#define ST_NO_ESPER  2 // no esper object, an empty slot
#define ST_BAD_IDENTIFY 3 // probe and identify failure
#define ST_BAD_READ     4 // read failure
#define ST_BAD_CHECK    5 // check failure

class AdcCtrl
{
public: // settings and configuration
   TMFE* fMfe = NULL;
   TMFeEquipment* fEq = NULL;
   EsperComm* fEsper = NULL;

   std::string fOdbName;
   int fOdbIndex = -1;

   int fModule = 0; // alpha16 rev1 module number 1..20

   bool fVerbose = false;

   int fState = ST_ABSENT;

   int fPollSleep = 10;
   int fFailedSleep = 10;

public: // state and global variables
   std::mutex fLock;

   int fNumBanks = 0;

   Fault fCheckComm;
   Fault fCheckId;
   Fault fCheckPage;
   Fault fCheckEsata0;
   Fault fCheckEsataLock;
   Fault fCheckPllLock;
   Fault fCheckUdpState;
   Fault fCheckRunState;

public:
   AdcCtrl(TMFE* xmfe, TMFeEquipment* xeq, const char* xodbname, int xodbindex)
   {
      fMfe = xmfe;
      fEq = xeq;
      fOdbName = xodbname;
      fOdbIndex = xodbindex;

      fCheckComm.Setup(fMfe, fOdbName.c_str(), "communication");
      fCheckId.Setup(fMfe, fOdbName.c_str(), "identification");
      fCheckPage.Setup(fMfe, fOdbName.c_str(), "epcq boot page");
      fCheckEsata0.Setup(fMfe, fOdbName.c_str(), "no ESATA clock");
      fCheckEsataLock.Setup(fMfe, fOdbName.c_str(), "ESATA clock lock");
      fCheckPllLock.Setup(fMfe, fOdbName.c_str(), "PLL lock");
      fCheckUdpState.Setup(fMfe, fOdbName.c_str(), "UDP state");
      fCheckRunState.Setup(fMfe, fOdbName.c_str(), "run state");
   }

   bool ReadAdcLocked(EsperNodeData* data)
   {
      if (fVerbose)
         printf("Reading %s\n", fOdbName.c_str());

      if (!fEsper || fEsper->fFailed)
         return false;

      std::vector<std::string> modules;

      KOtcpError e = fEsper->GetModules(fMfe, &modules);

      if (e.error) {
         return false;
      }

      for (unsigned i=0; i<modules.size(); i++) {
         if (modules[i] == "sp32wv")
            continue;
         if (modules[i] == "sp16wv")
            continue;
         if (modules[i] == "fmc32wv")
            continue;
         if (modules[i] == "adc16wv")
            continue;
         e = fEsper->ReadVariables(fMfe, fEq, fOdbName.c_str(), modules[i], &(*data)[modules[i]]);
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
         fMfe->Msg(MERROR, "Read", "HttpGet() error %s", e.message.c_str());
         fFailed = true;
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

   int fUpdateCount = 0;

   bool fLmkFirstTime = true;
   int fLmkPll1lcnt = 0;
   int fLmkPll2lcnt = 0;

   double fFpgaTemp = 0;
   double fSensorTemp0 = 0;
   double fSensorTempMax = 0;
   double fSensorTempMin = 0;

   bool fCheckOk = true;
   bool fUnusable = false;

   bool fEnableAdcTrigger = true;

   bool CheckAdcLocked(EsperNodeData data)
   {
      assert(fEsper);

      if (fEsper->fFailed) {
         //printf("%s: failed\n", fOdbName.c_str());
         fCheckComm.Fail("see previous messages");
         fUnusable = true;
         return false;
      }

      if (fUnusable) {
         return false;
      }

      int run_state = 0;
      int transition_in_progress = 0;
      fMfe->fOdbRoot->RI("Runinfo/State", 0, &run_state, false);
      fMfe->fOdbRoot->RI("Runinfo/Transition in progress", 0, &transition_in_progress, false);

      bool running = (run_state == 3);

      if (!fEnableAdcTrigger)
         running = false;

      //printf("%s: run_state %d, running %d, transition_in_progress %d\n", fOdbName.c_str(), run_state, running, transition_in_progress);

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
      double fpga_temp = data["board"].d["fpga_temp"];
      int clk_lmk = data["board"].i["clk_lmk"];
      int lmk_pll1_lcnt = data["board"].i["lmk_pll1_lcnt"];
      int lmk_pll2_lcnt = data["board"].i["lmk_pll2_lcnt"];
      int lmk_pll1_lock = data["board"].b["lmk_pll1_lock"];
      int lmk_pll2_lock = data["board"].b["lmk_pll2_lock"];
      int page_select = data["update"].i["page_select"];

      printf("%s: fpga temp: %.0f, freq_esata: %d, clk_lmk %d lock %d %d lcnt %d %d, run %d, nim %d %d, esata %d %d, trig %d %d, udp %d, tx_cnt %d, page_select %d\n",
             fOdbName.c_str(),
             fpga_temp,
             freq_esata,
             clk_lmk,
             lmk_pll1_lock, lmk_pll2_lock,
             lmk_pll1_lcnt, lmk_pll2_lcnt,
             force_run,
             nim_ena, nim_inv,
             esata_ena, esata_inv,
             trig_nim_cnt, trig_esata_cnt,
             udp_enable, udp_tx_cnt,
             page_select);

      if (freq_esata == 0) {
         fCheckEsata0.Fail("board.freq_esata is zero: " + toString(freq_esata));
         ok = false;
      } else {
         fCheckEsata0.Ok();
      }

      if (iabs(freq_esata - 62500000) > 1) {
         fCheckEsataLock.Fail("board.freq_esata is bad: " + toString(freq_esata));
         ok = false;
      } else {
         fCheckEsataLock.Ok();
      }

      if (!lmk_pll1_lock || !lmk_pll2_lock) {
         fCheckPllLock.Fail("lmk_pll1_lock or lmk_pll2_lock is bad");
         ok = false;
      } else {
         fCheckPllLock.Ok();
      }

      if (lmk_pll1_lcnt != fLmkPll1lcnt) {
         if (!fLmkFirstTime) {
            fMfe->Msg(MERROR, "Check", "%s: LMK PLL1 lock count changed %d to %d", fOdbName.c_str(), fLmkPll1lcnt, lmk_pll1_lcnt);
         }
         fLmkPll1lcnt = lmk_pll1_lcnt;
      }

      if (lmk_pll2_lcnt != fLmkPll2lcnt) {
         if (!fLmkFirstTime) {
            fMfe->Msg(MERROR, "Check", "%s: LMK PLL2 lock count changed %d to %d", fOdbName.c_str(), fLmkPll2lcnt, lmk_pll2_lcnt);
         }
         fLmkPll2lcnt = lmk_pll2_lcnt;
      }

      fLmkFirstTime = false;

      if (!udp_enable) {
         fCheckUdpState.Fail("udp.enable is bad: " + toString(udp_enable));
         ok = false;
      } else {
         fCheckUdpState.Ok();
      }

      if (!transition_in_progress) {
         if (force_run != running) {
            fCheckRunState.Fail("board.force_run is bad: " + toString(force_run));
            ok = false;
         } else {
            fCheckRunState.Ok();
         }
      }

      fFpgaTemp = fpga_temp;
      fSensorTemp0 = data["board"].da["sensor_temp"][0];
      fSensorTempMax = data["board"].da["sensor_temp"][0];
      fSensorTempMin = data["board"].da["sensor_temp"][0];

      for (unsigned i=1; i<data["board"].da["sensor_temp"].size(); i++) {
         double t = data["board"].da["sensor_temp"][i];
         if (t > fSensorTempMax)
            fSensorTempMax = t;
         if (t < fSensorTempMin)
            fSensorTempMin = t;
      }

      fUpdateCount++;

      fCheckOk = ok;

      return ok;
   }

   int xatoi(const char* s)
   {
      if (s == NULL)
         return 0;
      else if (s[0]=='[')
         return atoi(s+1);
      else
         return atoi(s);
   }

   std::string fLastErrmsg;

   bool fRebootingToUserPage = false;

   void ReportAdcUpdateLocked()
   {
      if (!fEsper)
         return;

      std::string wdtimer_src = fEsper->Read(fMfe, "update", "wdtimer_src");
      std::string nconfig_src = fEsper->Read(fMfe, "update", "nconfig_src");
      std::string runconfig_src = fEsper->Read(fMfe, "update", "runconfig_src");
      std::string nstatus_src = fEsper->Read(fMfe, "update", "nstatus_src");
      std::string crcerror_src = fEsper->Read(fMfe, "update", "crcerror_src");
      std::string watchdog_ena = fEsper->Read(fMfe, "update", "watchdog_ena");
      std::string application = fEsper->Read(fMfe, "update", "application");
      std::string page_select = fEsper->Read(fMfe, "update", "page_select");
      std::string sel_page = fEsper->Read(fMfe, "update", "sel_page");

      fMfe->Msg(MINFO, "Identify", "%s: remote update status: wdtimer: %s, nconfig: %s, runconfig: %s, nstatus: %s, crcerror: %s, watchdog_ena: %s, application: %s, page_select: %s, sel_page: %s", fOdbName.c_str(),
                wdtimer_src.c_str(),
                nconfig_src.c_str(),
                runconfig_src.c_str(),
                nstatus_src.c_str(),
                crcerror_src.c_str(),
                watchdog_ena.c_str(),
                application.c_str(),
                page_select.c_str(),
                sel_page.c_str()
                );
   }

   bool IdentifyAdcLocked()
   {
      if (!fEsper)
         return false;

      if (fEsper->fFailed) {
         fEsper->fFailed = false;
      } else {
         fLastErrmsg = "";
      }

      std::string elf_buildtime = fEsper->Read(fMfe, "board", "elf_buildtime", &fLastErrmsg);

      if (!elf_buildtime.length() > 0) {
         fCheckId.Fail("cannot read board.elf_buildtime");
         return false;
      }

      std::string sw_qsys_ts = fEsper->Read(fMfe, "board", "sw_qsys_ts", &fLastErrmsg);

      if (!sw_qsys_ts.length() > 0) {
         fCheckId.Fail("cannot read board.sw_qsys_ts");
         return false;
      }

      std::string hw_qsys_ts = fEsper->Read(fMfe, "board", "hw_qsys_ts", &fLastErrmsg);

      if (!hw_qsys_ts.length() > 0) {
         fCheckId.Fail("cannot read board.hw_qsys_ts");
         return false;
      }

      std::string fpga_build = fEsper->Read(fMfe, "board", "fpga_build", &fLastErrmsg);

      if (!fpga_build.length() > 0) {
         fCheckId.Fail("cannot read board.fpga_build");
         return false;
      }

      std::string page_select_str = fEsper->Read(fMfe, "update", "page_select", &fLastErrmsg);

      if (!page_select_str.length() > 0) {
         fCheckId.Fail("cannot read update.page_select");
         return false;
      }

      uint32_t elf_ts = xatoi(elf_buildtime.c_str());
      uint32_t qsys_sw_ts = xatoi(sw_qsys_ts.c_str());
      uint32_t qsys_hw_ts = xatoi(hw_qsys_ts.c_str());
      uint32_t sof_ts = xatoi(fpga_build.c_str());
      uint32_t page_select = xatoi(page_select_str.c_str());

      fMfe->Msg(MINFO, "Identify", "%s: firmware: elf 0x%08x, qsys_sw 0x%08x, qsys_hw 0x%08x, sof 0x%08x, epcq page %d", fOdbName.c_str(), elf_ts, qsys_sw_ts, qsys_hw_ts, sof_ts, page_select);

      ReportAdcUpdateLocked();

      bool user_page = false;

      if (page_select == 0) {
         // factory page
         user_page = false;
      } else if (page_select == 16777216) {
         // user page
         user_page = true;
      } else {
         fMfe->Msg(MERROR, "Identify", "%s: unexpected value of update.page_select: %s", fOdbName.c_str(), page_select_str.c_str());
         fCheckId.Fail("incompatible firmware, update.page_select: " + page_select_str);
         return false;
      }

      bool boot_load_only = false;

      if (elf_ts == 0x59555815) {
         boot_load_only = true;
      } else if (elf_ts == 0x59baf6f8) {
         boot_load_only = true;
      } else if (elf_ts == 0x59e552ef) {
         boot_load_only = true;
      } else if (elf_ts == 0x59eea9d4) { // added module_id
      } else {
         fMfe->Msg(MERROR, "Identify", "%s: firmware is not compatible with the daq, elf_buildtime 0x%08x", fOdbName.c_str(), elf_ts);
         fCheckId.Fail("incompatible firmware, elf_buildtime: " + elf_buildtime);
         return false;
      }

      if (qsys_sw_ts != qsys_hw_ts) {
         boot_load_only = true;
         fMfe->Msg(MERROR, "Identify", "%s: firmware is not compatible with the daq, qsys mismatch, sw 0x%08x vs hw 0x%08x", fOdbName.c_str(), qsys_sw_ts, qsys_hw_ts);
         fCheckId.Fail("incompatible firmware, qsys timestamp mismatch, sw: " + sw_qsys_ts + ", hw: " + hw_qsys_ts);
         //return false;
      }

      if (sof_ts == 0x594b603a) {
         boot_load_only = true;
      } else if (sof_ts == 0x59d96d5a) {
         boot_load_only = true;
      } else if (sof_ts == 0x59e691dc) {
         boot_load_only = true;
      } else if (sof_ts == 0x59e7d5f2) {
         boot_load_only = true;
      } else if (sof_ts == 0x59eeae46) { // added module_id and adc16 discriminators
      } else if (sof_ts == 0x5a839e66) { // added trigger thresholds via module_id upper 4 bits, added adc32 discriminators
      } else {
         fMfe->Msg(MERROR, "Identify", "%s: firmware is not compatible with the daq, sof fpga_build  0x%08x", fOdbName.c_str(), sof_ts);
         fCheckId.Fail("incompatible firmware, fpga_build: " + fpga_build);
         return false;
      }

      bool boot_from_user_page = false;

      fEq->fOdbEqSettings->RB("ADC/boot_user_page", fOdbIndex, &boot_from_user_page, false);

      if (boot_from_user_page != user_page) {
         if (boot_from_user_page) {
            if (fRebootingToUserPage) {
               fMfe->Msg(MERROR, "Identify", "%s: failed to boot the epcq user page", fOdbName.c_str());
               fRebootingToUserPage = true;
               ReportAdcUpdateLocked();
               return false;
            }

            fMfe->Msg(MERROR, "Identify", "%s: rebooting to the epcq user page", fOdbName.c_str());
            fEsper->Write(fMfe, "update", "sel_page", "0x01000000");
            fEsper->Write(fMfe, "update", "reconfigure", "y", true);
            fRebootingToUserPage = true;
            return false;
         }
      }

      fRebootingToUserPage = false;

      if (boot_load_only) {
         fMfe->Msg(MERROR, "Identify", "%s: firmware is not compatible with the daq, usable as boot loader only", fOdbName.c_str());
         fCheckId.Fail("incompatible firmware, usable as boot loader only");
         return false;
      }

      const char* s = fOdbName.c_str();
      while (*s && !isdigit(*s)) {
         s++;
      }
      fModule = atoi(s);

      fCheckId.Ok();

      return true;
   }

   bool fConfAdc16Enable = true;
   bool fConfAdc32Enable = false;

   int fNumBanksAdc16 = 0;
   int fNumBanksAdc32 = 0;

   bool ConfigureAdcLocked()
   {
      assert(fEsper);

      if (fEsper->fFailed) {
         printf("Configure %s: failed flag\n", fOdbName.c_str());
         return false;
      }

      int udp_port = 0;

      fMfe->fOdbRoot->RI("Equipment/UDP/Settings/udp_port", 0, &udp_port, false);

      int adc16_samples = 700;
      int adc16_trig_delay = 0;
      int adc16_trig_start = 200;

      fEq->fOdbEqSettings->RI("ADC/adc16_samples",    0, &adc16_samples, true);
      fEq->fOdbEqSettings->RI("ADC/adc16_trig_delay", 0, &adc16_trig_delay, true);
      fEq->fOdbEqSettings->RI("ADC/adc16_trig_start", 0, &adc16_trig_start, true);
      fEq->fOdbEqSettings->RB("ADC/adc16_enable",     fOdbIndex, &fConfAdc16Enable, false);

      int adc32_samples = 511;
      int adc32_trig_delay = 0;
      int adc32_trig_start = 175;

      fEq->fOdbEqSettings->RI("ADC/adc32_samples",    0, &adc32_samples, true);
      fEq->fOdbEqSettings->RI("ADC/adc32_trig_delay", 0, &adc32_trig_delay, true);
      fEq->fOdbEqSettings->RI("ADC/adc32_trig_start", 0, &adc32_trig_start, true);
      fEq->fOdbEqSettings->RB("ADC/adc32_enable",     fOdbIndex, &fConfAdc32Enable, false);

      int thr = 1;
      fEq->fOdbEqSettings->RI("ADC/trig_threshold", 0, &thr, true);

      uint32_t module_id = 0;
      module_id |= ((fOdbIndex)&0x0F);
      module_id |= ((thr<<4)&0xF0);

      fMfe->Msg(MINFO, "ADC::Configure", "%s: configure: udp_port %d, adc16 enable %d, samples %d, trig_delay %d, trig_start %d, adc32 enable %d, samples %d, trig_delay %d, trig_start %d, module_id 0x%02x", fOdbName.c_str(), udp_port, fConfAdc16Enable, adc16_samples, adc16_trig_delay, adc16_trig_start, fConfAdc32Enable, adc32_samples, adc32_trig_delay, adc32_trig_start, module_id);

      bool ok = true;

      ok &= StopAdcLocked();

      // make sure everything is stopped

      ok &= fEsper->Write(fMfe, "board", "force_run", "false");
      ok &= fEsper->Write(fMfe, "udp", "enable", "false");

      // switch clock to ESATA

      ok &= fEsper->Write(fMfe, "board", "clk_lmk", "1");

      // configure the ADCs

      fNumBanks = 0;

      if (fConfAdc16Enable) {
         fNumBanksAdc16 = 16;
         fNumBanks += 16;
      }

      {
         std::string json;
         json += "[";
         for (int i=0; i<16; i++) {
            if (fConfAdc16Enable) {
               json += "true";
            } else {
               json += "false";
            }
            json += ",";
         }
         json += "]";
         
         ok &= fEsper->Write(fMfe, "adc16", "enable", json.c_str());
      }

      {
         std::string json;
         json += "[";
         for (int i=0; i<16; i++) {
            json += toString(adc16_trig_delay);
            json += ",";
         }
         json += "]";
         
         ok &= fEsper->Write(fMfe, "adc16", "trig_delay", json.c_str());
      }

      {
         std::string json;
         json += "[";
         for (int i=0; i<16; i++) {
            json += toString(adc16_trig_start);
            json += ",";
         }
         json += "]";
         
         ok &= fEsper->Write(fMfe, "adc16", "trig_start", json.c_str());
      }

      {
         std::string json;
         json += "[";
         for (int i=0; i<16; i++) {
            json += toString(adc16_samples);
            json += ",";
         }
         json += "]";
         
         ok &= fEsper->Write(fMfe, "adc16", "trig_stop", json.c_str());
      }

      if (fConfAdc32Enable) {
         fNumBanksAdc32 = 32;
         fNumBanks += 32;
      }

      {
         std::string json;
         json += "[";
         for (int i=0; i<32; i++) {
            if (fConfAdc32Enable) {
               json += "true";
            } else {
               json += "false";
            }
            json += ",";
         }
         json += "]";
         
         ok &= fEsper->Write(fMfe, "fmc32", "enable", json.c_str());
      }

      if (fConfAdc32Enable) {
         std::string json;
         json += "[";
         for (int i=0; i<32; i++) {
            json += toString(adc32_trig_delay);
            json += ",";
         }
         json += "]";
         
         ok &= fEsper->Write(fMfe, "fmc32", "trig_delay", json.c_str());
      }

      if (fConfAdc32Enable) {
         std::string json;
         json += "[";
         for (int i=0; i<32; i++) {
            json += toString(adc32_trig_start);
            json += ",";
         }
         json += "]";
         
         ok &= fEsper->Write(fMfe, "fmc32", "trig_start", json.c_str());
      }

      if (fConfAdc32Enable) {
         std::string json;
         json += "[";
         for (int i=0; i<32; i++) {
            json += toString(adc32_samples);
            json += ",";
         }
         json += "]";
         
         ok &= fEsper->Write(fMfe, "fmc32", "trig_stop", json.c_str());
      }

      // program module id and trigger threshold

      ok &= fEsper->Write(fMfe, "board", "module_id", toString(module_id).c_str());

      // program the IP address and port number in the UDP transmitter

      int udp_ip = 0;
      udp_ip |= (192<<24);
      udp_ip |= (168<<16);
      udp_ip |= (1<<8);
      udp_ip |= (1<<0);

      ok &= fEsper->Write(fMfe, "udp", "dst_ip", toString(udp_ip).c_str());
      ok &= fEsper->Write(fMfe, "udp", "dst_port", toString(udp_port).c_str());
      ok &= fEsper->Write(fMfe, "udp", "enable", "true");

      return ok;
   }

   bool StartAdcLocked()
   {
      assert(fEsper);
      bool ok = true;
      ok &= fEsper->Write(fMfe, "board", "nim_ena", "true");
      ok &= fEsper->Write(fMfe, "board", "esata_ena", "true");
      ok &= fEsper->Write(fMfe, "board", "force_run", "true");
      return ok;
   }

   bool StopAdcLocked()
   {
      assert(fEsper);
      bool ok = true;
      ok &= fEsper->Write(fMfe, "board", "force_run", "false");
      ok &= fEsper->Write(fMfe, "board", "nim_ena", "false");
      ok &= fEsper->Write(fMfe, "board", "esata_ena", "false");
      return ok;
   }

   bool SoftTriggerAdcLocked()
   {
      assert(fEsper);
      //printf("SoftTrigger!\n");
      bool ok = true;
      ok &= fEsper->Write(fMfe, "board", "nim_inv", "true");
      ok &= fEsper->Write(fMfe, "board", "nim_inv", "false");
      //printf("SoftTrigger done!\n");
      return ok;
   }

   void ThreadAdc()
   {
      printf("thread for %s started\n", fOdbName.c_str());
      assert(fEsper);
      while (!fMfe->fShutdown) {
         if (fEsper->fFailed) {
            bool ok;
            {
               std::lock_guard<std::mutex> lock(fLock);
               ok = IdentifyAdcLocked();
               // fLock implicit unlock
            }
            if (!ok) {
               fState = ST_BAD_IDENTIFY;
               for (int i=0; i<fFailedSleep; i++) {
                  if (fMfe->fShutdown)
                     break;
                  sleep(1);
               }
               continue;
            }
         }

         {
            std::lock_guard<std::mutex> lock(fLock);
            ReadAndCheckAdcLocked();
         }

         for (int i=0; i<fPollSleep; i++) {
            if (fMfe->fShutdown)
               break;
            sleep(1);
         }
      }
      printf("thread for %s shutdown\n", fOdbName.c_str());
   }

   std::thread* fThread = NULL;

   void StartThreadsAdc()
   {
      assert(fThread == NULL);
      fThread = new std::thread(&AdcCtrl::ThreadAdc, this);
   }

   void JoinThreadsAdc()
   {
      if (fThread) {
         fThread->join();
         delete fThread;
         fThread = NULL;
      }
   }

   void ReadAndCheckAdcLocked()
   {
      if (!fEsper) {
         fState = ST_NO_ESPER;
         return;
      }

      EsperNodeData e;

      bool ok = ReadAdcLocked(&e);
      if (!ok) {
         fState = ST_BAD_READ;
         return;
      }

      ok = CheckAdcLocked(e);
      if (!ok) {
         fState = ST_BAD_CHECK;
         return;
      }

      fState = ST_GOOD;
   }

   void BeginRunAdcLocked(bool start, bool enableAdcTrigger)
   {
      if (!fEsper)
         return;
      fEnableAdcTrigger = enableAdcTrigger;
      bool ok = IdentifyAdcLocked();
      if (!ok) {
         fState = ST_BAD_IDENTIFY;
         return;
      }
      ConfigureAdcLocked();
      ReadAndCheckAdcLocked();
      //WriteVariables();
      if (start && enableAdcTrigger) {
         StartAdcLocked();
      }
   }
};

class PwbCtrl
{
public:
   TMFE* fMfe = NULL;
   TMFeEquipment* fEq = NULL;
   EsperComm* fEsper = NULL;

   std::string fOdbName;
   int fOdbIndex = -1;

   int fModule = -1;

   bool fVerbose = false;

   int fState = ST_ABSENT;

   int fPollSleep = 10;
   int fFailedSleep = 10;

   std::mutex fLock;

   bool fEnablePwbTrigger = true;

   int fNumBanks = 0;

   Fault fCheckComm;
   Fault fCheckId;
   Fault fCheckPage;
   //Fault fCheckEsata0;
   //Fault fCheckEsataLock;
   Fault fCheckClockSelect;
   Fault fCheckPllLock;
   Fault fCheckUdpState;
   Fault fCheckRunState;

public:
   PwbCtrl(TMFE* xmfe, TMFeEquipment* xeq, const char* xodbname, int xodbindex)
   {
      fMfe = xmfe;
      fEq = xeq;
      fOdbName = xodbname;
      fOdbIndex = xodbindex;

      fCheckComm.Setup(fMfe, fOdbName.c_str(), "communication");
      fCheckId.Setup(fMfe, fOdbName.c_str(), "identification");
      fCheckPage.Setup(fMfe, fOdbName.c_str(), "epcq boot page");
      //fCheckEsata0.Setup(fMfe, fOdbName.c_str(), "no ESATA clock");
      //fCheckEsataLock.Setup(fMfe, fOdbName.c_str(), "ESATA clock lock");
      fCheckClockSelect.Setup(fMfe, fOdbName.c_str(), "clock select");
      fCheckPllLock.Setup(fMfe, fOdbName.c_str(), "PLL lock");
      fCheckUdpState.Setup(fMfe, fOdbName.c_str(), "UDP state");
      fCheckRunState.Setup(fMfe, fOdbName.c_str(), "run state");
   }
         
   bool ReadPwbLocked(EsperNodeData* data)
   {
      assert(fEsper);

      if (fVerbose)
         printf("Reading %s\n", fOdbName.c_str());

      if (fEsper->fFailed)
         return false;

      std::vector<std::string> modules;

      KOtcpError e = fEsper->GetModules(fMfe, &modules);

      if (e.error) {
         return false;
      }

      for (unsigned i=0; i<modules.size(); i++) {
         //if (modules[i] == "signalproc")
         //   continue;
         e = fEsper->ReadVariables(fMfe, fEq, fOdbName.c_str(), modules[i], &(*data)[modules[i]]);
      }

      return true;
   }

   int fUpdateCount = 0;

   double fTempFpga = 0;
   double fTempBoard = 0;
   double fTempScaA = 0;
   double fTempScaB = 0;
   double fTempScaC = 0;
   double fTempScaD = 0;

   double fVoltSca12 = 0;
   double fVoltSca34 = 0;
   double fVoltP2 = 0;
   double fVoltP5 = 0;

   double fCurrSca12 = 0;
   double fCurrSca34 = 0;
   double fCurrP2 = 0;
   double fCurrP5 = 0;

   bool CheckPwbLocked(EsperNodeData data)
   {
      assert(fEsper);

      if (fEsper->fFailed) {
         printf("%s: failed\n", fOdbName.c_str());
         return false;
      }

      int run_state = 0;
      int transition_in_progress = 0;
      fMfe->fOdbRoot->RI("Runinfo/State", 0, &run_state, false);
      fMfe->fOdbRoot->RI("Runinfo/Transition in progress", 0, &transition_in_progress, false);

      bool running = (run_state == 3);

      if (!fEnablePwbTrigger)
         running = false;

      //printf("%s: run_state %d, running %d, transition_in_progress %d\n", fOdbName.c_str(), run_state, running, transition_in_progress);

      bool ok = true;

      bool plls_locked = data["clockcleaner"].b["plls_locked"];
      bool sfp_sel = data["clockcleaner"].b["sfp_sel"];
      bool osc_sel = data["clockcleaner"].b["osc_sel"];
      int freq_sfp  = data["board"].i["freq_sfp"];
      //int freq_sata = data["board"].i["freq_sata"];
      bool force_run = data["signalproc"].b["force_run"];

      fTempFpga = data["board"].d["temp_fpga"];
      fTempBoard = data["board"].d["temp_board"];
      fTempScaA = data["board"].d["temp_sca_a"];
      fTempScaB = data["board"].d["temp_sca_b"];
      fTempScaC = data["board"].d["temp_sca_c"];
      fTempScaD = data["board"].d["temp_sca_d"];

      fVoltSca12 = data["board"].d["v_sca12"];
      fVoltSca34 = data["board"].d["v_sca34"];
      fVoltP2 = data["board"].d["v_p2"];
      fVoltP5 = data["board"].d["v_p5"];

      fCurrSca12 = data["board"].d["i_sca12"];
      fCurrSca34 = data["board"].d["i_sca34"];
      fCurrP2 = data["board"].d["i_p2"];
      fCurrP5 = data["board"].d["i_p5"];

      printf("%s: fpga temp: %.1f %.1f %.1f %.1f %1.f %.1f, freq_sfp: %d, pll locked %d, sfp_sel %d, osc_sel %d, run %d\n",
             fOdbName.c_str(),
             fTempFpga,
             fTempBoard,
             fTempScaA,
             fTempScaB,
             fTempScaC,
             fTempScaD,
             freq_sfp,
             plls_locked,
             sfp_sel,
             osc_sel,
             force_run);

      if (!plls_locked) {
         fCheckPllLock.Fail("plls_locked is bad: " + toString(plls_locked));
         ok = false;
      } else {
         fCheckPllLock.Ok();
      }

      if (sfp_sel || osc_sel) {
         fCheckClockSelect.Fail("wrong clock selected, sfp_sel: " + toString(sfp_sel) + ", osc_sel: " + toString(osc_sel));
         ok = false;
      } else {
         fCheckClockSelect.Ok();
      }

      if (!transition_in_progress) {
         if (force_run != running) {
            fCheckRunState.Fail("signalproc.force_run is bad: " + toString(force_run));
            ok = false;
         } else {
            fCheckRunState.Ok();
         }
      }

#if 0
      bool nim_ena = data["board"].b["nim_ena"];
      bool nim_inv = data["board"].b["nim_inv"];
      bool esata_ena = data["board"].b["esata_ena"];
      bool esata_inv = data["board"].b["esata_inv"];
      int trig_nim_cnt = data["board"].i["trig_nim_cnt"];
      int trig_esata_cnt = data["board"].i["trig_esata_cnt"];
      bool udp_enable = data["udp"].b["enable"];
      int  udp_tx_cnt = data["udp"].i["tx_cnt"];
      double fpga_temp = data["board"].d["fpga_temp"];
      int clk_lmk = data["board"].i["clk_lmk"];

      printf("%s: fpga temp: %.0f, freq_esata: %d, clk_lmk %d lock %d %d lcnt %d %d, run %d, nim %d %d, esata %d %d, trig %d %d, udp %d, tx_cnt %d\n",
             fOdbName.c_str(),
             fpga_temp,
             freq_esata,
             clk_lmk,
             lmk_pll1_lock, lmk_pll2_lock,
             lmk_pll1_lcnt, lmk_pll2_lcnt,
             force_run, nim_ena, nim_inv, esata_ena, esata_inv, trig_nim_cnt, trig_esata_cnt, udp_enable, udp_tx_cnt);
#endif

      fUpdateCount++;

      return ok;
   }

   int xatoi(const char* s)
   {
      if (s == NULL)
         return 0;
      else if (s[0]=='[')
         return atoi(s+1);
      else
         return atoi(s);
   }

   std::string fLastErrmsg;

   bool IdentifyPwbLocked()
   {
      assert(fEsper);

      if (fEsper->fFailed) {
         fEsper->fFailed = false;
      } else {
         fLastErrmsg = "";
      }

      std::string elf_buildtime = fEsper->Read(fMfe, "board", "elf_buildtime", &fLastErrmsg);

      if (!elf_buildtime.length() > 0) {
         fCheckId.Fail("cannot read board.elf_buildtime");
         return false;
      }

      std::string sw_qsys_ts = fEsper->Read(fMfe, "board", "sw_qsys_ts", &fLastErrmsg);

      if (!sw_qsys_ts.length() > 0) {
         fCheckId.Fail("cannot read board.sw_qsys_ts");
         return false;
      }

      std::string hw_qsys_ts = fEsper->Read(fMfe, "board", "hw_qsys_ts", &fLastErrmsg);

      if (!hw_qsys_ts.length() > 0) {
         fCheckId.Fail("cannot read board.hw_qsys_ts");
         return false;
      }

      std::string fpga_build = fEsper->Read(fMfe, "board", "fpga_build", &fLastErrmsg);

      if (!fpga_build.length() > 0) {
         fCheckId.Fail("cannot read board.fpga_build");
         return false;
      }

      std::string page_select_str = fEsper->Read(fMfe, "update", "page_select", &fLastErrmsg);

      if (!page_select_str.length() > 0) {
         fCheckId.Fail("cannot read update.page_select");
         return false;
      }

      uint32_t elf_ts = xatoi(elf_buildtime.c_str());
      uint32_t qsys_sw_ts = xatoi(sw_qsys_ts.c_str());
      uint32_t qsys_hw_ts = xatoi(hw_qsys_ts.c_str());
      uint32_t sof_ts = xatoi(fpga_build.c_str());
      uint32_t page_select = xatoi(page_select_str.c_str());

      fMfe->Msg(MINFO, "Identify", "%s: firmware: elf 0x%08x, qsys_sw 0x%08x, qsys_hw 0x%08x, sof 0x%08x, epcq page %d", fOdbName.c_str(), elf_ts, qsys_sw_ts, qsys_hw_ts, sof_ts, page_select);

      bool user_page = false;

      if (page_select == 0) {
         // factory page
         user_page = false;
      } else if (page_select == 16777216) {
         // user page
         user_page = true;
      } else {
         fMfe->Msg(MERROR, "Identify", "%s: unexpected value of update.page_select: %s", fOdbName.c_str(), page_select_str.c_str());
         fCheckId.Fail("incompatible firmware, update.page_select: " + page_select_str);
         return false;
      }


#if 0
      if (xatoi(elf_buildtime.c_str()) == 0x59f91401) {
      } else if (xatoi(elf_buildtime.c_str()) == 0x59a1bb8e) {
         // 0x59a4915b-0x59a1bb8e-0x59a1bb8e
      } else if (xatoi(elf_buildtime.c_str()) == 0x5a1cccf0) {
         // 0x59a4915b-0x59a1bb8e-0x59a1bb8e
      } else if (xatoi(elf_buildtime.c_str()) == 0x5a1de902) {
         // 0x59a4915b-0x59a1bb8e-0x59a1bb8e
      } else {
         mfe->Msg(MINFO, "Identify", "%s: firmware is not compatible with the daq", fOdbName.c_str());
         return false;
      }
#endif

      bool boot_load_only = false;

      if (elf_ts == 0xdeaddead) {
         boot_load_only = true;
      } else if (elf_ts == 0x59cc3664) { // has clock select, no udp
         boot_load_only = true;
      } else if (elf_ts == 0x59f227ec) { // has udp, but no clock select
         boot_load_only = true;
      } else if (elf_ts == 0x5a1de902) { // current good, bad data alignement
      } else if (elf_ts == 0x5a2850a5) { // current good
      } else if (elf_ts == 0x5a5d21a8) { // K.O. build
      } else {
         fMfe->Msg(MERROR, "Identify", "%s: firmware is not compatible with the daq, elf_buildtime 0x%08x", fOdbName.c_str(), elf_ts);
         fCheckId.Fail("incompatible firmware, elf_buildtime: " + elf_buildtime);
         return false;
      }

      if (qsys_sw_ts != qsys_hw_ts) {
         fMfe->Msg(MERROR, "Identify", "%s: firmware is not compatible with the daq, qsys mismatch, sw 0x%08x vs hw 0x%08x", fOdbName.c_str(), qsys_sw_ts, qsys_hw_ts);
         fCheckId.Fail("incompatible firmware, qsys timestamp mismatch, sw: " + sw_qsys_ts + ", hw: " + hw_qsys_ts);
         return false;
      }

      if (sof_ts == 0xdeaddead) {
      } else if (sof_ts == 0) {
      } else {
         fMfe->Msg(MERROR, "Identify", "%s: firmware is not compatible with the daq, sof fpga_build  0x%08x", fOdbName.c_str(), sof_ts);
         fCheckId.Fail("incompatible firmware, fpga_build: " + fpga_build);
         return false;
      }

      bool boot_from_user_page = false;
      fEq->fOdbEqSettings->RB("PWB/boot_user_page", fOdbIndex, &boot_from_user_page, false);

      if (boot_from_user_page != user_page) {
         if (boot_from_user_page) {
            fMfe->Msg(MERROR, "Identify", "%s: rebooting to the epcq user page", fOdbName.c_str());
            fEsper->Write(fMfe, "update", "sel_page", "0x01000000");
            fEsper->Write(fMfe, "update", "reconfigure", "y", true);
            return false;
         }
      }

      if (boot_load_only) {
         fMfe->Msg(MERROR, "Identify", "%s: firmware is not compatible with the daq, usable as boot loader only", fOdbName.c_str());
         fCheckId.Fail("incompatible firmware, usable as boot loader only");
         return false;
      }

      const char* s = fOdbName.c_str();
      while (*s && !isdigit(*s)) {
         s++;
      }
      fModule = atoi(s);

      fNumBanks = 256;

      fCheckId.Ok();

      return true;
   }

   bool ConfigurePwbLocked()
   {
      assert(fEsper);

      if (fEsper->fFailed) {
         printf("Configure %s: failed flag\n", fOdbName.c_str());
         return false;
      }

      bool ok = true;

      //
      // clockcleaner/clkin_sel values:
      //
      // 0 = CLKin0 (external)
      // 1 = CLKin1 (sfp)
      // 2 = CLKin2 (internal osc)
      // 3 = auto
      //
      int clkin_sel = 0;

      //
      // signalproc/trig_delay
      //
      int trig_delay = 312;

      // 
      // sca/gain values:
      //
      // 0 = 120 fC
      // 1 = 240 fC
      // 2 = 360 fC
      // 3 = 600 fC
      //
      int sca_gain = 0;

      fEq->fOdbEqSettings->RI("PWB/clkin_sel", 0, &clkin_sel, true);
      fEq->fOdbEqSettings->RI("PWB/trig_delay", 0, &trig_delay, true);
      fEq->fOdbEqSettings->RI("PWB/sca_gain", 0, &sca_gain, true);

      fMfe->Msg(MINFO, "ConfigurePwbLocked", "%s: configure: clkin_sel %d, trig_delay %d, sca gain %d", fOdbName.c_str(), clkin_sel, trig_delay, sca_gain);

      // make sure everything is stopped

      ok &= StopPwbLocked();

      // switch clock to external clock

      std::string x_clkin_sel_string = fEsper->Read(fMfe, "clockcleaner", "clkin_sel");
      int x_clkin_sel = xatoi(x_clkin_sel_string.c_str());

      if (x_clkin_sel != clkin_sel) {
         printf("%s: clkin_sel: [%s] %d should be %d\n", fOdbName.c_str(), x_clkin_sel_string.c_str(), x_clkin_sel, clkin_sel);

         fMfe->Msg(MINFO, "ConfigurePwbLocked", "%s: configure: switching the clock source clkin_sel from %d to %d", fOdbName.c_str(), x_clkin_sel, clkin_sel);

         ok &= fEsper->Write(fMfe, "clockcleaner", "clkin_sel", toString(clkin_sel).c_str());
      }

      // configure the trigger

      ok &= fEsper->Write(fMfe, "signalproc", "trig_delay", toString(trig_delay).c_str());

      // configure the SCAs

      ok &= fEsper->Write(fMfe, "sca0", "gain", toString(sca_gain).c_str());
      ok &= fEsper->Write(fMfe, "sca1", "gain", toString(sca_gain).c_str());
      ok &= fEsper->Write(fMfe, "sca2", "gain", toString(sca_gain).c_str());
      ok &= fEsper->Write(fMfe, "sca3", "gain", toString(sca_gain).c_str());

#if 0
      // program the IP address and port number in the UDP transmitter

      int udp_ip = 0;
      udp_ip |= (192<<24);
      udp_ip |= (168<<16);
      udp_ip |= (1<<8);
      udp_ip |= (1<<0);

      ok &= Write("udp", "dst_ip", toString(udp_ip).c_str());
      ok &= Write("udp", "dst_port", toString(udp_port).c_str());
      ok &= Write("udp", "enable", "true");
#endif

      fMfe->Msg(MINFO, "ConfigurePwbLocked", "%s: configure ok", fOdbName.c_str());

      return ok;
   }

   bool StartPwbLocked()
   {
      assert(fEsper);
      bool ok = true;
      ok &= fEsper->Write(fMfe, "signalproc", "ext_trig_ena", "true");
      ok &= fEsper->Write(fMfe, "signalproc", "force_run", "true");
      //fMfe->Msg(MINFO, "StartPwbLocked", "%s: started", fOdbName.c_str());
      return ok;
   }

   bool StopPwbLocked()
   {
      assert(fEsper);
      bool ok = true;
      ok &= fEsper->Write(fMfe, "signalproc", "force_run", "false");
      ok &= fEsper->Write(fMfe, "signalproc", "ext_trig_ena", "false");
      //fMfe->Msg(MINFO, "StopPwbLocked", "%s: stopped", fOdbName.c_str());
      return ok;
   }

   bool SoftTriggerPwbLocked()
   {
      assert(fEsper);
      //printf("SoftTrigger!\n");
      bool ok = true;
      ok &= fEsper->Write(fMfe, "signalproc", "ext_trig_inv", "true");
      ok &= fEsper->Write(fMfe, "signalproc", "ext_trig_inv", "false");
      //printf("SoftTrigger done!\n");
      return ok;
   }

   void ThreadPwb()
   {
      printf("thread for %s started\n", fOdbName.c_str());
      assert(fEsper);
      while (!fMfe->fShutdown) {
         if (fEsper->fFailed) {
            bool ok;
            {
               std::lock_guard<std::mutex> lock(fLock);
               ok = IdentifyPwbLocked();
               // fLock implicit unlock
            }
            if (!ok) {
               fState = ST_BAD_IDENTIFY;
               for (int i=0; i<fFailedSleep; i++) {
                  if (fMfe->fShutdown)
                     break;
                  sleep(1);
               }
               continue;
            }
         }

         {
            std::lock_guard<std::mutex> lock(fLock);
            ReadAndCheckPwbLocked();
         }

         for (int i=0; i<fPollSleep; i++) {
            if (fMfe->fShutdown)
               break;
            sleep(1);
         }
      }
      printf("thread for %s shutdown\n", fOdbName.c_str());
   }

   std::thread* fThread = NULL;

   void StartThreadsPwb()
   {
      //std::thread * t = new std::thread(&PwbCtrl::ThreadPwb, fPwbCtrl[i]);
      //t->detach();

      assert(fThread == NULL);
      fThread = new std::thread(&PwbCtrl::ThreadPwb, this);
   }

   void JoinThreadsPwb()
   {
      if (fThread) {
         fThread->join();
         delete fThread;
         fThread = NULL;
      }
   }

   void ReadAndCheckPwbLocked()
   {
      if (!fEsper) {
         fState = ST_NO_ESPER;
         return;
      }

      EsperNodeData e;

      bool ok = ReadPwbLocked(&e);
      if (!ok) {
         fState = ST_BAD_READ;
         return;
      }

      ok = CheckPwbLocked(e);
      if (!ok) {
         fState = ST_BAD_CHECK;
         return;
      }

      fState = ST_GOOD;
   }

   void BeginRunPwbLocked(bool start, bool enablePwbTrigger)
   {
      if (!fEsper)
         return;
      fEnablePwbTrigger = enablePwbTrigger;
      bool ok = IdentifyPwbLocked();
      if (!ok) {
         fState = ST_BAD_IDENTIFY;
         return;
      }
      ConfigurePwbLocked();
      ReadAndCheckPwbLocked();
      //WriteVariables();
      if (start && enablePwbTrigger) {
         StartPwbLocked();
      }
   }
};

// test griffin parameter read/write:  param r|w par chan val
//    gcc -g -o param param.c

#include <stdio.h>
#include <netinet/in.h> /* sockaddr_in, IPPROTO_TCP, INADDR_ANY */
#include <netdb.h>      /* gethostbyname */
#include <stdlib.h>     /* exit() */
#include <string.h>     /* memset() */
#include <errno.h>      /* errno */
#include <unistd.h>     /* usleep */
#include <time.h>       /* select */

typedef struct parname_struct {
   int param_id; const char* const param_name;
} Parname;

static const Parname param_names[]={
   { 1,  "threshold"},{ 2, "pulserctrl"},{ 3,   "polarity"},{ 4,  "diffconst"},
   { 5, "integconst"},{ 6, "decimation"},{ 7, "numpretrig"},{ 8, "numsamples"},
   { 9,   "polecorn"},{10,   "hitinteg"},{11,    "hitdiff"},{12, "integdelay"},
   {13,"wavebufmode"},{14,  "trigdtime"},{15,  "enableadc"},{16,    "dettype"},
   {17,   "synclive"},{18,   "syncdead"},{19,   "cfddelay"},{20,    "cfdfrac"},
   {21,   "exthiten"},{22,  "exthitreq"},{23,  "chandelay"},{24,   "blrspeed"},
   {25,  "phtrigdly"},{26, "cfdtrigdly"},{27,    "cfddiff"},{28,     "cfdint"},
                                                            {31,  "timestamp"},
   {32,  "eventrate"},{33, "evraterand"},{34,   "baseline"},{35,   "risetime"},
   {36,    "blrauto"},{37, "blinedrift"},{38,      "noise"},
                                         {63,        "csr"},{64,   "chanmask"},
   {65, "trigoutdly"},{66, "trigoutwid"},{67,"exttrigcond"},
   {128,   "filter1"},{129,   "filter2"},{130,   "filter3"},{131,   "filter4"},
   {132,   "filter5"},{133,   "filter6"},{134,   "filter7"},{135,   "filter8"},
   {136,   "filter9"},{137,  "filter10"},{138,  "filter11"},{139,  "filter12"},
   {140,  "filter13"},{141,  "filter14"},{142,  "filter15"},{143,  "filter16"},
   {144,   "pscale1"},{145,   "pscale2"},{146,   "pscale3"},{147,   "pscale4"},
   {148,  "coincwin"},
   {256,   "scaler1"},{257,   "scaler2"},{258,   "scaler3"},{259,   "scaler4"},
   {260,   "scaler5"},{261,   "scaler6"},{262,   "scaler7"},{263,   "scaler8"},
   {264,   "scaler9"},{265,  "scaler10"},{266,  "scaler11"},{267,  "scaler12"},
   {268,  "scaler13"},{269,  "scaler14"},{270,  "scaler15"},{271,  "scaler16"},
   {272,  "scaler17"},{273,  "scaler18"},{274,  "scaler19"},{275,  "scaler20"},
   {276,  "scaler21"},{277,  "scaler22"},{278,  "scaler23"},{279,  "scaler24"},
   {-1,    "invalid"}
};

class GrifComm
{
public:
   TMFE* mfe = NULL;
   std::string fHostname;

public:
   struct sockaddr fCmdAddr;
   int fCmdAddrLen = 0;
   int fCmdSocket = -1;

   struct sockaddr fDataAddr;
   int fDataAddrLen = 0;
   int fDataSocket = -1;

   int fDebug = 0;
   int fTimeout_usec = 10000;

public:
   //std::string fError = "constructed";
   bool fFailed = true;

public:   
   static const int kMaxPacketSize = 1500;
   
   ///////////////////////////////////////////////////////////////////////////
   //////////////////////      network data link      ////////////////////////

private:
   bool sndmsg(int sock_fd, const struct sockaddr *addr, int addr_len, const char *message, int msglen, std::string* errstr)
   {
      int flags=0;
      int bytes = sendto(sock_fd, message, msglen, flags, addr, addr_len);
      if (bytes < 0) {
         char err[256];
         sprintf(err, "sndmsg: sendto(%d) failed, errno %d (%s)", msglen, errno, strerror(errno));
         *errstr = err;
         //mfe->Msg(MERROR, "AlphaTctrl::sendmsg", "sndmsg: sendto failed, errno %d (%s)", errno, strerror(errno));
         return false;
      }
      if (bytes != msglen) {
         char err[256];
         sprintf(err, "sndmsg: sendto(%d) failed, short return %d", msglen, bytes);
         *errstr = err;
         //mfe->Msg(MERROR, "AlphaTctrl::sendmsg", "sndmsg: sendto failed, short return %d", bytes);
         return false;
      }
      if (fDebug) {
         printf("sendmsg %s\n", message);
      }
      return true;
   }

private:   
   bool waitmsg(int socket, int timeout, std::string *errstr)
   {
      //printf("waitmsg: timeout %d\n", timeout);
      
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = timeout;

      fd_set read_fds;
      FD_ZERO(&read_fds);
      FD_SET(socket, &read_fds);

      int num_fd = socket+1;
      int ret = select(num_fd, &read_fds, NULL, NULL, &tv);
      if (ret < 0) {
         char err[256];
         sprintf(err, "waitmsg: select() returned %d, errno %d (%s)", ret, errno, strerror(errno));
         *errstr = err;
         return false;
      } else if (ret == 0) {
         *errstr = "timeout";
         return false; // timeout
      } else {
         return true; // have data
      }
   }

public:   
   //
   // readmsg(): return -1 for error, 0 for timeout, otherwise number of bytes read
   //
   int readmsg(int socket, char* replybuf, int bufsize, int timeout, std::string *errstr)
   {
      //printf("readmsg enter!\n");
      
      bool ok = waitmsg(socket, timeout, errstr);
      if (!ok) {
         *errstr = "readmsg: " + *errstr;
         return 0;
      }

      int flags=0;
      struct sockaddr client_addr;
      socklen_t client_addr_len;
      int bytes = recvfrom(socket, replybuf, bufsize, flags, &client_addr, &client_addr_len);

      if (bytes < 0) {
         char err[256];
         sprintf(err, "readmsg: recvfrom() returned %d, errno %d (%s)", bytes, errno, strerror(errno));
         *errstr = err;
         //mfe->Msg(MERROR, "AlphaTctrl::readmsg", "readmsg: recvfrom failed, errno %d (%s)", errno, strerror(errno));
         return -1;
      }

      if (fDebug) {
         printf("readmsg return %d\n", bytes);
      }
      return bytes;
   }

private:
   void flushmsg(int socket, std::string *errstr)
   {
      while (1) {
         char replybuf[kMaxPacketSize];
         int rd = readmsg(socket, replybuf, sizeof(replybuf), 1, errstr);
         if (rd <= 0)
            break;
         printf("flushmsg read %d\n", rd);
      }
   }

private:   
   int open_udp_socket(int server_port, const char *hostname, struct sockaddr *addr, int *addr_len)
   {
      struct sockaddr_in local_addr;
      struct hostent *hp;
      int sock_fd;
      
      int mxbufsiz = 0x00800000; /* 8 Mbytes ? */
      int sockopt=1; // Re-use the socket
      if( (sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
         mfe->Msg(MERROR, "AlphaTctrl::open_udp_socket", "cannot create udp socket, socket(AF_INET,SOCK_DGRAM) errno %d (%s)", errno, strerror(errno));
         return(-1);
      }
      setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int));
      if(setsockopt(sock_fd, SOL_SOCKET,SO_RCVBUF, &mxbufsiz, sizeof(int)) == -1){
         mfe->Msg(MERROR, "AlphaTctrl::open_udp_socket", "cannot create udp socket, setsockopt(SO_RCVBUF) errno %d (%s)", errno, strerror(errno));
         return(-1);
      }
      memset(&local_addr, 0, sizeof(local_addr));
      local_addr.sin_family = AF_INET;
      local_addr.sin_port = htons(0);          // 0 => use any available port
      local_addr.sin_addr.s_addr = INADDR_ANY; // listen to all local addresses
      if( bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr) )<0) {
         mfe->Msg(MERROR, "AlphaTctrl::open_udp_socket", "cannot create udp socket, bind() errno %d (%s)", errno, strerror(errno));
         return(-1);
      }
      // now fill in structure of server addr/port to send to ...
      *addr_len = sizeof(struct sockaddr);
      bzero((char *) addr, *addr_len);
      if( (hp=gethostbyname(hostname)) == NULL ){
         mfe->Msg(MERROR, "AlphaTctrl::open_udp_socket", "cannot create udp socket, gethostbyname() errno %d (%s)", errno, strerror(errno));
         return(-1);
      }
      struct sockaddr_in* addrin = (struct sockaddr_in*)addr;
      memcpy(&addrin->sin_addr, hp->h_addr_list[0], hp->h_length);
      addrin->sin_family = AF_INET;
      addrin->sin_port = htons(server_port);
      
      return sock_fd;
   }

private:   
   void printreply(const char* replybuf, int bytes)
   {
      int i;
      printf("  ");
      for(i=0; i<bytes; i++){
         printf("%02x", replybuf[i]);
         if( !((i+1)%2) ){ printf(" "); }
      }
      printf("::");
      for(i=0; i<bytes; i++){
         if( replybuf[i] > 20 && replybuf[i] < 127 ){
            printf("%c", replybuf[i]);
         } else {
            printf(".");
         }
      }
      printf("\n");
   }
   
private:
   const char *parname(int num)
   {
      int i;
      for(i=0; ; i++) {
         if (param_names[i].param_id == num || param_names[i].param_id == -1) {
            break;
         }
      }
      return( param_names[i].param_name );
   }
   
   const int WRITE = 1;
   const int READ = 0;

   // initial test ...
   //   p,a, r,m,   2,0, 0,0,   83,3, 0,0    len=12
   //   data sent to chan starts with rm, ends with 83,3
   //   memcpy(msgbuf, "PARM", 4);
   //   *(unsigned short *)(&msgbuf[4]) =   2; // parnum = 2       [0x0002]
   //   *(unsigned short *)(&msgbuf[6]) =   0; // op=write, chan=0 [0x0000]
   //   *(unsigned int   *)(&msgbuf[8]) = 899; // value = 899 [0x0000 0383]
   void param_encode(char *buf, int par, int write, int chan, int val)
   {
      memcpy(buf, "PARM", 4);
      buf [4] = (par & 0x3f00)     >>  8; buf[ 5] = (par & 0x00ff);
      buf[ 6] = (chan& 0xFf00)     >>  8; buf[ 7] = (chan& 0x00ff);
      buf[ 8] = (val & 0xff000000) >> 24; buf[ 9] = (val & 0x00ff0000) >> 16;
      buf[10] = (val & 0x0000ff00) >>  8; buf[11] = (val & 0x000000ff);
      if( ! write ){ buf[4] |= 0x40; }
   }

public:   
   bool try_write_param(int par, int chan, int val, std::string *errstr)
   {
      flushmsg(fCmdSocket, errstr);

      char msgbuf[256];
      //printf("Writing %d [0x%08x] into %s[par%d] on chan%d[%2d,%2d,%3d]\n", val, val, parname(par), par, chan, chan>>12, (chan>>8)&0xF, chan&0xFF);
      param_encode(msgbuf, par, WRITE, chan, val);
      bool ok = sndmsg(fCmdSocket, &fCmdAddr, fCmdAddrLen, msgbuf, 12, errstr);
      if (!ok) {
         *errstr = "try_write_param: " + *errstr;
         return false;
      }

      char replybuf[kMaxPacketSize];

      int bytes = readmsg(fCmdSocket, replybuf, sizeof(replybuf), fTimeout_usec, errstr);

      if (bytes == 0) {
         *errstr = "try_write_param: " + *errstr;
         //mfe->Msg(MERROR, "AlphaTctrl::write_param", "write_param timeout: %s", fError.c_str());
         //fFailed = true;
         return false;
      } else if (bytes < 0) {
         *errstr = "try_write_param: " + *errstr;
         //mfe->Msg(MERROR, "AlphaTctrl::write_param", "write_param error: %s", fError.c_str());
         //fFailed = true;
         return false;
      }
      if (bytes != 4) {
         //fFailed = true;
         char err[256];
         sprintf(err, "write_param: wrong reply packet size %d should be 4", bytes);
         *errstr = err;
         //mfe->Msg(MERROR, "AlphaTctrl::write_param", "write_param: wrong reply packet size %d should be 4", bytes);
         return false;
      }
      if (strncmp((char*)replybuf, "OK  ", 4) != 0) {
         //fFailed = true;
         char err[256];
         sprintf(err, "write_param: reply packet is not OK: [%s]", replybuf);
         *errstr = err;
         //mfe->Msg(MERROR, "AlphaTctrl::write_param", "write_param: reply packet is not OK: [%s]", replybuf);
         return false;
      }
      return true;
   }

public:   
   bool try_read_param(int par, int chan, uint32_t* value, std::string *errstr)
   {
      //printf("Reading %s[par%d] on chan%d[%2d,%2d,%3d]\n", parname(par), par, chan, chan>>12, (chan>>8)&0xF, chan&0xFF);
      flushmsg(fCmdSocket, errstr);
      char msgbuf[256];
      param_encode(msgbuf, par, READ, chan, 0);
      bool ok = sndmsg(fCmdSocket, &fCmdAddr, fCmdAddrLen, msgbuf, 12, errstr);
      if (!ok) {
         *errstr = "try_read_param: " + *errstr;
         return false;
      }

      char replybuf[kMaxPacketSize];

      int bytes = readmsg(fCmdSocket, replybuf, sizeof(replybuf), fTimeout_usec, errstr);

      if (bytes == 0) {
         *errstr = "try_read_param: " + *errstr;
         return false;
      } else if (bytes < 0) {
         *errstr = "try_read_param: " + *errstr;
         fFailed = true;
         return false;
      }

      if (bytes != 4) {
         char err[256];
         sprintf(err, "try_read_param: wrong reply packet size %d should be 4", bytes);
         *errstr = err;
         //mfe->Msg(MERROR, "AlphaTctrl::read_param", "read_param: wrong reply packet size %d should be 4", bytes);
         return false;
      }
      //replybuf[bytes+1] = 0;
      //printf("   reply      :%d bytes ... [%s]\n", bytes, replybuf);
      if (strncmp((char*)replybuf, "OK  ", 4) != 0) {
         char err[256];
         sprintf(err, "try_read_param: reply packet is not OK: [%s]", replybuf);
         *errstr = err;
         //mfe->Msg(MERROR, "AlphaTctrl::read_param", "read_param: reply packet is not OK: [%s]", replybuf);
         return false;
      }

      bytes = readmsg(fCmdSocket, replybuf, sizeof(replybuf), fTimeout_usec, errstr);
      if (bytes == 0) {
         *errstr = "try_read_param: RDBK: " + *errstr;
         return false;
      } else if (bytes < 0) {
         *errstr = "try_read_param: RDBK: " + *errstr;
         fFailed = true;
         return false;
      }
      if (bytes != 12) {
         char err[256];
         sprintf(err, "try_read_param: wrong RDBK packet size %d should be 12", bytes);
         *errstr = err;
         //mfe->Msg(MERROR, "AlphaTctrl::read_param", "read_param: wrong RDBK packet size %d should be 12", bytes);
         return false;
      }
      if (strncmp((char*)replybuf, "RDBK", 4) != 0) {
         replybuf[5] = 0;
         char err[256];
         sprintf(err, "try_read_param: RDBK packet is not RDBK: [%s]", replybuf);
         *errstr = err;
         //mfe->Msg(MERROR, "AlphaTctrl::read_param", "read_param: RDBK packet is not RDBK: [%s]", replybuf);
         return false;
      }
      //printf("   read-return:%d bytes ... ", bytes);
      //printreply(replybuf, bytes);

      if (!(replybuf[4] & 0x40)) { // readbit not set
         *errstr = "try_read_param: RDBK packet readbit is not set";
         //mfe->Msg(MERROR, "AlphaTctrl::read_param", "read_param: RDBK packet readbit is not set");
         return false;
      }

      int xpar  = ((replybuf[4] & 0x3f ) << 8) | (replybuf[5]&0xFF);
      int xchan = ((replybuf[6] & 0xff ) << 8) | (replybuf[7]&0xFF);
      uint32_t val  = ((replybuf[8]&0xFF)<<24) | ((replybuf[9]&0xFF)<<16) | ((replybuf[10]&0xFF)<<8) | (replybuf[11]&0xFF);

      if (xpar != par || xchan != xchan) {
         *errstr = "try_read_param: RDBK packet par or chan mismatch";
         //mfe->Msg(MERROR, "AlphaTctrl::read_param", "read_param: RDBK packet par or chan mismatch");
         return false;
      }

      *value = val;
      return true;
   }

public:   
   bool read_param(int par, int chan, uint32_t* value)
   {
      std::string errs;
      for (int i=0; i<5; i++) {
         if (fFailed) {
            return false;
         }
         std::string errstr;
         bool ok = try_read_param(par, chan, value, &errstr);
         if (!ok) {
            if (errs.length() > 0)
               errs += ", ";
            errs += errstr;
            continue;
         }

         if (i>0) {
            mfe->Msg(MINFO, "read_param", "read_param(%d,%d) ok after %d retries: %s", par, chan, i, errs.c_str());
         }
         return ok;
      }
      fFailed = true;
      mfe->Msg(MERROR, "read_param", "read_param(%d,%d) failed after retries: %s", par, chan, errs.c_str());
      return false;
   }

public:   
   bool write_param(int par, int chan, int value)
   {
      std::string errs;
      for (int i=0; i<5; i++) {
         if (fFailed) {
            return false;
         }
         std::string errstr;
         bool ok = try_write_param(par, chan, value, &errstr);
         if (!ok) {
            if (errs.length() > 0)
               errs += ", ";
            errs += errstr;
            continue;
         }

         if (i>0) {
            mfe->Msg(MINFO, "write_param", "write_param(%d,%d,%d) ok after %d retries: %s", par, chan, value, i, errs.c_str());
         }
         return ok;
      }
      fFailed = true;
      mfe->Msg(MERROR, "write_param", "write_param(%d,%d,%d) failed after retries: %s", par, chan, value, errs.c_str());
      return false;
   }

public:
   bool write_drq()
   {
      if (fFailed) {
         return false;
      }

      std::string errstr;
      flushmsg(fCmdSocket, &errstr);

      char msgbuf[256];

      //printf("Writing %d [0x%08x] into %s[par%d] on chan%d[%2d,%2d,%3d]\n", val, val, parname(par), par, chan, chan>>12, (chan>>8)&0xF, chan&0xFF);
      param_encode(msgbuf, 0, WRITE, 0, 0);
      memcpy(msgbuf, "DRQ ", 4);
      bool ok = sndmsg(fDataSocket, &fDataAddr, fDataAddrLen, msgbuf, 12, &errstr);
      if (!ok) {
         errstr = "write_drq: " + errstr;
         mfe->Msg(MERROR, "AlphaTctrl::write_drq", "write_drq error: %s", errstr.c_str());
         return false;
      }

      char replybuf[kMaxPacketSize];

      int bytes = readmsg(fCmdSocket, replybuf, sizeof(replybuf), fTimeout_usec, &errstr);

      if (bytes == 0) {
         errstr = "write_drq: " + errstr;
         mfe->Msg(MERROR, "AlphaTctrl::write_drq", "write_drq: timeout");
         return false;
      } else if (bytes < 0) {
         errstr = "write_drq: " + errstr;
         mfe->Msg(MERROR, "AlphaTctrl::write_drq", "write_drq error: %s", errstr.c_str());
         fFailed = true;
         return false;
      }

      if (bytes != 4) {
         mfe->Msg(MERROR, "AlphaTctrl::write_drq", "write_drq: wrong reply packet size %d should be 4", bytes);
         return false;
      }
      if (strncmp((char*)replybuf, "OK  ", 4) != 0) {
         mfe->Msg(MERROR, "AlphaTctrl::write_drq", "write_drq: reply packet is not OK: [%s]", replybuf);
         return false;
      }
      return true;
   }

public:
   bool write_stop()
   {
      if (fFailed) {
         return false;
      }

      std::string errstr;
      flushmsg(fCmdSocket, &errstr);

      char msgbuf[256];

      //printf("Writing %d [0x%08x] into %s[par%d] on chan%d[%2d,%2d,%3d]\n", val, val, parname(par), par, chan, chan>>12, (chan>>8)&0xF, chan&0xFF);
      param_encode(msgbuf, 0, WRITE, 0, 0);
      memcpy(msgbuf, "STOP", 4);
      bool ok = sndmsg(fDataSocket, &fDataAddr, fDataAddrLen, msgbuf, 12, &errstr);
      if (!ok) {
         errstr = "write_stop: " + errstr;
         mfe->Msg(MERROR, "AlphaTctrl::write_stop", "write_stop error: %s", errstr.c_str());
         return false;
      }

      char replybuf[kMaxPacketSize];

      int bytes = readmsg(fCmdSocket, replybuf, sizeof(replybuf), fTimeout_usec, &errstr);

      if (bytes == 0) {
         errstr = "write_stop: " + errstr;
         mfe->Msg(MERROR, "AlphaTctrl::write_stop", "write_stop: timeout");
         return false;
      } else if (bytes < 0) {
         errstr = "write_stop: " + errstr;
         mfe->Msg(MERROR, "AlphaTctrl::write_stop", "write_stop error: %s", errstr.c_str());
         fFailed = true;
         return false;
      }

      if (bytes != 4) {
         mfe->Msg(MERROR, "AlphaTctrl::write_stop", "write_stop: wrong reply packet size %d should be 4", bytes);
         return false;
      }
      if (strncmp((char*)replybuf, "OK  ", 4) != 0) {
         mfe->Msg(MERROR, "AlphaTctrl::write_stop", "write_stop: reply packet is not OK: [%s]", replybuf);
         return false;
      }
      return true;
   }

#if 0
   bool ReadCsr(uint32_t* valuep)
   {
      return read_param(63, 0xFFFF, valuep);
   }

   bool xWriteCsr(uint32_t value)
   {
      printf("WriteCsr 0x%08x\n", value);

      bool ok = write_param(63, 0xFFFF, value);
      if (!ok)
         return false;

      uint32_t readback = 0;
      ok = ReadCsr(&readback);
      if (!ok)
         return false;

      if (value != readback) {
         mfe->Msg(MERROR, "WriteCsr", "ALPHAT %s CSR write failure: wrote 0x%08x, read 0x%08x", fOdbName.c_str(), value, readback);
         return false;
      }
      return true;
   }

   bool WriteCsr(uint32_t value)
   {
      for (int i=0; i<5; i++) {
         bool ok = xWriteCsr(value);
         if (ok) {
            if (i>0) {
               mfe->Msg(MERROR, "WriteCsr", "WriteCsr(0x%08x) ok after %d retries", value, i);
            }
            return ok;
         }
      }
      mfe->Msg(MERROR, "WriteCsr", "WriteCsr(0x%08x) failure after retries", value);
      return false;
   }
#endif

public:
   bool OpenSockets()
   {
      const int CMD_PORT  = 8808;
      const int DATA_PORT = 8800;

      bool ok = true;
      fCmdSocket  = open_udp_socket(CMD_PORT, fHostname.c_str(),&fCmdAddr,&fCmdAddrLen);
      fDataSocket = open_udp_socket(DATA_PORT,fHostname.c_str(),&fDataAddr,&fDataAddrLen);
      ok &= (fCmdSocket >= 0);
      ok &= (fDataSocket >= 0);
      return ok;
   }
};

#include "atpacket.h"

typedef std::vector<char> AtData;

std::vector<AtData*> gAtDataBuf;
std::mutex gAtDataBufLock;

class AlphaTctrl
{
public:
   TMFE* fMfe = NULL;
   TMFeEquipment* fEq = NULL;
   std::string fOdbName;
   GrifComm* fComm = NULL;

public:

   int fModule = 1;

   bool fVerbose = false;

   bool fOk = true;

   int fPollSleep = 10;
   int fFailedSleep = 10;

   int fNumBanks = 1;

   std::mutex fLock;

   int fUpdateCount = 0;

   int fDebug = 0;

   Fault fCheckComm;

public:
   AlphaTctrl(TMFE* mfe, TMFeEquipment* eq, const char* hostname, const char* odbname)
   {
      fMfe = mfe;
      fEq = eq;
      fOdbName = odbname;

      fComm = new GrifComm();
      fComm->mfe = mfe;
      fComm->fHostname = hostname;
      fComm->OpenSockets();
      
      fCheckComm.Setup(fMfe, fOdbName.c_str(), "communication");
   }

   std::string fLastCommError;

   bool IdentifyLocked()
   {
      //fComm->fFailed = false;

      bool ok = true;

      uint32_t timestamp = 0;

      std::string errstr;

      ok &= fComm->try_read_param(31, 0xFFFF, &timestamp, &errstr);

      if (!ok) {
         if (errstr != fLastCommError) {
            fMfe->Msg(MERROR, "Identify", "%s: communication failure: %s", fOdbName.c_str(), errstr.c_str());
            fLastCommError = errstr;
         }
         fCheckComm.Fail("try_read_param error");
         return false;
      }

      fComm->fFailed = false;

      time_t ts = (time_t)timestamp;
      const struct tm* tptr = localtime(&ts);
      char tstampbuf[256];
      strftime(tstampbuf, sizeof(tstampbuf), "%d%b%g_%H:%M", tptr);

      fMfe->Msg(MINFO, "Identify", "%s: firmware timestamp 0x%08x (%s)", fOdbName.c_str(), timestamp, tstampbuf);

      fCheckComm.Ok();

      return true;
   }

   //bool fConfCosmicEnable = false;
   bool fConfSwPulserEnable = false;
   double fConfSwPulserFreq = 1.0;
   int    fConfSyncCount = 5;
   double fConfSyncPeriodSec = 1.5;

   // pulser configuration
   double fConfPulserClockFreq = 62.5*1e6; // 62.5 MHz
   int fConfPulserWidthClk = 5;
   int fConfPulserPeriodClk = 0;
   double fConfPulserFreq = 10.0; // 10 Hz
   bool fConfRunPulser = true;
   bool fConfOutputPulser = true;

   int fConfTrigWidthClk = 5;

   int fConfBusyWidthClk = 10;
   int fConfA16BusyWidthClk  =     6250; // 100usec
   int fConfPwbBusyWidthClk = 62500000; // 1sec


   //int fConfSasTrigMask = 0;

   bool fConfTrigPulser = false;
   bool fConfTrigEsataNimGrandOr = false;

   bool fConfTrigAdc16GrandOr = false;
   bool fConfTrigAdc32GrandOr = false;

   bool fConfTrig1ormore = false;
   bool fConfTrig2ormore = false;
   bool fConfTrig3ormore = false;
   bool fConfTrig4ormore = false;

   bool fConfTrigAdc16Coinc = false;

   bool fConfTrigCoincA = false;
   bool fConfTrigCoincB = false;
   bool fConfTrigCoincC = false;
   bool fConfTrigCoincD = false;

   uint32_t fConfCoincA = 0;
   uint32_t fConfCoincB = 0;
   uint32_t fConfCoincC = 0;
   uint32_t fConfCoincD = 0;

   uint32_t fConfNimMask = 0;
   uint32_t fConfEsataMask = 0;

   int fSasLinkModId[16];
   std::string fSasLinkModName[16];

   bool fConfPassThrough = false;

   std::string LinkMaskToString(uint32_t mask)
   {
      std::string s;
      for (int i=0; i<16; i++) {
         if (mask & (1<<i)) {
            if (s.length() > 0)
               s += "+";
            s += "link";
            s += toString(i);
         }
      }
      return s;
   }

   std::string LinkMaskToAdcString(uint32_t mask)
   {
      std::string s;
      for (int i=0; i<16; i++) {
         if (mask & (1<<i)) {
            if (s.length() > 0)
               s += "+";
            s += fSasLinkModName[i];
         }
      }
      return s;
   }

   bool ConfigureAtLocked(bool enablePwbTrigger)
   {
      if (fComm->fFailed) {
         printf("Configure %s: no communication\n", fOdbName.c_str());
         return false;
      }

      //fEq->fOdbEqSettings->RB("CosmicEnable",   0, &fConfCosmicEnable, true);
      fEq->fOdbEqSettings->RB("SwPulserEnable", 0, &fConfSwPulserEnable, true);
      fEq->fOdbEqSettings->RD("SwPulserFreq",   0, &fConfSwPulserFreq, true);
      fEq->fOdbEqSettings->RI("SyncCount",      0, &fConfSyncCount, true);
      if (enablePwbTrigger)
         fEq->fOdbEqSettings->RD("PwbSyncPeriodSec",  0, &fConfSyncPeriodSec, true);
      else
         fEq->fOdbEqSettings->RD("SyncPeriodSec",  0, &fConfSyncPeriodSec, true);

      fEq->fOdbEqSettings->RD("Pulser/ClockFreqHz",     0, &fConfPulserClockFreq, true);
      fEq->fOdbEqSettings->RI("Pulser/PulseWidthClk",   0, &fConfPulserWidthClk, true);
      fEq->fOdbEqSettings->RI("Pulser/PulsePeriodClk",  0, &fConfPulserPeriodClk, true);
      fEq->fOdbEqSettings->RD("Pulser/PulseFreqHz",     0, &fConfPulserFreq, true);
      fEq->fOdbEqSettings->RB("Pulser/Enable",          0, &fConfRunPulser, true);
      fEq->fOdbEqSettings->RB("Pulser/OutputEnable",    0, &fConfOutputPulser, true);

      fEq->fOdbEqSettings->RI("TrigWidthClk",  0, &fConfTrigWidthClk, true);
      fEq->fOdbEqSettings->RI("A16BusyWidthClk",  0, &fConfA16BusyWidthClk, true);
      fEq->fOdbEqSettings->RI("FeamBusyWidthClk",  0, &fConfPwbBusyWidthClk, true);
      fEq->fOdbEqSettings->RI("BusyWidthClk",  0, &fConfBusyWidthClk, true);


      fEq->fOdbEqSettings->RB("TrigSrc/TrigPulser",  0, &fConfTrigPulser, true);
      fEq->fOdbEqSettings->RB("TrigSrc/TrigEsataNimGrandOr",  0, &fConfTrigEsataNimGrandOr, true);

      fEq->fOdbEqSettings->RB("TrigSrc/TrigAdc16GrandOr",  0, &fConfTrigAdc16GrandOr, true);
      fEq->fOdbEqSettings->RB("TrigSrc/TrigAdc32GrandOr",  0, &fConfTrigAdc32GrandOr, true);

      fEq->fOdbEqSettings->RB("TrigSrc/Trig1ormore",  0, &fConfTrig1ormore, true);
      fEq->fOdbEqSettings->RB("TrigSrc/Trig2ormore",  0, &fConfTrig2ormore, true);
      fEq->fOdbEqSettings->RB("TrigSrc/Trig3ormore",  0, &fConfTrig3ormore, true);
      fEq->fOdbEqSettings->RB("TrigSrc/Trig4ormore",  0, &fConfTrig4ormore, true);

      fEq->fOdbEqSettings->RB("TrigSrc/TrigAdc16Coinc",  0, &fConfTrigAdc16Coinc, true);

      fEq->fOdbEqSettings->RB("TrigSrc/TrigCoincA",  0, &fConfTrigCoincA, true);
      fEq->fOdbEqSettings->RB("TrigSrc/TrigCoincB",  0, &fConfTrigCoincB, true);
      fEq->fOdbEqSettings->RB("TrigSrc/TrigCoincC",  0, &fConfTrigCoincC, true);
      fEq->fOdbEqSettings->RB("TrigSrc/TrigCoincD",  0, &fConfTrigCoincD, true);

      fEq->fOdbEqSettings->RU32("Trig/CoincA",  0, &fConfCoincA, true);
      fEq->fOdbEqSettings->RU32("Trig/CoincB",  0, &fConfCoincB, true);
      fEq->fOdbEqSettings->RU32("Trig/CoincC",  0, &fConfCoincC, true);
      fEq->fOdbEqSettings->RU32("Trig/CoincD",  0, &fConfCoincD, true);

      fEq->fOdbEqSettings->RU32("Trig/NimMask",  0, &fConfNimMask, true);
      fEq->fOdbEqSettings->RU32("Trig/EsataMask",  0, &fConfEsataMask, true);

      fEq->fOdbEqSettings->RB("Trig/PassThrough",  0, &fConfPassThrough, true);

      //fEq->fOdbEqSettings->RI("SasTrigMask",  0, &fConfSasTrigMask, true);

      bool ok = true;

      ok &= StopAtLocked();

      fComm->write_param(0x25, 0xFFFF, 0); // disable all triggers
      fComm->write_param(0x08, 0xFFFF, AlphaTPacket::kPacketSize-2*4); // AT packet size in bytes minus the last 0xExxxxxxx word

      fMfe->Msg(MINFO, "Configure", "%s: enablePwbTrigger: %d", fOdbName.c_str(), enablePwbTrigger);

      fComm->write_param(0x20, 0xFFFF, fConfTrigWidthClk);

      if (enablePwbTrigger) {
         fComm->write_param(0x21, 0xFFFF, fConfPwbBusyWidthClk);
         fMfe->Msg(MINFO, "Configure", "%s: using pwb busy %d", fOdbName.c_str(), fConfPwbBusyWidthClk);
      } else {
         fComm->write_param(0x21, 0xFFFF, fConfA16BusyWidthClk);
         fMfe->Msg(MINFO, "Configure", "%s: using a16 busy %d", fOdbName.c_str(), fConfA16BusyWidthClk);
      }

      fComm->write_param(0x22, 0xFFFF, fConfPulserWidthClk);

      if (fConfPulserFreq) {
         int clk = fConfPulserClockFreq/fConfPulserFreq;
         fComm->write_param(0x23, 0xFFFF, clk);
         fMfe->Msg(MINFO, "Configure", "%s: pulser freq %f Hz, period %d clocks", fOdbName.c_str(), fConfPulserFreq, clk);
      } else {
         fComm->write_param(0x23, 0xFFFF, fConfPulserPeriodClk);
         fMfe->Msg(MINFO, "Configure", "%s: pulser period %d clocks, frequency %f Hz", fOdbName.c_str(), fConfPulserPeriodClk, fConfPulserFreq/fConfPulserPeriodClk);
      }

      //fComm->write_param(0x26, 0xFFFF, fConfSasTrigMask);

      // NIM and ESATA masks

      fComm->write_param(0x29, 0xFFFF, fConfNimMask);
      fComm->write_param(0x2A, 0xFFFF, fConfEsataMask);

      // ADC16 masks

      std::vector<int> adc16_mask;
      fEq->fOdbEqSettings->RIA("Trig/adc16_masks", &adc16_mask, true, 16);

      for (int i=0; i<8; i++) {
         int m0 = adc16_mask[2*i+0];
         int m1 = adc16_mask[2*i+1];
         uint32_t m = ((0xFFFF&((uint32_t)m1))<<16)|(0xFFFF&(uint32_t)m0);
         fComm->write_param(0x200+i, 0xFFFF, m);
      }

      // ADC32 masks

      std::vector<int> adc32_mask;
      fEq->fOdbEqSettings->RIA("Trig/adc32_masks", &adc32_mask, true, 16);

      for (int i=0; i<16; i++) {
         fComm->write_param(0x300+i, 0xFFFF, adc32_mask[i]);
      }

      ReadSasBitsLocked();

      fComm->write_param(0x2C, 0xFFFF, fConfCoincA);
      fComm->write_param(0x2D, 0xFFFF, fConfCoincB);
      fComm->write_param(0x2E, 0xFFFF, fConfCoincC);
      fComm->write_param(0x2F, 0xFFFF, fConfCoincD);

      fMfe->Msg(MINFO, "Configure", "%s: ConfCoincA,B,C,D: 0x%08x, 0x%08x, 0x%08x, 0x%08x", fOdbName.c_str(), fConfCoincA, fConfCoincB, fConfCoincC, fConfCoincD);

      fMfe->Msg(MINFO, "Configure", "%s: ConfCoincA: 0x%08x: (%s) * (%s)", fOdbName.c_str(), fConfCoincA, LinkMaskToString((fConfCoincA>>16)&0xFFFF).c_str(),LinkMaskToString(fConfCoincA&0xFFFF).c_str());
      fMfe->Msg(MINFO, "Configure", "%s: ConfCoincB: 0x%08x: (%s) * (%s)", fOdbName.c_str(), fConfCoincB, LinkMaskToString((fConfCoincB>>16)&0xFFFF).c_str(),LinkMaskToString(fConfCoincB&0xFFFF).c_str());
      fMfe->Msg(MINFO, "Configure", "%s: ConfCoincC: 0x%08x: (%s) * (%s)", fOdbName.c_str(), fConfCoincC, LinkMaskToString((fConfCoincC>>16)&0xFFFF).c_str(),LinkMaskToString(fConfCoincC&0xFFFF).c_str());
      fMfe->Msg(MINFO, "Configure", "%s: ConfCoincD: 0x%08x: (%s) * (%s)", fOdbName.c_str(), fConfCoincD, LinkMaskToString((fConfCoincD>>16)&0xFFFF).c_str(),LinkMaskToString(fConfCoincD&0xFFFF).c_str());

      fMfe->Msg(MINFO, "Configure", "%s: ConfCoincA: 0x%08x: (%s) * (%s)", fOdbName.c_str(), fConfCoincA, LinkMaskToAdcString((fConfCoincA>>16)&0xFFFF).c_str(),LinkMaskToAdcString(fConfCoincA&0xFFFF).c_str());
      fMfe->Msg(MINFO, "Configure", "%s: ConfCoincB: 0x%08x: (%s) * (%s)", fOdbName.c_str(), fConfCoincB, LinkMaskToAdcString((fConfCoincB>>16)&0xFFFF).c_str(),LinkMaskToAdcString(fConfCoincB&0xFFFF).c_str());
      fMfe->Msg(MINFO, "Configure", "%s: ConfCoincC: 0x%08x: (%s) * (%s)", fOdbName.c_str(), fConfCoincC, LinkMaskToAdcString((fConfCoincC>>16)&0xFFFF).c_str(),LinkMaskToAdcString(fConfCoincC&0xFFFF).c_str());
      fMfe->Msg(MINFO, "Configure", "%s: ConfCoincD: 0x%08x: (%s) * (%s)", fOdbName.c_str(), fConfCoincD, LinkMaskToAdcString((fConfCoincD>>16)&0xFFFF).c_str(),LinkMaskToAdcString(fConfCoincD&0xFFFF).c_str());

      return ok;
   }

   bool fRunning = false;
   int  fSyncPulses = 0;
   double fSyncPeriodSec = 0;

   bool StartAtLocked()
   {
      bool ok = true;

      fSyncPulses = fConfSyncCount;
      fSyncPeriodSec = fConfSyncPeriodSec;

      fRunning = false;

      uint32_t trig_enable = 0;

      if (fSyncPulses) {
         trig_enable |= (1<<0); // enable software trigger
      }

      if (!fConfPassThrough) {
         trig_enable |= (1<<13); // enable udp packets
         trig_enable |= (1<<14); // enable busy counter
      }

      fComm->write_param(0x25, 0xFFFF, trig_enable);

      fComm->write_drq(); // request udp packet data

      return ok;
   }

   bool StopAtLocked()
   {
      bool ok = true;
      ok &= fComm->write_param(0x25, 0xFFFF, 0); // disable all triggers
      fComm->write_stop(); // stop sending udp packet data
      fRunning = false;
      fSyncPulses = 0;
      return ok;
   }

   bool SoftTriggerLocked()
   {
      printf("AlphaTctrl::SoftTrigger!\n");
      bool ok = true;
      ok &= fComm->write_param(0x24, 0xFFFF, 0);
      return ok;
   }

   void ReadSasBitsLocked()
   {
      fComm->write_param(0x2B, 0xFFFF, 0); // write conf_latch

      for (int i=0; i<16; i++) {
         uint32_t v0;
         uint32_t v1;
         fComm->read_param(0x400+2*i+0, 0xFFFF, &v0);
         fComm->read_param(0x400+2*i+1, 0xFFFF, &v1);

         int modid = (v1&0xF0000000)>>28;

         if (v1 == 0)
            modid = -1;

         if (modid >= 0 && modid < 16) {
            fEq->fOdbEqSettings->RS("ADC/modules",  modid, &fSasLinkModName[i], false);
         }

         fMfe->Msg(MINFO, "ReadSasBits", "%s: link %2d sas_bits: 0x%08x%08x, adc[%d], %s", fOdbName.c_str(), i, v1, v0, modid, fSasLinkModName[i].c_str());

         fSasLinkModId[i] = modid;

      }
   }

   double fScPrevTime = 0;
   uint32_t fScPrevClk = 0;
   std::vector<int> fScPrev;

   void ReadScalers()
   {
      const int NRD = 16 + 8;
      const int NSC = 32;
      std::vector<int> sc;

      while (fScPrev.size() < NSC) {
         fScPrev.push_back(0);
      }

      double t = 0;

      const int iclk = 0;

      {
         std::lock_guard<std::mutex> lock(fLock);

         fComm->write_param(0x2B, 0xFFFF, 0); // write conf_latch

         for (int i=0; i<NRD; i++) {
            uint32_t v;
            fComm->read_param(0x100+i, 0xFFFF, &v);
            if (i==0) {
               t = TMFE::GetTime();
            }
            if (i<16) {
               sc.push_back(v);
            } else {
               sc.push_back(v&0xFFFF);
               sc.push_back((v>>16)&0xFFFF);
            }
         }
      }

      uint32_t clk = sc[iclk];

      uint32_t dclk = clk - fScPrevClk;
      double dclk_sec = dclk/(62.5*1e6);

      //printf("clk 0x%08x -> 0x%08x, dclk 0x%08x, time %f sec\n", fScPrevClk, clk, dclk, dclk_sec);

      fEq->fOdbEqVariables->WIA("scalers", sc);

      printf("scalers: ");
      for (int i=0; i<NSC; i++) {
         printf(" 0x%08x", sc[i]);
      }
      printf("\n");

      double dead_time = 0;

      if (fScPrevTime) {
         std::vector<double> rate;
         //double dt = t - fScPrevTime;
         double dt = dclk_sec;
      
         for (int i=0; i<NSC; i++) {
            int diff = sc[i]-fScPrev[i];
            double r = 0;
            if (dt > 0 && diff >= 0 && diff < 1*1000*1000) {
               r = diff/dt;
            }
            rate.push_back(r);
         }

         dead_time = 0;
         if (rate[1] > 0) {
            dead_time = 1.0 - rate[1]/rate[2];
         }

         fEq->fOdbEqVariables->WD("trigger_dead_time", dead_time);
         fEq->fOdbEqVariables->WDA("scalers_rate", rate);
      }

      fScPrevTime = t;
      fScPrevClk = clk;
      for (int i=0; i<NSC; i++) {
         fScPrev[i] = sc[i];
      }
   }

   time_t fNextScalers = 0;

   void MaybeReadScalers()
   {
      time_t now = time(NULL);

      if (fNextScalers == 0)
         fNextScalers = now;

      if (now >= fNextScalers) {
         fNextScalers += 10;
         ReadScalers();
      }
   }

   void Thread()
   {
      printf("thread for %s started\n", fOdbName.c_str());
      while (!fMfe->fShutdown) {
         if (fComm->fFailed) {
            fCheckComm.Fail("see previous messages");
            bool ok;
            {
               std::lock_guard<std::mutex> lock(fLock);
               ok = IdentifyLocked();
               // fLock implicit unlock
            }
            if (!ok) {
               fOk = false;
               for (int i=0; i<fFailedSleep; i++) {
                  if (fMfe->fShutdown)
                     break;
                  sleep(1);
               }
               continue;
            }
         }

         if (fSyncPulses) {
            printf("Sync pulse %d!\n", fSyncPulses);
            
            {
               std::lock_guard<std::mutex> lock(fLock);
               SoftTriggerLocked();
            }

            if (fSyncPulses > 0) {
               fSyncPeriodSec += 0.100;
               fSyncPulses--;
            }

            if (fSyncPulses == 0) {
               uint32_t trig_enable = 0;

               // trig_enable bits:
               // 1<<0 - conf_enable_sw_trigger
               // 1<<1 - conf_enable_pulser - trigger on pulser
               // 1<<2 - conf_enable_sas_or - trigger on (sas_trig_or|sas_bits_or|sas_bits_grand_or)
               // 1<<3 - conf_run_pulser - let the pulser run
               // 1<<4 - conf_output_pulser - send pulser to external output
               // 1<<5 - conf_enable_esata_nim - trigger on esata_nim_grand_or

               // wire conf_enable_sw_trigger = conf_trig_enable[0];
               // wire conf_enable_pulser = conf_trig_enable[1];
               // wire conf_enable_sas_or = conf_trig_enable[2];
               // wire conf_run_pulser = conf_trig_enable[3];
               // wire conf_output_pulser = conf_trig_enable[4];
               // wire conf_enable_esata_nim = conf_trig_enable[5];
               // wire conf_enable_adc16 = conf_trig_enable[6];
               // wire conf_enable_adc32 = conf_trig_enable[7];
               // wire conf_enable_1ormore = conf_trig_enable[8];
               // wire conf_enable_2ormore = conf_trig_enable[9];
               // wire conf_enable_3ormore = conf_trig_enable[10];
               // wire conf_enable_4ormore = conf_trig_enable[11];
               // wire conf_enable_adc16_coinc = conf_trig_enable[12];

               // wire conf_enable_udp     = conf_trig_enable[13];
               // wire conf_enable_busy    = conf_trig_enable[14];

               // wire conf_enable_coinc_a = conf_trig_enable[16];
               // wire conf_enable_coinc_b = conf_trig_enable[17];
               // wire conf_enable_coinc_c = conf_trig_enable[18];
               // wire conf_enable_coinc_d = conf_trig_enable[19];

               //if (fConfCosmicEnable) {
               //   //trig_enable |= (1<<2);
               //   trig_enable |= (1<<11);
               //}

               //if (fConfHwPulserEnable) {
               //   trig_enable |= (1<<1); // conf_enable_pulser
               //   trig_enable |= (1<<3); // conf_run_pulser
               //   if (fConfOutputPulser) {
               //      trig_enable |= (1<<4); // conf_output_pulser
               //   }
               //} else if (fConfOutputPulser) {
               //   trig_enable |= (1<<3); // conf_run_pulser
               //   trig_enable |= (1<<4); // conf_output_pulser
               //}

               if (fConfSwPulserEnable) {
                  trig_enable |= (1<<0);
               }

               if (fConfTrigPulser)
                  trig_enable |= (1<<1);

               if (fConfRunPulser)
                  trig_enable |= (1<<3);
               if (fConfOutputPulser)
                  trig_enable |= (1<<4);

               if (fConfTrigEsataNimGrandOr)
                  trig_enable |= (1<<5);

               if (fConfTrigAdc16GrandOr)
                  trig_enable |= (1<<6);
               if (fConfTrigAdc32GrandOr)
                  trig_enable |= (1<<7);

               if (fConfTrig1ormore)
                  trig_enable |= (1<<8);
               if (fConfTrig2ormore)
                  trig_enable |= (1<<9);
               if (fConfTrig3ormore)
                  trig_enable |= (1<<10);
               if (fConfTrig4ormore)
                  trig_enable |= (1<<11);

               if (fConfTrigAdc16Coinc)
                  trig_enable |= (1<<12);

               if (!fConfPassThrough) {
                  trig_enable |= (1<<13);
                  trig_enable |= (1<<14);
               }

               if (fConfTrigCoincA)
                  trig_enable |= (1<<16);
               if (fConfTrigCoincB)
                  trig_enable |= (1<<17);
               if (fConfTrigCoincC)
                  trig_enable |= (1<<18);
               if (fConfTrigCoincD)
                  trig_enable |= (1<<19);

               fMfe->Msg(MINFO, "AtCtrl::Tread", "%s: Writing trig_enable 0x%08x", fOdbName.c_str(), trig_enable);

               {
                  std::lock_guard<std::mutex> lock(fLock);
                  fComm->write_param(0x25, 0xFFFF, trig_enable);
               }
               
               fRunning = true;
            }

            double t0 = fMfe->GetTime();
            while (1) {
               double t1 = fMfe->GetTime();
               if (t1 - t0 > fSyncPeriodSec)
                  break;
               usleep(1000);
            };
         } else if (fRunning && fConfSwPulserEnable) {
            {
               std::lock_guard<std::mutex> lock(fLock);
               SoftTriggerLocked();
            }
            double t0 = fMfe->GetTime();
            while (1) {
               double t1 = fMfe->GetTime();
               if (t1 - t0 > 1.0/fConfSwPulserFreq)
                  break;
               MaybeReadScalers();
               usleep(1000);
            };
         } else {
            MaybeReadScalers();
            sleep(1);
         }
      }
      printf("thread for %s shutdown\n", fOdbName.c_str());
   }

   void ReadDataThread()
   {
      const double clk625 = 62.5*1e6; // 62.5 MHz
      uint32_t tsprev = 0;
      double tprev = 0;
      int epoch = 0;

      uint32_t prev_packet_no = 0;
      uint32_t prev_trig_no = 0;
      uint32_t prev_ts_625 = 0;

      printf("data thread for %s started\n", fOdbName.c_str());
      while (!fMfe->fShutdown) {
         std::string errstr;
         char replybuf[fComm->kMaxPacketSize];
         int rd = fComm->readmsg(fComm->fDataSocket, replybuf, sizeof(replybuf), 10000, &errstr);
         if (rd == 0) {
            // timeout
            continue;
         } else if (rd < 0) {
            fMfe->Msg(MERROR, "ReadDataThread", "%s: error reading UDP packet: %s", fOdbName.c_str(), errstr.c_str());
            continue;
         }

#if 0
         printf("ReadDataThread read %d\n", rd);
         uint32_t* p32 = (uint32_t*)replybuf;
         int s32 = rd/4;
         for (int i=0; i<s32; i++) {
            printf("%3d: 0x%08x\n", i, p32[i]);
         }
#endif
         
         if (1) {
            AlphaTPacket p;
            p.Unpack(replybuf, rd);

            if (prev_packet_no != 0) {
               if (p.packet_no != prev_packet_no + 1) {
                  printf("missing packets: %d..%d count %d\n", prev_packet_no, p.packet_no, p.packet_no - prev_packet_no);
                  fMfe->Msg(MERROR, "ReadDataThread", "ALPHAT %s missing packets: %d..%d count %d, ts 0x%08x -> 0x%08x diff 0x%08x", fOdbName.c_str(), prev_packet_no, p.packet_no, p.packet_no - prev_packet_no, prev_ts_625, p.ts_625, p.ts_625-prev_ts_625);
               }
            }
            prev_packet_no = p.packet_no;

            if (prev_trig_no != 0) {
               if (p.trig_no_header != prev_trig_no + 1) {
                  printf("missing triggers: %d..%d count %d\n", prev_trig_no, p.trig_no_header, p.trig_no_header - prev_trig_no);
                  fMfe->Msg(MERROR, "ReadDataThread", "ALPHAT %s missing triggers: %d..%d count %d", fOdbName.c_str(), prev_trig_no, p.trig_no_header, p.trig_no_header - prev_trig_no);
               }
            }
            prev_trig_no = p.trig_no_header;

            prev_ts_625 = p.ts_625;

            uint32_t ts = p.ts_625;
            
            if (ts < tsprev) {
               epoch++;
            }
            
            double t = ts/clk625 + epoch*2.0*(0x80000000/clk625);
            double dt = t-tprev;
            
            tprev = t;
            tsprev = p.ts_625;

            if (0) {
               p.Print();
               printf(", epoch %d, time %f, dt %f\n", epoch, t, dt);
            }
         }
         
         AtData *buf = new AtData;
         buf->resize(rd);
         memcpy(buf->data(), replybuf, rd);
            
         {
            std::lock_guard<std::mutex> lock(gAtDataBufLock);
            gAtDataBuf.push_back(buf);
         }
      }
      printf("data thread for %s shutdown\n", fOdbName.c_str());
   }

   void ReadAndCheckLocked()
   {
      //EsperNodeData e;
      //bool ok = ReadAll(&e);
      //if (ok) {
      //   ok = Check(e);
      //}
   }

   void BeginRunAtLocked(bool start, bool enablePwbTrigger)
   {
      IdentifyLocked();
      ConfigureAtLocked(enablePwbTrigger);
      ReadAndCheckLocked();
      //WriteVariables();
      //if (start) {
      //Start();
      //}
   }

   std::thread* fThread = NULL;
   std::thread* fDataThread = NULL;

   void StartThreads()
   {
      fThread = new std::thread(&AlphaTctrl::Thread, this);
      fDataThread = new std::thread(&AlphaTctrl::ReadDataThread, this);
   }

   void JoinThreads()
   {
      if (fThread) {
         fThread->join();
         delete fThread;
         fThread = NULL;
      }
      if (fDataThread) {
         fDataThread->join();
         delete fDataThread;
         fDataThread = NULL;
      }
   }
};

class Ctrl : public TMFeRpcHandlerInterface
{
public:
   TMFE* fMfe = NULL;
   TMFeEquipment* fEq = NULL;

   AlphaTctrl* fATctrl = NULL;
   std::vector<AdcCtrl*> fAdcCtrl;
   std::vector<PwbCtrl*> fPwbCtrl;

   bool fConfEnablePwbTrigger = true;
   bool fConfTrigPassThrough = false;

   int fNumBanks = 0;

   void WVD(const char* name, const std::vector<double> &v)
   {
      if (fMfe->fShutdown)
         return;
      
      std::string path;
      path += "/Equipment/";
      path += fEq->fName;
      path += "/Variables/";
      path += name;

      LOCK_ODB();

      //printf("Write ODB %s Variables %s: %s\n", C(path), name, v);
      int status = db_set_value(fMfe->fDB, 0, C(path), &v[0], sizeof(v[0])*v.size(), v.size(), TID_DOUBLE);
      if (status != DB_SUCCESS) {
         printf("WVD: db_set_value status %d\n", status);
      }
   };

   void WVI(const char* name, const std::vector<int> &v)
   {
      if (fMfe->fShutdown)
         return;
      
      std::string path;
      path += "/Equipment/";
      path += fEq->fName;
      path += "/Variables/";
      path += name;

      LOCK_ODB();

      //printf("Write ODB %s Variables %s, array size %d\n", C(path), name, (int)v.size());
      int status = db_set_value(fMfe->fDB, 0, C(path), &v[0], sizeof(v[0])*v.size(), v.size(), TID_INT);
      if (status != DB_SUCCESS) {
         printf("WVI: db_set_value status %d\n", status);
      }
   };

   void LoadOdb()
   {
      // check that LoadOdb() is not called twice
      assert(fATctrl == NULL);

      int countAT = 0;

      bool enable_at = true;

      fEq->fOdbEqSettings->RB("Trig/Enable", 0, &enable_at, true);

      if (enable_at) {
         std::vector<std::string> names;
         fEq->fOdbEqSettings->RSA("Trig/Modules", &names, true, 1, 32);

         if (names.size() > 0) {
            std::string name = names[0];
            if (name[0] != '#') {
               AlphaTctrl* at = new AlphaTctrl(fMfe, fEq, name.c_str(), name.c_str());
               fATctrl = at;
               countAT++;
            }
         }
      }

      printf("LoadOdb: TRG_MODULES: %d\n", countAT);

      // check that Init() is not called twice
      assert(fAdcCtrl.size() == 0);

      bool enable_adc = true;

      fEq->fOdbEqSettings->RB("ADC/enable", 0, &enable_adc, true);

      int countAdc = 0;
         
      if (enable_adc) {
         std::vector<std::string> modules;

         fEq->fOdbEqSettings->RSA("ADC/modules", &modules, true, 16, 32);
         fEq->fOdbEqSettings->RBA("ADC/boot_user_page", NULL, true, 16);
         fEq->fOdbEqSettings->RBA("ADC/adc16_enable", NULL, true, 16);
         fEq->fOdbEqSettings->RBA("ADC/adc32_enable", NULL, true, 16);

         for (unsigned i=0; i<modules.size(); i++) {
            std::string name = modules[i];
            
            //printf("index %d name [%s]\n", i, name.c_str());

            AdcCtrl* adc = new AdcCtrl(fMfe, fEq, name.c_str(), i);
            
            if (name.length() > 0 && name[0] != '#') {
               KOtcpConnection* s = new KOtcpConnection(name.c_str(), "http");
            
               s->fConnectTimeoutMilliSec = 2*1000;
               s->fReadTimeoutMilliSec = 2*1000;
               s->fWriteTimeoutMilliSec = 2*1000;
               s->fHttpKeepOpen = false;
               
               adc->fEsper = new EsperComm;
               adc->fEsper->s = s;
               countAdc++;
            }
               
            fAdcCtrl.push_back(adc);
         }
      }

      printf("LoadOdb: ADC_MODULES: %d\n", countAdc);

      int countPwb = 0;

      bool enable_pwb = true;

      fEq->fOdbEqSettings->RB("PWB/Enable", 0, &enable_pwb, true);
         
      if (enable_pwb) {
         // check that Init() is not called twice
         assert(fPwbCtrl.size() == 0);
         
         std::vector<std::string> modules;

         const int num_pwb = 64;

         fEq->fOdbEqSettings->RSA("PWB/modules", &modules, true, num_pwb, 32);
         fEq->fOdbEqSettings->RBA("PWB/boot_user_page", NULL, true, num_pwb);

         double to_connect = 2.0;
         double to_read = 10.0;
         double to_write = 2.0;

         fEq->fOdbEqSettings->RD("PWB/connect_timeout", 0, &to_connect, true);
         fEq->fOdbEqSettings->RD("PWB/read_timeout", 0, &to_read, true);
         fEq->fOdbEqSettings->RD("PWB/write_timeout", 0, &to_write, true);

         for (unsigned i=0; i<modules.size(); i++) {
            std::string name = modules[i];
         
            //printf("index %d name [%s]\n", i, name.c_str());
            
            PwbCtrl* pwb = new PwbCtrl(fMfe, fEq, name.c_str(), i);
            
            pwb->fEsper = NULL;
            
            if (name.length() > 0 && name[0] != '#') {
               KOtcpConnection* s = new KOtcpConnection(name.c_str(), "http");
            
               s->fConnectTimeoutMilliSec = to_connect*1000;
               s->fReadTimeoutMilliSec = to_read*1000;
               s->fWriteTimeoutMilliSec = to_write*1000;
               s->fHttpKeepOpen = false;
               
               pwb->fEsper = new EsperComm;
               pwb->fEsper->s = s;

               countPwb++;
            }
            
            fPwbCtrl.push_back(pwb);
         }
      }

      printf("LoadOdb: PWB_MODULES: %d\n", countPwb);
         
      fMfe->Msg(MINFO, "LoadOdb", "Found in ODB: %d TRG, %d ADC, %d PWB modules", countAT, countAdc, countPwb);
   }

   bool StopLocked()
   {
      bool ok = true;

      if (fATctrl) {
         ok &= fATctrl->StopAtLocked();
      }

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i] && fAdcCtrl[i]->fEsper) {
            ok &= fAdcCtrl[i]->StopAdcLocked();
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i] && fPwbCtrl[i]->fEsper) {
            ok &= fPwbCtrl[i]->StopPwbLocked();
         }
      }

      fMfe->Msg(MINFO, "Stop", "Stop ok %d", ok);
      return ok;
   }

   bool SoftTriggerLocked()
   {
      bool ok = true;

      if (fATctrl) {
         ok &= fATctrl->SoftTriggerLocked();
      } else {
         for (unsigned i=0; i<fAdcCtrl.size(); i++) {
            if (fAdcCtrl[i]) {
               ok &= fAdcCtrl[i]->SoftTriggerAdcLocked();
            }
         }
         for (unsigned i=0; i<fPwbCtrl.size(); i++) {
            if (fPwbCtrl[i]) {
               ok &= fPwbCtrl[i]->SoftTriggerPwbLocked();
            }
         }
      }

      return ok;
   }

   void ThreadReadAndCheck()
   {
      int count_at = 0;
      int adc_countOk = 0;
      int adc_countBad = 0;
      int adc_countDead = 0;
      int pwb_countOk = 0;
      int pwb_countBad = 0;
      int pwb_countDead = 0;

      if (fATctrl) {
         if (fATctrl->fOk) {
            count_at += 1;
         }
      }

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i] && fAdcCtrl[i]->fEsper) {
            int state = fAdcCtrl[i]->fState;
            if (state == ST_GOOD) {
               adc_countOk += 1;
            } else if (state == ST_BAD_CHECK) {
               adc_countBad += 1;
            } else {
               adc_countDead += 1;
            }
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i] && fPwbCtrl[i]->fEsper) {
            int state = fPwbCtrl[i]->fState;
            if (state == ST_GOOD) {
               pwb_countOk += 1;
            } else if (state == ST_BAD_CHECK) {
               pwb_countBad += 1;
            } else {
               pwb_countDead += 1;
            }
         }
      }

      {
         LOCK_ODB();
         char buf[256];
         if (adc_countBad == 0 && pwb_countBad == 0) {
            sprintf(buf, "%d TRG, %d ADC Ok, %d PWB Ok", count_at, adc_countOk, pwb_countOk);
            fEq->SetStatus(buf, "#00FF00");
         } else {
            sprintf(buf, "%d TRG, %d/%d/%d ADC, %d/%d/%d PWB (G/B/D)", count_at, adc_countOk, adc_countBad, adc_countDead, pwb_countOk, pwb_countBad, pwb_countDead);
            fEq->SetStatus(buf, "yellow");
         }
      }
   }

   void WriteVariables()
   {
      if (fAdcCtrl.size() > 0) {
         std::vector<double> fpga_temp;
         fpga_temp.resize(fAdcCtrl.size(), 0);
         
         std::vector<double> sensor_temp0;
         sensor_temp0.resize(fAdcCtrl.size(), 0);
         
         std::vector<double> sensor_temp_max;
         sensor_temp_max.resize(fAdcCtrl.size(), 0);
         
         std::vector<double> sensor_temp_min;
         sensor_temp_min.resize(fAdcCtrl.size(), 0);
         
         std::vector<int> adc_state;
         adc_state.resize(fAdcCtrl.size(), 0);

         for (unsigned i=0; i<fAdcCtrl.size(); i++) {
            if (fAdcCtrl[i]) {
               adc_state[i] = fAdcCtrl[i]->fState;
               fpga_temp[i] = fAdcCtrl[i]->fFpgaTemp;
               sensor_temp0[i] = fAdcCtrl[i]->fSensorTemp0;
               sensor_temp_max[i] = fAdcCtrl[i]->fSensorTempMax;
               sensor_temp_min[i] = fAdcCtrl[i]->fSensorTempMin;
            }
         }

         std::vector<double> pwb_http_time;
         pwb_http_time.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_temp_fpga;
         pwb_temp_fpga.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_temp_board;
         pwb_temp_board.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_temp_sca_a;
         pwb_temp_sca_a.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_temp_sca_b;
         pwb_temp_sca_b.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_temp_sca_c;
         pwb_temp_sca_c.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_temp_sca_d;
         pwb_temp_sca_d.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_v_p2;
         pwb_v_p2.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_v_p5;
         pwb_v_p5.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_v_sca12;
         pwb_v_sca12.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_v_sca34;
         pwb_v_sca34.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_i_p2;
         pwb_i_p2.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_i_p5;
         pwb_i_p5.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_i_sca12;
         pwb_i_sca12.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_i_sca34;
         pwb_i_sca34.resize(fPwbCtrl.size(), 0);

         std::vector<int> pwb_state;
         pwb_state.resize(fPwbCtrl.size(), 0);

         for (unsigned i=0; i<fPwbCtrl.size(); i++) {
            if (fPwbCtrl[i]) {
               if (fPwbCtrl[i]->fEsper) {
                  pwb_http_time[i] = fPwbCtrl[i]->fEsper->fMaxHttpTime;
                  fPwbCtrl[i]->fEsper->fMaxHttpTime = 0;
               }

               pwb_state[i] = fPwbCtrl[i]->fState;
               
               pwb_temp_fpga[i] = fPwbCtrl[i]->fTempFpga;
               pwb_temp_board[i] = fPwbCtrl[i]->fTempBoard;

               pwb_temp_sca_a[i] = fPwbCtrl[i]->fTempScaA;
               pwb_temp_sca_b[i] = fPwbCtrl[i]->fTempScaB;
               pwb_temp_sca_c[i] = fPwbCtrl[i]->fTempScaC;
               pwb_temp_sca_d[i] = fPwbCtrl[i]->fTempScaD;

               pwb_v_p2[i] = fPwbCtrl[i]->fVoltP2;
               pwb_v_p5[i] = fPwbCtrl[i]->fVoltP5;
               pwb_v_sca12[i] = fPwbCtrl[i]->fVoltSca12;
               pwb_v_sca34[i] = fPwbCtrl[i]->fVoltSca34;

               pwb_i_p2[i] = fPwbCtrl[i]->fCurrP2;
               pwb_i_p5[i] = fPwbCtrl[i]->fCurrP5;
               pwb_i_sca12[i] = fPwbCtrl[i]->fCurrSca12;
               pwb_i_sca34[i] = fPwbCtrl[i]->fCurrSca34;
            }
         }
               
         fEq->fOdbEqVariables->WIA("adc_state", adc_state);
         WVD("fpga_temp", fpga_temp);
         WVD("sensor_temp0", sensor_temp0);
         WVD("sensor_temp_max", sensor_temp_max);
         WVD("sensor_temp_min", sensor_temp_min);

         fEq->fOdbEqVariables->WIA("pwb_state", pwb_state);
         WVD("pwb_http_time", pwb_http_time);

         WVD("pwb_temp_fpga", pwb_temp_fpga);
         WVD("pwb_temp_board", pwb_temp_board);
         WVD("pwb_temp_sca_a", pwb_temp_sca_a);
         WVD("pwb_temp_sca_b", pwb_temp_sca_b);
         WVD("pwb_temp_sca_c", pwb_temp_sca_c);
         WVD("pwb_temp_sca_d", pwb_temp_sca_d);

         WVD("pwb_v_p2", pwb_v_p2);
         WVD("pwb_v_p5", pwb_v_p5);
         WVD("pwb_v_sca12", pwb_v_sca12);
         WVD("pwb_v_sca34", pwb_v_sca34);

         WVD("pwb_i_p2", pwb_i_p2);
         WVD("pwb_i_p5", pwb_i_p5);
         WVD("pwb_i_sca12", pwb_i_sca12);
         WVD("pwb_i_sca34", pwb_i_sca34);
      }
   }

   void ThreadPeriodic()
   {
      ThreadReadAndCheck();
      WriteVariables();
   }

   std::string HandleRpc(const char* cmd, const char* args)
   {
      fMfe->Msg(MINFO, "HandleRpc", "RPC cmd [%s], args [%s]", cmd, args);
      return "OK";
   }

   void LockAll()
   {
      printf("LockAll...\n");
      
      if (fATctrl) {
         fATctrl->fLock.lock();
      }

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i]) {
            fAdcCtrl[i]->fLock.lock();
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i]) {
            fPwbCtrl[i]->fLock.lock();
         }
      }

      printf("LockAll...done\n");
   }

   void UnlockAll()
   {
      // MUST BE IN EXACT REVERSE ORDER FROM LockAll()

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i]) {
            fPwbCtrl[i]->fLock.unlock();
         }
      }

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i]) {
            fAdcCtrl[i]->fLock.unlock();
         }
      }

      if (fATctrl) {
         fATctrl->fLock.unlock();
      }

      printf("UnlockAll...done\n");
   }

   void WriteEvbConfigLocked()
   {
      const int ts625 =  62500000;
      //const int ts100 = 100000000;
      const int ts125 = 125000000;

      std::vector<std::string> name;
      std::vector<int> type;
      std::vector<int> module;
      std::vector<int> nbanks;
      std::vector<int> tsfreq;

      if (fATctrl) {
         name.push_back(fATctrl->fOdbName);
         type.push_back(1);
         module.push_back(fATctrl->fModule);
         nbanks.push_back(1);
         tsfreq.push_back(ts625);
      }

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i] && fAdcCtrl[i]->fEsper) {
            if (fAdcCtrl[i]->fNumBanks < 1)
               continue;
            if (fAdcCtrl[i]->fModule < 1)
               continue;
            if (fAdcCtrl[i]->fConfAdc16Enable) {
               name.push_back(fAdcCtrl[i]->fOdbName);
               type.push_back(2);
               module.push_back(fAdcCtrl[i]->fModule);
               nbanks.push_back(fAdcCtrl[i]->fNumBanksAdc16);
               tsfreq.push_back(ts125);
            }
            if (fAdcCtrl[i]->fConfAdc32Enable) {
               name.push_back(fAdcCtrl[i]->fOdbName + "/adc32");
               type.push_back(2);
               module.push_back(fAdcCtrl[i]->fModule + 100);
               nbanks.push_back(fAdcCtrl[i]->fNumBanksAdc32);
               tsfreq.push_back(ts125);
            }
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i] && fPwbCtrl[i]->fEsper) {
            if (fPwbCtrl[i]->fNumBanks < 1)
               continue;
            if (fPwbCtrl[i]->fModule < 0)
               continue;
            if (!fConfEnablePwbTrigger)
               continue;
            name.push_back(fPwbCtrl[i]->fOdbName);
            type.push_back(4);
            module.push_back(fPwbCtrl[i]->fModule);
            nbanks.push_back(fPwbCtrl[i]->fNumBanks);
            tsfreq.push_back(ts125);
         }
      }

      gEvbC->WSA("name", name, 32);
      gEvbC->WIA("type", type);
      gEvbC->WIA("module", module);
      gEvbC->WIA("nbanks", nbanks);
      gEvbC->WIA("tsfreq", tsfreq);
   }

   void BeginRun(bool start)
   {
      printf("BeginRun!\n");

      fEq->fOdbEqSettings->RB("Trig/PassThrough", 0, &fConfTrigPassThrough, true);
      fEq->fOdbEqSettings->RB("PWB/Trigger", 0, &fConfEnablePwbTrigger, true);

      LockAll();

      printf("Creating threads!\n");
      std::vector<std::thread*> t;

      if (fATctrl) {
         t.push_back(new std::thread(&AlphaTctrl::BeginRunAtLocked, fATctrl, start, fConfEnablePwbTrigger));
      }

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i]) {
            t.push_back(new std::thread(&AdcCtrl::BeginRunAdcLocked, fAdcCtrl[i], start, !fConfTrigPassThrough));
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i]) {
            t.push_back(new std::thread(&PwbCtrl::BeginRunPwbLocked, fPwbCtrl[i], start, fConfEnablePwbTrigger && !fConfTrigPassThrough));
         }
      }

      printf("Joining threads!\n");
      for (unsigned i=0; i<t.size(); i++) {
         t[i]->join();
         delete t[i];
      }

      printf("Done!\n");

      WriteEvbConfigLocked();

      int num_banks = 0;

      if (fATctrl) {
         num_banks += fATctrl->fNumBanks;
      }

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i]) {
            num_banks += fAdcCtrl[i]->fNumBanks;
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i] && fConfEnablePwbTrigger) {
            num_banks += fPwbCtrl[i]->fNumBanks;
         }
      }

      fEq->fOdbEqVariables->WI("num_banks", num_banks);

      printf("Have %d data banks!\n", num_banks);

      fNumBanks = num_banks;

      if (fATctrl && start) {
         fATctrl->StartAtLocked();
      }

      UnlockAll();
   }

   void HandleBeginRun()
   {
      printf("BeginRun!\n");
      BeginRun(true);
   }

   void HandleEndRun()
   {
      printf("EndRun!\n");
      LockAll();
      StopLocked();
      UnlockAll();
   }

   void StartThreads()
   {
      // ensure threads are only started once
      static bool gOnce = true;
      assert(gOnce == true);
      gOnce = false;

      if (fATctrl) {
         fATctrl->StartThreads();
      }

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i] && fAdcCtrl[i]->fEsper) {
            fAdcCtrl[i]->StartThreadsAdc();
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i] && fPwbCtrl[i]->fEsper) {
            fPwbCtrl[i]->StartThreadsPwb();
         }
      }
   }

   void JoinThreads()
   {
      printf("Ctrl::JoinThreads!\n");

      if (fATctrl)
         fATctrl->JoinThreads();

      printf("Ctrl::JoinThreads: ATctrl done!\n");

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i] && fAdcCtrl[i]->fEsper) {
            fAdcCtrl[i]->JoinThreadsAdc();
         }
      }

      printf("Ctrl::JoinThreads: ADC done!\n");

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i] && fPwbCtrl[i]->fEsper) {
            fPwbCtrl[i]->JoinThreadsPwb();
         }
      }

      printf("Ctrl::JoinThreads: PWB done!\n");

      printf("Ctrl::JoinThread: done!\n");
   }
};

class ThreadTest
{
public:
   int fIndex = 0;
   ThreadTest(int i)
   {
      fIndex = i;
   }

   void Thread(int i)
   {
      printf("class method thread %d, arg %d\n", fIndex, i);
   }
};

void thread_test_function(int i)
{
   printf("thread %d\n", i);
}

void thread_test()
{
   std::vector<ThreadTest*> v;
   printf("Creating objects!\n");
   for (int i=0; i<10; i++) {
      v.push_back(new ThreadTest(i));
   }
   printf("Creating threads!\n");
   std::vector<std::thread*> t;
   for (unsigned i=0; i<v.size(); i++) {
      //t.push_back(new std::thread(thread_test_function, i));
      t.push_back(new std::thread(&ThreadTest::Thread, v[i], 100+i));
   }
   printf("Joining threads!\n");
   for (unsigned i=0; i<t.size(); i++) {
      t[i]->join();
      delete t[i];
      t[i] = NULL;
   }
   printf("Done!\n");
   exit(1);
}

int main(int argc, char* argv[])
{
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   signal(SIGPIPE, SIG_IGN);

   //thread_test();

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
   eqc->Buffer = "BUFUDP";
   
   TMFeEquipment* eq = new TMFeEquipment("CTRL");
   eq->Init(mfe->fOdbRoot, eqc);
   eq->SetStatus("Starting...", "white");

   mfe->RegisterEquipment(eq);

   gEvbC = mfe->fOdbRoot->Chdir(("Equipment/" + eq->fName + "/EvbConfig").c_str(), true);

   Ctrl* ctrl = new Ctrl;

   ctrl->fMfe = mfe;
   ctrl->fEq = eq;

   mfe->RegisterRpcHandler(ctrl);
   mfe->SetTransitionSequence(910, 90, -1, -1);

   ctrl->LoadOdb();

   {
      int run_state = 0;
      mfe->fOdbRoot->RI("Runinfo/State", 0, &run_state, false);
      bool running = (run_state == 3);
      if (running) {
         ctrl->HandleBeginRun();
      } else {
         ctrl->BeginRun(false);
      }
   }

   ctrl->StartThreads();

   printf("init done!\n");

   time_t next_periodic = time(NULL) + 1;

   while (!mfe->fShutdown) {
      time_t now = time(NULL);

      if (now > next_periodic) {
         ctrl->ThreadPeriodic();
         next_periodic += 5;
      }

      {
         std::vector<AtData*> atbuf;

         {
            std::lock_guard<std::mutex> lock(gAtDataBufLock);
            //printf("Have events: %d\n", gAtDataBuf.size());
            for (unsigned i=0; i<gAtDataBuf.size(); i++) {
               atbuf.push_back(gAtDataBuf[i]);
               gAtDataBuf[i] = NULL;
            }
            gAtDataBuf.clear();
         }

         if (atbuf.size() > 0) {
            for (unsigned i=0; i<atbuf.size(); i++) {
               char buf[2560000];
               eq->ComposeEvent(buf, sizeof(buf));

               eq->BkInit(buf, sizeof(buf));
               char* xptr = (char*)eq->BkOpen(buf, "ATAT", TID_DWORD);
               char* ptr = xptr;
               int size = atbuf[i]->size();
               memcpy(ptr, atbuf[i]->data(), size);
               ptr += size;
               eq->BkClose(buf, ptr);

               delete atbuf[i];
               atbuf[i] = NULL;

               eq->SendEvent(buf);
            }

            atbuf.clear();

            eq->WriteStatistics();
         }
      }

      mfe->PollMidas(1000);
      if (mfe->fShutdown)
         break;
   }

   ctrl->JoinThreads();

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
