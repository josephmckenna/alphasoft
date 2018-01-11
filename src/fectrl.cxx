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
#include <mutex>
#include <thread>

#include "tmfe.h"

#include "KOtcp.h"

#include "midas.h"
#include "mjson.h"

#include "tmvodb.h"

static TMVOdb* gOdb = NULL;
static TMVOdb* gS = NULL;
static TMVOdb* gV = NULL;
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
         
void WRS(TMFE*mfe, TMFeEquipment* eq, const char* mod, const char* mid, const char* vid, const std::vector<std::string>& v)
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

   unsigned len = 0;
   for (unsigned i=0; i<v.size(); i++) {
      if (v[i].length() > len) {
         len = v[i].length();
      }
   }

   len += 1; // strings are NUL terminated

   int size = v.size()*len;
   char *ss = new char[size];
   for (unsigned i=0; i<v.size(); i++) {
      strlcpy(ss+i*len, v[i].c_str(), len);
   }
   
   LOCK_ODB();
   
   int status = db_set_value(mfe->fDB, 0, C(path), ss, size, v.size(), TID_STRING);
   if (status != DB_SUCCESS) {
      printf("WR: db_set_value status %d\n", status);
   }
   
   delete ss;
}

int OdbGetInt(TMFE* mfe, const char* path, int default_value, bool create)
{
   LOCK_ODB();

   int v = 0;
   int size = sizeof(v);

   int status = db_get_value(mfe->fDB, 0, path, &v, &size, TID_INT, create);

   if (status != DB_SUCCESS) {
      return default_value;
   }

   return v;
}

std::string OdbGetString(TMFE* mfe, const char* path, int index)
{
   LOCK_ODB();

   std::string s;
   int status = db_get_value_string(mfe->fDB, 0, path, index, &s, FALSE);
   if (status != DB_SUCCESS) {
      return "";
   }
   return s;
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

class Alpha16ctrl
{
public: // settings and configuration
   TMFE* fMfe = NULL;
   TMFeEquipment* fEq = NULL;
   EsperComm* fEsper = NULL;

   std::string fOdbName;
   int fOdbIndex = -1;

   int fModule = 0; // alpha16 rev1 module number 1..20

   bool fVerbose = false;

   int fPollSleep = 10;
   int fFailedSleep = 10;

public: // state and global variables
   std::mutex fLock;

   int fNumBanks = 0;

   bool fOk = true;

   Fault fCheckComm;
   Fault fCheckId;
   Fault fCheckPage;
   Fault fCheckEsata0;
   Fault fCheckEsataLock;
   Fault fCheckPllLock;
   Fault fCheckUdpState;
   Fault fCheckRunState;

public:
   Alpha16ctrl(TMFE* xmfe, TMFeEquipment* xeq, const char* xodbname, int xodbindex)
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
      gOdb->RI("Runinfo/State", 0, &run_state, false);
      gOdb->RI("Runinfo/Transition in progress", 0, &transition_in_progress, false);

      bool running = (run_state == 3);

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
      } else {
         fMfe->Msg(MERROR, "Identify", "%s: firmware is not compatible with the daq, sof fpga_build  0x%08x", fOdbName.c_str(), sof_ts);
         fCheckId.Fail("incompatible firmware, fpga_build: " + fpga_build);
         return false;
      }

      bool boot_from_user_page = false;
      gS->RB("ALPHA16_BOOT_USER_PAGE", fOdbIndex, &boot_from_user_page, false);

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

      int udp_port = OdbGetInt(fMfe, "/Equipment/UDP/Settings/udp_port", 0, false);

      int adc16_samples = OdbGetInt(fMfe, "/Equipment/CTRL/Settings/adc16_samples", 700, true);
      int adc16_trig_delay = OdbGetInt(fMfe, "/Equipment/CTRL/Settings/adc16_trig_delay", 0, true);
      int adc16_trig_start = OdbGetInt(fMfe, "/Equipment/CTRL/Settings/adc16_trig_start", 150, true);

      gS->RB("adc16_enable", fOdbIndex, &fConfAdc16Enable, false);

      int adc32_samples = OdbGetInt(fMfe, "/Equipment/CTRL/Settings/adc32_samples", 511, true);
      int adc32_trig_delay = OdbGetInt(fMfe, "/Equipment/CTRL/Settings/adc32_trig_delay", 0, true);
      int adc32_trig_start = OdbGetInt(fMfe, "/Equipment/CTRL/Settings/adc32_trig_start", 100, true);

      gS->RB("adc32_enable", fOdbIndex, &fConfAdc32Enable, false);

      fMfe->Msg(MINFO, "A16::Configure", "%s: configure: udp_port %d, adc16 enable %d, samples %d, trig_delay %d, trig_start %d, adc32 enable %d, samples %d, trig_delay %d, trig_start %d, module_id 0x%02x", fOdbName.c_str(), udp_port, fConfAdc16Enable, adc16_samples, adc16_trig_delay, adc16_trig_start, fConfAdc32Enable, adc32_samples, adc32_trig_delay, adc32_trig_start, fOdbIndex);

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

      // program module id

      ok &= fEsper->Write(fMfe, "board", "module_id", toString(fOdbIndex).c_str());

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
               fOk = false;
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

            EsperNodeData e;
            bool ok = ReadAdcLocked(&e);
            if (ok) {
               ok = CheckAdcLocked(e);
               if (ok)
                  fOk = true;
            }
            if (!ok)
               fOk = false;
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
      fThread = new std::thread(&Alpha16ctrl::ThreadAdc, this);
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
      if (!fEsper)
         return;
      EsperNodeData e;
      bool ok = ReadAdcLocked(&e);
      if (ok) {
         ok = CheckAdcLocked(e);
      }
   }

   void BeginRunAdcLocked(bool start)
   {
      if (!fEsper)
         return;
      IdentifyAdcLocked();
      ConfigureAdcLocked();
      ReadAndCheckAdcLocked();
      //WriteVariables();
      if (start) {
         StartAdcLocked();
      }
   }
};

class Feam0ctrl
{
public:
   TMFE* mfe = NULL;
   TMFeEquipment* eq = NULL;
   KOtcpConnection* s = NULL;

   std::string fOdbName;
   int fOdbIndex = -1;

   int fModule = 0;

   bool fVerbose = false;

   bool fFailed = false;

   bool fOk = true;

   int fPollSleep = 10;
   int fFailedSleep = 10;

   std::mutex fLock;

   int fNumBanks = 0;

   bool fEnableFeamTrigger = true;

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

   KOtcpError GetModules(std::vector<std::string>* mid)
   {
      std::vector<std::string> headers;
      std::vector<std::string> reply_headers;
      std::string reply_body;

      KOtcpError e = s->HttpGet(headers, "/read_node?includeMods=y", &reply_headers, &reply_body);

      if (e.error) {
         mfe->Msg(MERROR, "GetModules", "HttpGet() error %s", e.message.c_str());
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

   void ReadVariables(const MJsonNode* n, const std::string& mid, EsperModuleData* vars)
   {
      // $json->{"data"}->{"node"}->{"module"}->{"system"}->{"var"}->{$v}->{"d"}->[0];

      //fVerbose = true;

      if (!n)
         return;

      //n->Dump();

      const MJsonNode* v = n->FindObjectNode("var");
      if (v) {
         //v->Dump();
         const MJsonStringVector *vn = v->GetObjectNames();
         const MJsonNodeVector *va = v->GetObjectNodes();
         if (va && vn) {
            for (unsigned i=0; i<va->size(); i++) {
               const MJsonNode* vae = va->at(i);
               if (vae) {
                  std::string vid = vn->at(i);
                  //printf("mid [%s] vid [%s] data:\n", mid.c_str(), vid.c_str());
                  //vae->Dump();
                  const MJsonNode* vaed = vae->FindObjectNode("d");
                  if (vaed) {
                     int type = 0;
                     if (vaed->GetType() == MJSON_ARRAY) {
                        const MJsonNodeVector *da = vaed->GetArray();
                        if (da->size() > 0) {
                           type = da->at(0)->GetType();
                        }
                     }
                     vars->t[vid] = type;
                     if (fVerbose)
                        printf("mid [%s] vid [%s] type %d json value %s\n", mid.c_str(), vid.c_str(), type, vaed->Stringify().c_str());
                     if (type == MJSON_STRING) {
                        std::vector<std::string> val = JsonToStringArray(vaed);
                        if (val.size() == 1)
                           vars->s[vid] = val[0];
                        else
                           vars->sa[vid] = val;
                        WRS(mfe, eq, fOdbName.c_str(), mid.c_str(), vid.c_str(), val);
                        //WR(mid.c_str(), vid.c_str(), vaed->Stringify().c_str());
                     } else if (0 && type == MJSON_INT) {
                        std::vector<int> val = JsonToIntArray(vaed);
                        if (val.size() == 1)
                           vars->i[vid] = val[0];
                        else
                           vars->ia[vid] = val;
                        WRI(mfe, eq, fOdbName.c_str(), mid.c_str(), vid.c_str(), val);
                     } else if (type == MJSON_NUMBER || type == MJSON_INT) {
                        std::vector<double> val = JsonToDoubleArray(vaed);
                        if (val.size() == 1)
                           vars->d[vid] = val[0];
                        else
                           vars->da[vid] = val;
                        WRD(mfe, eq, fOdbName.c_str(), mid.c_str(), vid.c_str(), val);
                     } else if (type == MJSON_BOOL) {
                        std::vector<bool> val = JsonToBoolArray(vaed);
                        if (val.size() == 1)
                           vars->b[vid] = val[0];
                        else
                           vars->ba[vid] = val;
                        WRB(mfe, eq, fOdbName.c_str(), mid.c_str(), vid.c_str(), val);
                     } else {
                        //printf("mid [%s] vid [%s] type %d json value %s\n", mid.c_str(), vid.c_str(), type, vaed->Stringify().c_str());
                        WR(mfe, eq, fOdbName.c_str(), mid.c_str(), vid.c_str(), vaed->Stringify().c_str());
                     }
                     //variables.push_back(vid);
                  }
               }
            }
         }
      }
   }

   bool ReadAll(EsperNodeData* data)
   {
      if (fVerbose)
         printf("Reading %s\n", fOdbName.c_str());

      if (fFailed)
         return false;

      const MJsonNode* n = ReadAll();

      if (!n) {
         return false;
      }

      // $json->{"data"}->{"node"}->{"module"}->{"system"}->{"var"}->{$v}->{"d"}->[0];

      n = n->FindObjectNode("data");
      if (n) {
         n = n->FindObjectNode("node");
         if (n) {
            n = n->FindObjectNode("module");
            if (n) {
               const MJsonStringVector* names = n->GetObjectNames();
               for (unsigned i=0; i<names->size(); i++) {
                  //printf("module [%s]\n", (*names)[i].c_str());
                  ReadVariables(n->FindObjectNode((*names)[i].c_str()), (*names)[i], &(*data)[(*names)[i]]);
               }
            }
         }
      }

      delete n;

      return true;
   }

   bool Write1(const char* mid, const char* vid, char value)
   {
      return Write(mid, vid, &value, 1);
   }

   bool Write4(const char* mid, const char* vid, int value)
   {
      char buf[4];
      buf[3] = (value>>24)&0xFF;
      buf[2] = (value>>16)&0xFF;
      buf[1] = (value>>8)&0xFF;
      buf[0] = (value>>0)&0xFF;
      return Write(mid, vid, buf, 4);
   }

   bool Write(const char* mid, const char* vid, const char* data, int length)
   {
      if (fFailed)
         return false;

      //printf("FEAM write not implemented [%s] [%s] [%d]\n", mid, vid, value);
      //return true;

      std::string url;
      url += "/write_var?";
      url += "binary=y";
      url += "&";
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
      url += "1";
      url += "&";
      url += "ts=";
      url += "99999999";
      //url += "&";
      //url += "wc=";
      //url += "21";

      //printf("URL: %s\n", url.c_str());

      std::vector<std::string> headers;
      std::vector<std::string> reply_headers;
      std::string reply_body;

      KOtcpError e = s->HttpPost(headers, url.c_str(), data, length, &reply_headers, &reply_body);

      if (e.error) {
         mfe->Msg(MERROR, "Write", "HttpPost() error %s", e.message.c_str());
         fFailed = true;
         return false;
      }

#if 0
      printf("reply headers:\n");
      for (unsigned i=0; i<reply_headers.size(); i++)
         printf("%d: %s\n", i, reply_headers[i].c_str());

      //printf("json: %s\n", reply_body.c_str());
#endif

      if (reply_headers.size() < 1) {
         mfe->Msg(MERROR, "Write", "HttpPost() error, no HTTP status reply header");
         //fFailed = true;
         return false;
      }

      if (reply_headers[0].find("200 OK") == std::string::npos) {
         mfe->Msg(MERROR, "Write", "HttpPost() error, HTTP status %s", reply_headers[0].c_str());
         //fFailed = true;
         return false;
      }

      return true;
   }

   std::string XRead(const char* mid, const char* vid, std::string* last_errmsg = NULL)
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

      printf("URL: %s\n", url.c_str());

      std::vector<std::string> headers;
      std::vector<std::string> reply_headers;
      std::string reply_body;

      KOtcpError e = s->HttpGet(headers, url.c_str(), &reply_headers, &reply_body);

      if (e.error) {
         if (!last_errmsg || e.message != *last_errmsg) {
            mfe->Msg(MERROR, "Read", "HttpGet() error %s", e.message.c_str());
            if (last_errmsg) {
               *last_errmsg = e.message;
            }
         }
         fFailed = true;
         return "";
      }

#if 1
      printf("reply headers:\n");
      for (unsigned i=0; i<reply_headers.size(); i++)
         printf("%d: %s\n", i, reply_headers[i].c_str());

      printf("json: %s\n", reply_body.c_str());
#endif

      return reply_body;
   }

   MJsonNode* ReadAll(std::string* last_errmsg = NULL)
   {
      if (fFailed)
         return NULL;

      //wget "http://feam1/read_node?includeModules=y&includeVars=y&dataOnly=y" -O xxx

      std::string url;
      url += "/read_node?includeModules=y&includeVars=y&dataOnly=y";

      //printf("URL: %s\n", url.c_str());

      std::vector<std::string> headers;
      std::vector<std::string> reply_headers;
      std::string reply_body;

      KOtcpError e = s->HttpGet(headers, url.c_str(), &reply_headers, &reply_body);

      if (e.error) {
         if (!last_errmsg || e.message != *last_errmsg) {
            mfe->Msg(MERROR, "Read", "HttpGet() error %s", e.message.c_str());
            if (last_errmsg) {
               *last_errmsg = e.message;
            }
         }
         fFailed = true;
         return NULL;
      }

#if 0
      printf("reply headers:\n");
      for (unsigned i=0; i<reply_headers.size(); i++)
         printf("%d: %s\n", i, reply_headers[i].c_str());

      printf("json: %s\n", reply_body.c_str());
#endif

      return MJsonNode::Parse(reply_body.c_str());
   }

   const MJsonNode* FindNode(const MJsonNode* n, const char* mid, const char* vid)
   {
      // $json->{"data"}->{"node"}->{"module"}->{"system"}->{"var"}->{$v}->{"d"}->[0];
      n = n->FindObjectNode("data");
      if (n == NULL)
         return n;
      n = n->FindObjectNode("node");
      if (n == NULL)
         return n;
      n = n->FindObjectNode("module");
      if (n == NULL)
         return n;
      n = n->FindObjectNode(mid);
      if (n == NULL)
         return n;
      n = n->FindObjectNode("var");
      if (n == NULL)
         return n;
      n = n->FindObjectNode(vid);
      if (n == NULL)
         return n;
      n = n->FindObjectNode("d");
      if (n == NULL)
         return n;
      const MJsonNodeVector*a = n->GetArray();
      if (a == NULL)
         return n;
      n = (*a)[0];
      if (n == NULL)
         return n;
      return n;
   }

   std::string FindString(const MJsonNode* n, const char* mid, const char* vid)
   {
      n = FindNode(n, mid, vid);
      if (n == NULL)
         return "";
      //n->Dump();
      //printf("Found string [%s] [%s] value [%s]\n", mid, vid, n->GetString().c_str());
      return n->GetString();
   }

   double FindDouble(const MJsonNode* n, const char* mid, const char* vid)
   {
      n = FindNode(n, mid, vid);
      if (n == NULL)
         return 0;
      //n->Dump();
      //printf("Found string [%s] [%s] value [%s]\n", mid, vid, n->GetString().c_str());
      return n->GetDouble();
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

   int fUpdateCount = 0;

   double fFpgaTemp = 0;

   bool CheckFeamLocked(EsperNodeData data)
   {
      if (fFailed) {
         printf("%s: failed\n", fOdbName.c_str());
         return false;
      }

      int run_state = 0;
      int transition_in_progress = 0;
      gOdb->RI("Runinfo/State", 0, &run_state, false);
      gOdb->RI("Runinfo/Transition in progress", 0, &transition_in_progress, false);

      bool running = (run_state == 3);

      if (!fEnableFeamTrigger)
         running = false;

      //printf("%s: run_state %d, running %d, transition_in_progress %d\n", fOdbName.c_str(), run_state, running, transition_in_progress);

      bool ok = true;

      int freq_sata = data["board"].d["freq_sata"];
      int freq_sfp  = data["board"].d["freq_sfp"];
      int freq_sca_wr = data["board"].d["freq_sca_wr"];
      int freq_sca_rd = data["board"].d["freq_sca_rd"];

      int plls_locked = data["clockcleaner"].b["plls_locked"];

      bool force_run = data["signalproc"].b["force_run"];
      bool ext_trig_ena = data["board"].b["ext_trig_ena"];
      bool ext_trig_inv = data["board"].b["ext_trig_inv"];

      //bool udp_enable = data["udp"].b["enable"];
      //int  udp_tx_cnt = data["udp"].i["tx_cnt"];

      double fpga_temp = data["board"].d["fpga_temp"];

      printf("%s: fpga temp: %.0f, plls_locked: %d, freq_sata: %d, sfp %d, sca_wr %d, sca_rd %d, run %d, ext_trig %d %d\n",
             fOdbName.c_str(),
             fpga_temp,
             plls_locked,
             freq_sata,
             freq_sfp,
             freq_sca_wr,
             freq_sca_rd,
             force_run,
             ext_trig_ena,
             ext_trig_inv);

#if 0
      if (freq_sata == 0) {
         if (LogOnce("board.freq_sata.missing"))
            mfe->Msg(MERROR, "Check", "FEAM %s: no SATA clock", fOdbName.c_str());
         ok = false;
      } else {
         LogOk("board.freq_sata.missing");
      }

      if (freq_sata != 62500000) {
         if (LogOnce("board.freq_sata.locked"))
            mfe->Msg(MERROR, "Check", "FEAM %s: not locked to SATA clock", fOdbName.c_str());
         ok = false;
      } else {
         LogOk("board.freq_sata.locked");
      }
#endif

      if (!plls_locked) {
         if (LogOnce("clockcleaner.plls_locked"))
            mfe->Msg(MERROR, "Check", "FEAM %s: PLLs not locked", fOdbName.c_str());
         ok = false;
      } else {
         LogOk("clockcleaner.plls_locked");
      }

      if (!transition_in_progress) {
         if (force_run != running) {
            if (LogOnce("board.force_run"))
               mfe->Msg(MERROR, "Check", "FEAM %s: board.force_run is wrong", fOdbName.c_str());
            ok = false;
         } else {
            LogOk("board.force_run");
         }
      }

      fFpgaTemp = fpga_temp;

      fUpdateCount++;

      return ok;
   }

   std::string fLastErrmsg;

   bool IdentifyFeamLocked()
   {
      if (fFailed) {
         fFailed = false;
      } else {
         fLastErrmsg = "";
      }

      fNumBanks = 0;

      MJsonNode* data = ReadAll(&fLastErrmsg);

      if (!data)
         return false;

      std::string elf_build_str = FindString(data, "system", "elf_build_str");
      double elf_buildtime = FindDouble(data, "system", "elf_buildtime");
      double sw_qsys_ts = FindDouble(data, "system", "sw_qsys_ts");
      double hw_qsys_ts = FindDouble(data, "system", "hw_qsys_ts");

      delete data;

#if 0
      if (!elf_buildtime.length() > 0)
         return false;
      if (!sw_qsys_ts.length() > 0)
         return false;
      if (!hw_qsys_ts.length() > 0)
         return false;
#endif

      mfe->Msg(MINFO, "Identify", "FEAMrev0 %s firmware 0x%08x-0x%08x-0x%08x", fOdbName.c_str(), (uint32_t)elf_buildtime, (uint32_t)sw_qsys_ts, (uint32_t)hw_qsys_ts);

      if (elf_buildtime != 0x5927476e) {
         mfe->Msg(MINFO, "Identify", "FEAMrev0 %s firmware is not compatible with the daq", fOdbName.c_str());
         return false;
      }

      const char* s = fOdbName.c_str();
      while (*s && !isdigit(*s)) {
         s++;
      }
      fModule = atoi(s);

      fNumBanks = 256;

      return true;
   }

   bool ConfigureFeamLocked()
   {
      if (fFailed) {
         printf("Configure %s: failed flag\n", fOdbName.c_str());
         return false;
      }

      int feam_trig_delay = OdbGetInt(mfe, "/Equipment/CTRL/Settings/feam_trig_delay", 312, true);
      int feam_sca_gain = OdbGetInt(mfe, "/Equipment/CTRL/Settings/feam_sca_gain", 0, true);

      printf("%s: configure: trig_delay %d, sca gain %d\n", fOdbName.c_str(), feam_trig_delay, feam_sca_gain);

      bool ok = true;

      ok &= StopFeamLocked();

      // make sure everything is stopped

#if 0
      // switch clock to ESATA

      ok &= Write("board", "clk_lmk", "1");
#endif

      // configure the trigger

      ok &= Write4("signalproc", "trig_delay", feam_trig_delay);

      // configure the SCAs

      ok &= Write1("sca0", "gain", feam_sca_gain);
      ok &= Write1("sca1", "gain", feam_sca_gain);
      ok &= Write1("sca2", "gain", feam_sca_gain);
      ok &= Write1("sca3", "gain", feam_sca_gain);

#if 0
      // configure the ADCs

      fNumBanks = 0;

      if (adc16_enable) {
         fNumBanks += 16;
      }
#endif


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

      return ok;
   }

   bool StartFeamLocked()
   {
      bool ok = true;
      ok &= Write1("signalproc", "ext_trig_ena", 1);
      ok &= Write1("signalproc", "force_run", 1);
      return ok;
   }

   bool StopFeamLocked()
   {
      bool ok = true;
      ok &= Write1("signalproc", "force_run", 0);
      ok &= Write1("signalproc", "ext_trig_ena", 0);
      //mfe->Msg(MINFO, "StopFeamLocked", "%s: stopped", fOdbName.c_str());
      return ok;
   }

   bool SoftTrigger()
   {
      //printf("SoftTrigger!\n");
      bool ok = true;
      ok &= Write1("signalproc", "ext_trig_inv", 1);
      ok &= Write1("signalproc", "ext_trig_inv", 0);
      //printf("SoftTrigger done!\n");
      return ok;
   }

   void Thread()
   {
      printf("thread for %s started\n", fOdbName.c_str());
      while (!mfe->fShutdown) {
         if (fFailed) {
            bool ok;
            {
               std::lock_guard<std::mutex> lock(fLock);
               ok = IdentifyFeamLocked();
               // fLock implicit unlock
            }
            if (!ok) {
               fOk = false;
               for (int i=0; i<fFailedSleep; i++) {
                  if (mfe->fShutdown)
                     break;
                  sleep(1);
               }
               continue;
            }
         }

         {
            std::lock_guard<std::mutex> lock(fLock);

            EsperNodeData e;
            bool ok = ReadAll(&e);
            if (ok) {
               ok = CheckFeamLocked(e);
               if (ok)
                  fOk = true;
            }
            if (!ok)
               fOk = false;
         }

         for (int i=0; i<fPollSleep; i++) {
            if (mfe->fShutdown)
               break;
            sleep(1);
         }
      }
      printf("thread for %s shutdown\n", fOdbName.c_str());
   }

   void ReadAndCheckFeamLocked()
   {
      EsperNodeData e;
      bool ok = ReadAll(&e);
      if (ok) {
         ok = CheckFeamLocked(e);
      }
   }

   void BeginRunFeamLocked(bool start, bool enableFeamTrigger)
   {
      if (!s)
         return;
      fEnableFeamTrigger = enableFeamTrigger;
      IdentifyFeamLocked();
      ConfigureFeamLocked();
      ReadAndCheckFeamLocked();
      //WriteVariables();
      if (start && enableFeamTrigger) {
         StartFeamLocked();
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

   bool fOk = true;

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
      gOdb->RI("Runinfo/State", 0, &run_state, false);
      gOdb->RI("Runinfo/Transition in progress", 0, &transition_in_progress, false);

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
      gS->RB("PWB/boot_user_page", fOdbIndex, &boot_from_user_page, false);

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

      gS->RI("PWB/clkin_sel", 0, &clkin_sel, true);
      gS->RI("PWB/trig_delay", 0, &trig_delay, true);
      gS->RI("PWB/sca_gain", 0, &sca_gain, true);

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
               fOk = false;
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

            EsperNodeData e;
            bool ok = ReadPwbLocked(&e);
            if (ok) {
               ok = CheckPwbLocked(e);
               if (ok)
                  fOk = true;
            }
            if (!ok)
               fOk = false;
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
      if (!fEsper)
         return;
      EsperNodeData e;
      bool ok = ReadPwbLocked(&e);
      if (ok) {
         ok = CheckPwbLocked(e);
      }
   }

   void BeginRunPwbLocked(bool start, bool enablePwbTrigger)
   {
      if (!fEsper)
         return;
      fEnablePwbTrigger = enablePwbTrigger;
      IdentifyPwbLocked();
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
   TMVOdb* fS = NULL;
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
   int fConfFeamBusyWidthClk = 62500000; // 1sec


   int fConfSasTrigMask = 0;

   bool fConfTrigPulser = false;
   bool fConfTrigEsataNimGrandOr = false;

   bool fConfTrigAdc16GrandOr = false;
   bool fConfTrigAdc32GrandOr = false;

   bool fConfTrig1ormore = false;
   bool fConfTrig2ormore = false;
   bool fConfTrig3ormore = false;
   bool fConfTrig4ormore = false;

   bool fConfTrigAdc16Coinc = false;

   bool ConfigureAtLocked(bool enablePwbTrigger)
   {
      if (fComm->fFailed) {
         printf("Configure %s: no communication\n", fOdbName.c_str());
         return false;
      }

      //gS->RB("CosmicEnable",   0, &fConfCosmicEnable, true);
      gS->RB("SwPulserEnable", 0, &fConfSwPulserEnable, true);
      gS->RD("SwPulserFreq",   0, &fConfSwPulserFreq, true);
      gS->RI("SyncCount",      0, &fConfSyncCount, true);
      if (enablePwbTrigger)
         gS->RD("PwbSyncPeriodSec",  0, &fConfSyncPeriodSec, true);
      else
         gS->RD("SyncPeriodSec",  0, &fConfSyncPeriodSec, true);

      fS->RD("Pulser/ClockFreqHz",     0, &fConfPulserClockFreq, true);
      fS->RI("Pulser/PulseWidthClk",   0, &fConfPulserWidthClk, true);
      fS->RI("Pulser/PulsePeriodClk",  0, &fConfPulserPeriodClk, true);
      fS->RD("Pulser/PulseFreqHz",     0, &fConfPulserFreq, true);
      fS->RB("Pulser/Enable",          0, &fConfRunPulser, true);
      fS->RB("Pulser/OutputEnable",    0, &fConfOutputPulser, true);

      gS->RI("TrigWidthClk",  0, &fConfTrigWidthClk, true);
      gS->RI("A16BusyWidthClk",  0, &fConfA16BusyWidthClk, true);
      gS->RI("FeamBusyWidthClk",  0, &fConfFeamBusyWidthClk, true);
      gS->RI("BusyWidthClk",  0, &fConfBusyWidthClk, true);


      gS->RB("TrigSrc/TrigPulser",  0, &fConfTrigPulser, true);
      gS->RB("TrigSrc/TrigEsataNimGrandOr",  0, &fConfTrigEsataNimGrandOr, true);

      gS->RB("TrigSrc/TrigAdc16GrandOr",  0, &fConfTrigAdc16GrandOr, true);
      gS->RB("TrigSrc/TrigAdc32GrandOr",  0, &fConfTrigAdc32GrandOr, true);

      gS->RB("TrigSrc/Trig1ormore",  0, &fConfTrig1ormore, true);
      gS->RB("TrigSrc/Trig2ormore",  0, &fConfTrig2ormore, true);
      gS->RB("TrigSrc/Trig3ormore",  0, &fConfTrig3ormore, true);
      gS->RB("TrigSrc/Trig4ormore",  0, &fConfTrig4ormore, true);

      gS->RB("TrigSrc/TrigAdc16Coinc",  0, &fConfTrigAdc16Coinc, true);

      gS->RI("SasTrigMask",  0, &fConfSasTrigMask, true);

      bool ok = true;

      ok &= Stop();

      fComm->write_param(0x25, 0xFFFF, 0); // disable all triggers
      fComm->write_param(0x08, 0xFFFF, AlphaTPacket::kPacketSize-2*4); // AT packet size in bytes minus the last 0xExxxxxxx word

      fMfe->Msg(MINFO, "Configure", "%s: enablePwbTrigger: %d", fOdbName.c_str(), enablePwbTrigger);

      fComm->write_param(0x20, 0xFFFF, fConfTrigWidthClk);

      if (enablePwbTrigger) {
         fComm->write_param(0x21, 0xFFFF, fConfFeamBusyWidthClk);
         fMfe->Msg(MINFO, "Configure", "%s: using pwb busy %d", fOdbName.c_str(), fConfFeamBusyWidthClk);
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

      fComm->write_param(0x26, 0xFFFF, fConfSasTrigMask);

      // ADC16 masks

      std::vector<int> adc16_mask;
      gS->RIA("adc16_mask", &adc16_mask, true, 16);

      for (int i=0; i<8; i++) {
         int m0 = adc16_mask[2*i+0];
         int m1 = adc16_mask[2*i+1];
         uint32_t m = ((0xFFFF&((uint32_t)m1))<<16)|(0xFFFF&(uint32_t)m0);
         fComm->write_param(0x200+i, 0xFFFF, m);
      }

      // ADC32 masks

      std::vector<int> adc32_mask;
      gS->RIA("adc32_mask", &adc32_mask, true, 16);

      for (int i=0; i<16; i++) {
         fComm->write_param(0x300+i, 0xFFFF, adc32_mask[i]);
      }

      ReadSasBitsLocked();

      return ok;
   }

   bool fRunning = false;
   int  fSyncPulses = 0;
   double fSyncPeriodSec = 0;

   bool Start()
   {
      bool ok = true;

      fSyncPulses = fConfSyncCount;
      fSyncPeriodSec = fConfSyncPeriodSec;

      fRunning = false;

      fComm->write_param(0x25, 0xFFFF, 0); // disable all triggers

      if (fSyncPulses) {
         fComm->write_param(0x25, 0xFFFF, 1<<0); // enable software trigger
      }

      fComm->write_drq(); // request udp packet data

      return ok;
   }

   bool Stop()
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
         fMfe->Msg(MINFO, "ReadSasBits", "%s: link %2d sas_bits: 0x%08x%08x", fOdbName.c_str(), i, v1, v0);
      }
   }

   double fScPrevTime = 0;
   uint32_t fScPrevClk = 0;
   std::vector<int> fScPrev;

   void ReadScalers()
   {
      const int N = 16;
      std::vector<int> sc;

      while (fScPrev.size() < N) {
         fScPrev.push_back(0);
      }

      double t = 0;

      const int iclk = 0;

      {
         std::lock_guard<std::mutex> lock(fLock);

         fComm->write_param(0x2B, 0xFFFF, 0); // write conf_latch

         for (int i=0; i<N; i++) {
            uint32_t v;
            fComm->read_param(0x100+i, 0xFFFF, &v);
            if (i==0) {
               t = TMFE::GetTime();
            }
            sc.push_back(v);
         }
      }

      uint32_t clk = sc[iclk];

      uint32_t dclk = clk - fScPrevClk;
      double dclk_sec = dclk/(62.5*1e6);

      //printf("clk 0x%08x -> 0x%08x, dclk 0x%08x, time %f sec\n", fScPrevClk, clk, dclk, dclk_sec);

      gV->WIA("scalers", sc);

      printf("scalers: ");
      for (int i=0; i<N; i++) {
         printf(" 0x%08x", sc[i]);
      }
      printf("\n");

      double dead_time = 0;

      if (fScPrevTime) {
         std::vector<double> rate;
         //double dt = t - fScPrevTime;
         double dt = dclk_sec;
      
         for (int i=0; i<N; i++) {
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

         gV->WD("trigger_dead_time", dead_time);
         gV->WDA("scalers_rate", rate);
      }

      fScPrevTime = t;
      fScPrevClk = clk;
      for (int i=0; i<N; i++) {
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

            p.Print();
            uint32_t ts = p.ts_625;
            
            if (ts < tsprev) {
               epoch++;
            }
            
            double t = ts/clk625 + epoch*2.0*(0x80000000/clk625);
            double dt = t-tprev;
            
            tprev = t;
            tsprev = p.ts_625;
            
            printf(", epoch %d, time %f, dt %f\n", epoch, t, dt);
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
   TMFE* mfe = NULL;
   TMFeEquipment* eq = NULL;

   TMVOdb* fS = NULL;

   AlphaTctrl* fATctrl = NULL;
   std::vector<Alpha16ctrl*> fA16ctrl;
   std::vector<Feam0ctrl*> fFeam0ctrl;
   std::vector<PwbCtrl*> fPwbCtrl;

   bool fConfEnablePwbTrigger = true;

   int fNumBanks = 0;

   void WVD(const char* name, const std::vector<double> &v)
   {
      if (mfe->fShutdown)
         return;
      
      std::string path;
      path += "/Equipment/";
      path += eq->fName;
      path += "/Variables/";
      path += name;

      LOCK_ODB();

      //printf("Write ODB %s Variables %s: %s\n", C(path), name, v);
      int status = db_set_value(mfe->fDB, 0, C(path), &v[0], sizeof(v[0])*v.size(), v.size(), TID_DOUBLE);
      if (status != DB_SUCCESS) {
         printf("WVD: db_set_value status %d\n", status);
      }
   };

   void WVI(const char* name, const std::vector<int> &v)
   {
      if (mfe->fShutdown)
         return;
      
      std::string path;
      path += "/Equipment/";
      path += eq->fName;
      path += "/Variables/";
      path += name;

      LOCK_ODB();

      //printf("Write ODB %s Variables %s, array size %d\n", C(path), name, (int)v.size());
      int status = db_set_value(mfe->fDB, 0, C(path), &v[0], sizeof(v[0])*v.size(), v.size(), TID_INT);
      if (status != DB_SUCCESS) {
         printf("WVI: db_set_value status %d\n", status);
      }
   };

   void LoadOdb(TMVOdb* odbs)
   {
      // check that LoadOdb() is not called twice
      assert(fATctrl == NULL);

      int countAT = 0;

      bool enable_at = true;

      odbs->RB("ALPHAT_Enable", 0, &enable_at, true);

      if (enable_at) {
         std::string name;
         odbs->RS("ALPHAT_MODULES", 0, &name, true);

         if (name[0] != '#') {
            AlphaTctrl* at = new AlphaTctrl(mfe, eq, name.c_str(), name.c_str());
            fATctrl = at;
            fATctrl->fS = odbs;
            countAT++;
         }
      }

      printf("LoadOdb: ALPHAT_MODULES: %d\n", countAT);

      // check that Init() is not called twice
      assert(fA16ctrl.size() == 0);

      bool enable_a16 = true;

      odbs->RB("ALPHA16_enable", 0, &enable_a16, true);

      int countA16 = 0;
         
      if (enable_a16) {
         std::vector<std::string> modules;

         odbs->RSA("ALPHA16_MODULES", &modules, true, 16, 32);
         odbs->RBA("ALPHA16_BOOT_USER_PAGE", NULL, true, 16);
         odbs->RBA("adc16_enable", NULL, true, 16);
         odbs->RBA("adc32_enable", NULL, true, 16);

         for (unsigned i=0; i<modules.size(); i++) {
            std::string name = modules[i];
            
            //printf("index %d name [%s]\n", i, name.c_str());

            Alpha16ctrl* a16 = new Alpha16ctrl(mfe, eq, name.c_str(), i);
            
            if (name.length() > 0 && name[0] != '#') {
               KOtcpConnection* s = new KOtcpConnection(name.c_str(), "http");
            
               s->fConnectTimeoutMilliSec = 2*1000;
               s->fReadTimeoutMilliSec = 2*1000;
               s->fWriteTimeoutMilliSec = 2*1000;
               s->fHttpKeepOpen = false;
               
               a16->fEsper = new EsperComm;
               a16->fEsper->s = s;
               countA16++;
            }
               
            fA16ctrl.push_back(a16);
         }
      }

      printf("LoadOdb: ALPHA16_MODULES: %d\n", countA16);

      int countFeam = 0;

      bool enable_pwb = true;

      gS->RB("PWB/Enable", 0, &enable_pwb, true);
         
      if (enable_pwb) {
         // check that Init() is not called twice
         assert(fFeam0ctrl.size() == 0);
         
         std::vector<std::string> modules;

         odbs->RSA("FEAM0_MODULES", &modules, true, 10, 32);

         for (unsigned i=0; i<modules.size(); i++) {
            std::string name = modules[i];
         
            //printf("index %d name [%s]\n", i, name.c_str());
            
            Feam0ctrl* feam = new Feam0ctrl;
            
            feam->mfe = mfe;
            feam->eq = eq;
            feam->s = NULL;
            feam->fOdbName = name;
            feam->fOdbIndex = i;
            
            if (name.length() > 0 && name[0] != '#') {
               KOtcpConnection* s = new KOtcpConnection(name.c_str(), "http");
            
               s->fConnectTimeoutMilliSec = 2*1000;
               s->fReadTimeoutMilliSec = 2*1000;
               s->fWriteTimeoutMilliSec = 2*1000;
               s->fHttpKeepOpen = false;

               feam->s = s;
               feam->fOk = true;

               countFeam++;
            }
            
            fFeam0ctrl.push_back(feam);
         }
      }

      if (enable_pwb) {
         // check that Init() is not called twice
         assert(fPwbCtrl.size() == 0);
         
         std::vector<std::string> modules;

         const int num_pwb = 64;

         odbs->RSA("PWB/modules", &modules, true, num_pwb, 32);
         odbs->RBA("PWB/boot_user_page", NULL, true, num_pwb);

         double to_connect = 2.0;
         double to_read = 10.0;
         double to_write = 2.0;

         odbs->RD("PWB/connect_timeout", 0, &to_connect, true);
         odbs->RD("PWB/read_timeout", 0, &to_read, true);
         odbs->RD("PWB/write_timeout", 0, &to_write, true);

         for (unsigned i=0; i<modules.size(); i++) {
            std::string name = modules[i];
         
            //printf("index %d name [%s]\n", i, name.c_str());
            
            PwbCtrl* feam = new PwbCtrl(mfe, eq, name.c_str(), i);
            
            feam->fEsper = NULL;
            
            if (name.length() > 0 && name[0] != '#') {
               KOtcpConnection* s = new KOtcpConnection(name.c_str(), "http");
            
               s->fConnectTimeoutMilliSec = to_connect*1000;
               s->fReadTimeoutMilliSec = to_read*1000;
               s->fWriteTimeoutMilliSec = to_write*1000;
               s->fHttpKeepOpen = false;
               
               feam->fEsper = new EsperComm;
               feam->fEsper->s = s;
            
               feam->fOk = true;
               countFeam++;
            }
            
            fPwbCtrl.push_back(feam);
         }
      }

      printf("LoadOdb: FEAM_MODULES: %d\n", countFeam);
         
      mfe->Msg(MINFO, "LoadOdb", "Found in ODB: %d ALPHAT, %d ALPHA16, %d FEAM modules", countAT, countA16, countFeam);
   }

   bool StopLocked()
   {
      bool ok = true;

      if (fATctrl) {
         ok &= fATctrl->Stop();
      }

      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i] && fA16ctrl[i]->fEsper) {
            ok &= fA16ctrl[i]->StopAdcLocked();
         }
      }

      for (unsigned i=0; i<fFeam0ctrl.size(); i++) {
         if (fFeam0ctrl[i] && fFeam0ctrl[i]->s) {
            ok &= fFeam0ctrl[i]->StopFeamLocked();
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i] && fPwbCtrl[i]->fEsper) {
            ok &= fPwbCtrl[i]->StopPwbLocked();
         }
      }

      mfe->Msg(MINFO, "Stop", "Stop ok %d", ok);
      return ok;
   }

   bool SoftTriggerLocked()
   {
      bool ok = true;

      if (fATctrl) {
         ok &= fATctrl->SoftTriggerLocked();
      } else {
         for (unsigned i=0; i<fA16ctrl.size(); i++) {
            if (fA16ctrl[i]) {
               ok &= fA16ctrl[i]->SoftTriggerAdcLocked();
            }
         }
         for (unsigned i=0; i<fFeam0ctrl.size(); i++) {
            if (fFeam0ctrl[i]) {
               ok &= fFeam0ctrl[i]->SoftTrigger();
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
      int a16_countOk = 0;
      int a16_countBad = 0;
      int a16_countDead = 0;
      int feam_countOk = 0;
      int feam_countBad = 0;
      int feam_countDead = 0;

      if (fATctrl) {
         if (fATctrl->fOk) {
            count_at += 1;
         }
      }

      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i] && fA16ctrl[i]->fEsper) {
            bool ok = fA16ctrl[i]->fOk;
            if (ok) {
               a16_countOk += 1;
            } else if (fA16ctrl[i]->fEsper->fFailed || fA16ctrl[i]->fCheckId.fFailed) {
               a16_countDead += 1;
            } else {
               a16_countBad += 1;
            }
         }
      }

      for (unsigned i=0; i<fFeam0ctrl.size(); i++) {
         if (fFeam0ctrl[i] && fFeam0ctrl[i]->s) {
            bool ok = fFeam0ctrl[i]->fOk;
            if (ok) {
               feam_countOk += 1;
            } else if (fFeam0ctrl[i]->fFailed) {
               feam_countDead += 1;
            } else {
               feam_countBad += 1;
            }
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i] && fPwbCtrl[i]->fEsper) {
            bool ok = fPwbCtrl[i]->fOk;
            if (ok) {
               feam_countOk += 1;
            } else if (fPwbCtrl[i]->fEsper->fFailed) {
               feam_countDead += 1;
            } else {
               feam_countBad += 1;
            }
         }
      }

      {
         LOCK_ODB();
         char buf[256];
         if (a16_countBad == 0 && feam_countBad == 0) {
            sprintf(buf, "%d AT, %d A16 Ok, %d FEAM Ok, %d banks", count_at, a16_countOk, feam_countOk, fNumBanks);
            eq->SetStatus(buf, "#00FF00");
         } else {
            sprintf(buf, "%d AT, %d/%d/%d A16, %d/%d/%d FEAM (G/B/D), %d banks", count_at, a16_countOk, a16_countBad, a16_countDead, feam_countOk, feam_countBad, feam_countDead, fNumBanks);
            eq->SetStatus(buf, "yellow");
         }
      }
   }

   void WriteVariables()
   {
      if (fA16ctrl.size() > 0) {
         std::vector<double> fpga_temp;
         fpga_temp.resize(fA16ctrl.size(), 0);
         
         std::vector<double> sensor_temp0;
         sensor_temp0.resize(fA16ctrl.size(), 0);
         
         std::vector<double> sensor_temp_max;
         sensor_temp_max.resize(fA16ctrl.size(), 0);
         
         std::vector<double> sensor_temp_min;
         sensor_temp_min.resize(fA16ctrl.size(), 0);
         
         for (unsigned i=0; i<fA16ctrl.size(); i++) {
            if (fA16ctrl[i]) {
               fpga_temp[i] = fA16ctrl[i]->fFpgaTemp;
               sensor_temp0[i] = fA16ctrl[i]->fSensorTemp0;
               sensor_temp_max[i] = fA16ctrl[i]->fSensorTempMax;
               sensor_temp_min[i] = fA16ctrl[i]->fSensorTempMin;
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

         for (unsigned i=0; i<fPwbCtrl.size(); i++) {
            if (fPwbCtrl[i]) {
               if (fPwbCtrl[i]->fEsper) {
                  pwb_http_time[i] = fPwbCtrl[i]->fEsper->fMaxHttpTime;
                  fPwbCtrl[i]->fEsper->fMaxHttpTime = 0;
               }
               
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
               
         WVD("fpga_temp", fpga_temp);
         WVD("sensor_temp0", sensor_temp0);
         WVD("sensor_temp_max", sensor_temp_max);
         WVD("sensor_temp_min", sensor_temp_min);

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
      mfe->Msg(MINFO, "HandleRpc", "RPC cmd [%s], args [%s]", cmd, args);
      return "OK";
   }

   void LockAll()
   {
      printf("LockAll...\n");
      
      if (fATctrl) {
         fATctrl->fLock.lock();
      }

      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i]) {
            fA16ctrl[i]->fLock.lock();
         }
      }

      for (unsigned i=0; i<fFeam0ctrl.size(); i++) {
         if (fFeam0ctrl[i]) {
            fFeam0ctrl[i]->fLock.lock();
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

      for (unsigned i=0; i<fFeam0ctrl.size(); i++) {
         if (fFeam0ctrl[i]) {
            fFeam0ctrl[i]->fLock.unlock();
         }
      }

      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i]) {
            fA16ctrl[i]->fLock.unlock();
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

      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i] && fA16ctrl[i]->fEsper) {
            if (fA16ctrl[i]->fNumBanks < 1)
               continue;
            if (fA16ctrl[i]->fModule < 1)
               continue;
            if (fA16ctrl[i]->fConfAdc16Enable) {
               name.push_back(fA16ctrl[i]->fOdbName);
               type.push_back(2);
               module.push_back(fA16ctrl[i]->fModule);
               nbanks.push_back(fA16ctrl[i]->fNumBanksAdc16);
               tsfreq.push_back(ts125);
            }
            if (fA16ctrl[i]->fConfAdc32Enable) {
               name.push_back(fA16ctrl[i]->fOdbName + "/adc32");
               type.push_back(2);
               module.push_back(fA16ctrl[i]->fModule + 100);
               nbanks.push_back(fA16ctrl[i]->fNumBanksAdc32);
               tsfreq.push_back(ts125);
            }
         }
      }

      for (unsigned i=0; i<fFeam0ctrl.size(); i++) {
         if (fFeam0ctrl[i] && fFeam0ctrl[i]->s) {
            if (fFeam0ctrl[i]->fNumBanks < 1)
               continue;
            if (fFeam0ctrl[i]->fModule < 1)
               continue;
            if (!fConfEnablePwbTrigger)
               continue;
            name.push_back(fFeam0ctrl[i]->fOdbName);
            type.push_back(3);
            module.push_back(fFeam0ctrl[i]->fModule);
            nbanks.push_back(fFeam0ctrl[i]->fNumBanks);
            tsfreq.push_back(ts125);
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

      fS->RB("PWB/Trigger", 0, &fConfEnablePwbTrigger, true);

      LockAll();

      printf("Creating threads!\n");
      std::vector<std::thread*> t;

      if (fATctrl) {
         t.push_back(new std::thread(&AlphaTctrl::BeginRunAtLocked, fATctrl, start, fConfEnablePwbTrigger));
      }

      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i]) {
            t.push_back(new std::thread(&Alpha16ctrl::BeginRunAdcLocked, fA16ctrl[i], start));
         }
      }

      for (unsigned i=0; i<fFeam0ctrl.size(); i++) {
         if (fFeam0ctrl[i]) {
            t.push_back(new std::thread(&Feam0ctrl::BeginRunFeamLocked, fFeam0ctrl[i], start, fConfEnablePwbTrigger));
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i]) {
            t.push_back(new std::thread(&PwbCtrl::BeginRunPwbLocked, fPwbCtrl[i], start, fConfEnablePwbTrigger));
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

      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i]) {
            num_banks += fA16ctrl[i]->fNumBanks;
         }
      }

      for (unsigned i=0; i<fFeam0ctrl.size(); i++) {
         if (fFeam0ctrl[i] && fConfEnablePwbTrigger) {
            num_banks += fFeam0ctrl[i]->fNumBanks;
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i] && fConfEnablePwbTrigger) {
            num_banks += fPwbCtrl[i]->fNumBanks;
         }
      }

      gV->WI("num_banks", num_banks);

      printf("Have %d data banks!\n", num_banks);

      fNumBanks = num_banks;

      if (fATctrl && start) {
         fATctrl->Start();
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

      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i] && fA16ctrl[i]->fEsper) {
            fA16ctrl[i]->StartThreadsAdc();
         }
      }

      for (unsigned i=0; i<fFeam0ctrl.size(); i++) {
         if (fFeam0ctrl[i] && fFeam0ctrl[i]->s) {
            std::thread * t = new std::thread(&Feam0ctrl::Thread, fFeam0ctrl[i]);
            t->detach();
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

      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i] && fA16ctrl[i]->fEsper) {
            fA16ctrl[i]->JoinThreadsAdc();
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
   eq->Init(eqc);
   eq->SetStatus("Starting...", "white");

   mfe->RegisterEquipment(eq);

   gOdb = MakeOdb(mfe->fDB);
   gS = gOdb->Chdir(("Equipment/" + eq->fName + "/Settings").c_str(), true);
   gV = gOdb->Chdir(("Equipment/" + eq->fName + "/Variables").c_str(), true);
   gEvbC = gOdb->Chdir(("Equipment/" + eq->fName + "/EvbConfig").c_str(), true);

   Ctrl* ctrl = new Ctrl;

   ctrl->mfe = mfe;
   ctrl->eq = eq;
   ctrl->fS = gS;

   mfe->RegisterRpcHandler(ctrl);
   mfe->SetTransitionSequence(910, 90, -1, -1);

   ctrl->LoadOdb(gS);

   {
      int run_state = OdbGetInt(mfe, "/Runinfo/State", 0, false);
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
