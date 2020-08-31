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
#include <math.h> // fabs()

#include <vector>
#include <map>
#include <mutex>
#include <thread>

#include <stdexcept> // std::out_of_range

#include "tmfe.h"

#include "KOtcp.h"

#include "midas.h"
#include "mjson.h"

static MVOdb* gEvbC = NULL;
static MVOdb* gPwbState = NULL;

static std::mutex gOdbLock;
#define LOCK_ODB() std::lock_guard<std::mutex> lock(gOdbLock)
//#define LOCK_ODB() TMFE_LOCK_MIDAS(mfe)
static double gBeginRunStartThreadsTime = 0;

#define C(x) ((x).c_str())

static int iabs(int v)
{
   if (v>=0)
      return v;
   else
      return -v;
}

static std::string boolToString(bool value)
{
   if (value)
      return "true";
   else
      return "false";
}

static std::string doubleToString(const char* fmt, double value)
{
   char buf[256];
   sprintf(buf, fmt, value);
   return buf;
}

static std::string toString(int value)
{
   char buf[256];
   sprintf(buf, "%d", value);
   return buf;
}

static std::string toHexString(int value)
{
   char buf[256];
   sprintf(buf, "0x%x", value);
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
   if (mfe->fShutdownRequested)
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
   if (mfe->fShutdownRequested)
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
   if (mfe->fShutdownRequested)
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
   if (mfe->fShutdownRequested)
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
   
   delete[] bb;
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
   std::string fName;
   KOtcpConnection* s = NULL;
   bool fFailed = false;
   std::string fFailedMessage;
   bool fVerbose = false;

public:
   double fLastHttpTime = 0;
   double fMaxHttpTime = 0;

public:
   EsperComm(const char* name, KOtcpConnection* tcp)
   {
      fName = name;
      s = tcp;
   }

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
         char msg[1024];
         sprintf(msg, "GetModules() error: HttpGet(read_node) error %s", e.message.c_str());
         mfe->Msg(MERROR, "GetModules", "%s: %s", fName.c_str(), msg);
         fFailed = true;
         fFailedMessage = msg;
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
         char msg[1024];
         sprintf(msg, "ReadVariables() error: HttpGet(read_module %s) error %s", mid.c_str(), e.message.c_str());
         mfe->Msg(MERROR, "ReadVariables", "%s: %s", fName.c_str(), msg);
         fFailed = true;
         fFailedMessage = msg;
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

   bool Write(TMFE* mfe, const char* mid, const char* vid, const char* json, bool binaryn=false, bool no_reply=false)
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

      if (no_reply) {
         KOtcpError e = s->HttpPost(headers, url.c_str(), json, NULL, NULL);

         if (e.error) {
            char msg[1024];
            sprintf(msg, "Write() error: HttpPost(write_var %s.%s) error %s", mid, vid, e.message.c_str());
            mfe->Msg(MERROR, "Write", "%s: %s", fName.c_str(), msg);
            fFailed = true;
            fFailedMessage = msg;
            return false;
         }

         return true;
      }

      std::vector<std::string> reply_headers;
      std::string reply_body;

      double t0 = mfe->GetTime();

      KOtcpError e = s->HttpPost(headers, url.c_str(), json, &reply_headers, &reply_body);

      double t1 = mfe->GetTime();
      fLastHttpTime = t1-t0;
      if (fLastHttpTime > fMaxHttpTime)
         fMaxHttpTime = fLastHttpTime;

      if (e.error) {
         char msg[1024];
         sprintf(msg, "Write() error: HttpPost(write_var %s.%s) error %s", mid, vid, e.message.c_str());
         mfe->Msg(MERROR, "Write", "%s: %s", fName.c_str(), msg);
         fFailed = true;
         fFailedMessage = msg;
         return false;
      }

      if (reply_body.find("error") != std::string::npos) {
         mfe->Msg(MERROR, "Write", "%s: AJAX write %s.%s value \"%s\" error: %s", fName.c_str(), mid, vid, json, reply_body.c_str());
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
         char msg[1024];
         sprintf(msg, "Read %s.%s HttpGet() error %s", mid, vid, e.message.c_str());
         if (!last_errmsg || e.message != *last_errmsg) {
            mfe->Msg(MERROR, "Read", "%s: %s", fName.c_str(), msg);
            if (last_errmsg) {
               *last_errmsg = e.message;
            }
         }
         fFailed = true;
         fFailedMessage = msg;
         return "";
      }

#if 0
      printf("reply headers:\n");
      for (unsigned i=0; i<reply_headers.size(); i++)
         printf("%d: %s\n", i, reply_headers[i].c_str());

      printf("json: %s\n", reply_body.c_str());
#endif

#if 0
      if (strcmp(mid, "board") == 0) {
         printf("mid %s, vid %s, json: %s\n", mid, vid, reply_body.c_str());
      }
#endif

      if (reply_body.length()>0) {
         if (reply_body[0] == '{') {
            if (reply_body.find("{\"error")==0) {
               mfe->Msg(MERROR, "Read", "%s: Read %s.%s esper error %s", fName.c_str(), mid, vid, reply_body.c_str());
               return "";
            }
         }
      }

      return reply_body;
   }
};

class Fault
{
public: // state
   TMFE* fMfe = NULL;
   TMFeEquipment* fEq = NULL;
   std::string fModName;
   std::string fFaultName;
   bool fFailed = false;
   std::string fMessage;
   int fPreFailCount = 0;
   int fConfPreFailCount = 0;

public: // constructor

   void WriteOdb(const char* text)
   {
      // empty ODB slots have no module name, so nothing to write to odb
      if (fModName.length() < 1)
         return;

      std::string path;
      path += "Faults";
      path += "/";
      path += fModName;
      path += "/";
      path += fFaultName;

      //printf("Fault::WriteOdb: path [%s], text [%s]\n", path.c_str(), text);

      fEq->fOdbEq->WS(path.c_str(), text);
   }

   void Setup(TMFE* mfe, TMFeEquipment* eq, const char* mod_name, const char* fault_name, int pre_fail_count = 0)
   {
      fMfe = mfe;
      fEq  = eq;
      fModName = mod_name;
      fFaultName = fault_name;
      fConfPreFailCount = pre_fail_count;

      WriteOdb("");
   }

public: //operations
   void Fail(const std::string& message, bool once = false)
   {
      assert(fMfe);
      bool domsg = false;
      if (!fFailed) {
         fPreFailCount++;
         //fMfe->Msg(MERROR, "Check", "%s: Fault: %s: %s, count %d", fModName.c_str(), fFaultName.c_str(), message.c_str(), fPreFailCount);
         if (fPreFailCount > fConfPreFailCount) {
            fFailed = true;
            domsg = true;
         }
      }

      if (fFailed) {
         if (!once) {
            // if failure message has changed, report it!
            if (message != fMessage) {
               domsg = true;
            }
         }
         if (domsg) {
            fMfe->Msg(MERROR, "Check", "%s: Fault: %s: %s", fModName.c_str(), fFaultName.c_str(), message.c_str());
            WriteOdb(message.c_str());
            fMessage = message;
         }
      }
   }

   void Ok()
   {
      assert(fMfe);
      if (fFailed) {
         fMfe->Msg(MINFO, "Fault::Ok", "%s: Fault ok now: %s", fModName.c_str(), fFaultName.c_str());
         WriteOdb("");
         fFailed = false;
         fMessage = "";
      }
      fPreFailCount = 0;
   }
};

#define ST_EMPTY_SLOT_F      0 // empty slot
#define ST_INITIAL          10 // initial state
#define ST_SLOW_PING        20 // board never seen
#define ST_INIT             30 // initialize
#define ST_REBOOT           40 // reboot requested
#define ST_REBOOTING        50 // rebooting to user page firmware
#define ST_CONFIGURE        60 // configure requested
#define ST_WAIT_SLAVE       65 // wait for sata slave to configure
#define ST_FIRST_READ       70 // first read
#define ST_READ             80 // read and check
#define ST_BAD_READ          4 //  90 // read failure
#define ST_BAD_CHECK         5 // 100 // check failed
#define ST_GOOD              1 // 110 // ready to run
#define ST_FAST_PING       120 // read error, fast retry

#define ST_FINAL_F         200 // final error states
#define ST_BAD_IDENTIFY_F  210 // identify failed (incompatible firmware, etc)
#define ST_BAD_REBOOT_F    220 // reboot to user page firmware failed
#define ST_BAD_CONFIGURE_F 230 // configure failed

#define ST_ABSENT 0 // empty slot
//#define ST_GOOD   1 // state is good
#define ST_NO_ESPER  2 // no esper object, an empty slot
#define ST_BAD_IDENTIFY 3 // probe and identify failure
//#define ST_BAD_READ     4 // read failure
//#define ST_BAD_CHECK    5 // check failure

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

   int fConfPollSleep = 10;
   int fConfFailedSleep = 10;

public: // state and global variables
   std::mutex fLock;

   int fNumBanks = 0;

   Fault fCheckComm;
   Fault fCheckId;
   //Fault fCheckPage;
   Fault fCheckEsata0;
   Fault fCheckEsataLock;
   Fault fCheckPllLock;
   Fault fCheckUdpState;
   Fault fCheckRunState;

   Fault fCheckAdc16Locked;
   Fault fCheckAdc16Aligned;

   Fault fCheckAdc32Locked;
   Fault fCheckAdc32Aligned;

public:
   AdcCtrl(TMFE* xmfe, TMFeEquipment* xeq, const char* xodbname, int xodbindex)
   {
      fMfe = xmfe;
      fEq = xeq;
      fOdbName = xodbname;
      fOdbIndex = xodbindex;

      fCheckComm.Setup(fMfe, fEq, fOdbName.c_str(), "communication");
      fCheckId.Setup(fMfe, fEq, fOdbName.c_str(), "identification");
      //fCheckPage.Setup(fMfe, fEq, fOdbName.c_str(), "epcq boot page");
      fCheckEsata0.Setup(fMfe, fEq, fOdbName.c_str(), "no ESATA clock");
      fCheckEsataLock.Setup(fMfe, fEq, fOdbName.c_str(), "ESATA clock lock");
      fCheckPllLock.Setup(fMfe, fEq, fOdbName.c_str(), "PLL lock");
      fCheckUdpState.Setup(fMfe, fEq, fOdbName.c_str(), "UDP state");
      fCheckRunState.Setup(fMfe, fEq, fOdbName.c_str(), "run state");

      fCheckAdc16Locked.Setup(fMfe, fEq, fOdbName.c_str(), "adc16 lock");
      fCheckAdc16Aligned.Setup(fMfe, fEq, fOdbName.c_str(), "adc16 align");
      fCheckAdc32Locked.Setup(fMfe, fEq, fOdbName.c_str(), "adc32 lock");
      fCheckAdc32Aligned.Setup(fMfe, fEq, fOdbName.c_str(), "adc32 align");
   }

   void Lock()
   {
      fLock.lock();
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

   uint32_t fEpcqPage = 0;
   bool     fUserPage = false;

   bool fLmkFirstTime = true;
   int fLmkPll1lcnt = 0;
   int fLmkPll2lcnt = 0;

   double fFpgaTemp = 0;
   double fSensorTempBoard = 0;
   double fSensorTempAmpMin = 0;
   double fSensorTempAmpMax = 0;

   bool fUnusable = false;

   bool fEnableAdcTrigger = true;

   int    fSfpVendorPn = 0;
   double fSfpTemp = 0;
   double fSfpVcc  = 0;
   double fSfpTxBias  = 0;
   double fSfpTxPower = 0;
   double fSfpRxPower = 0;

   double fTrigEsataCnt = 0;

   double fLmkDac = 0;

   uint32_t fAdc16LockedCnt[4] = { 0,0,0,0 };
   uint32_t fAdc16AlignedCnt[4] = { 0,0,0,0 };

   uint32_t fFmc32LockedCnt[4] = { 0,0,0,0 };
   uint32_t fFmc32AlignedCnt[4] = { 0,0,0,0 };

   int fExtTrigCount0 = 0;
   int fTriggerTotalRequested0 = 0;
   int fTriggerTotalAccepted0 = 0;
   int fTriggerTotalDropped0 = 0;
   int fOffloadTxCnt0 = 0;

   int fExtTrigCount = 0;
   int fTriggerTotalRequested = 0;
   int fTriggerTotalAccepted = 0;
   int fTriggerTotalDropped = 0;
   int fOffloadTxCnt = 0;

   bool CheckAdcLocked(EsperNodeData data)
   {
      assert(fEsper);

      if (fEsper->fFailed) {
         if (!fCheckComm.fFailed) {
            fCheckComm.Fail("see previous messages");
         }
         fUnusable = true;
         return false;
      }

      if (fUnusable) {
         return false;
      }

      int run_state = 0;
      int transition_in_progress = 0;
      fMfe->fOdbRoot->RI("Runinfo/State", &run_state);
      fMfe->fOdbRoot->RI("Runinfo/Transition in progress", &transition_in_progress);

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
      int lmk_status_0 = data["board"].ba["lmk_status"][0];
      int lmk_status_1 = data["board"].ba["lmk_status"][1];
      int page_select = data["update"].i["page_select"];

      fExtTrigCount = data["board"].i["trig_esata_cnt"];

      //fTriggerTotalRequested = data["trigger"].i["total_requested"];
      //fTriggerTotalAccepted = data["trigger"].i["total_accepted"];
      //fTriggerTotalDropped = data["trigger"].i["total_dropped"];
      //fOffloadTxCnt = data["offload"].i["tx_cnt"];

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

      if (lmk_status_0 && lmk_status_1) {
         fCheckPllLock.Ok();
      } else if (!lmk_pll1_lock || !lmk_pll2_lock) {
         fCheckPllLock.Fail("lmk_pll1_lock or lmk_pll2_lock is bad");
         ok = false;
      } else {
         fCheckPllLock.Ok();
      }

      if (lmk_pll1_lcnt != fLmkPll1lcnt) {
         if (!fLmkFirstTime) {
            fMfe->Msg(MERROR, "Check", "%s: LMK PLL1 lock count changed from %d to %d", fOdbName.c_str(), fLmkPll1lcnt, lmk_pll1_lcnt);
         }
         fLmkPll1lcnt = lmk_pll1_lcnt;
      }

      if (lmk_pll2_lcnt != fLmkPll2lcnt) {
         if (!fLmkFirstTime) {
            fMfe->Msg(MERROR, "Check", "%s: LMK PLL2 lock count changed from %d to %d", fOdbName.c_str(), fLmkPll2lcnt, lmk_pll2_lcnt);
         }
         fLmkPll2lcnt = lmk_pll2_lcnt;
      }

      if (fConfAdc16Enable) {
         int bitmap = 0;
         for (int i=0; i<4; i++) {
            if (data["board"].ba["adc_locked"][i])
               bitmap |= (1<<i);

            if (1) {
               uint32_t adc_locked_cnt = data["board"].ia["adc_locked_cnt"][i];
               if (adc_locked_cnt != fAdc16LockedCnt[i]) {
                  if (!fLmkFirstTime) {
                     fMfe->Msg(MERROR, "Check", "%s: ADC16[%d] lock count changed from %d to %d", fOdbName.c_str(), i, fAdc16LockedCnt[i], adc_locked_cnt);
                  }
                  fAdc16LockedCnt[i] = adc_locked_cnt;
               }
            }

            if (1) {
               uint32_t adc_aligned_cnt = data["board"].ia["adc_aligned_cnt"][i];
               if (adc_aligned_cnt != fAdc16AlignedCnt[i]) {
                  if (!fLmkFirstTime) {
                     fMfe->Msg(MERROR, "Check", "%s: ADC16[%d] aligned count changed from %d to %d", fOdbName.c_str(), i, fAdc16AlignedCnt[i], adc_aligned_cnt);
                  }
                  fAdc16AlignedCnt[i] = adc_aligned_cnt;
               }
            }
         }
         if (bitmap != 0xF) {
            fCheckAdc16Locked.Fail("adc16 not locked, bitmap: " + toHexString(bitmap));
         } else {
            fCheckAdc16Locked.Ok();
         }
      } else {
         fCheckAdc16Locked.Ok();
      }

      if (fConfAdc16Enable) {
         int bitmap = 0;
         for (int i=0; i<4; i++) {
            if (data["board"].ba["adc_aligned"][i])
               bitmap |= (1<<i);
         }
         if (bitmap != 0xF) {
            fCheckAdc16Aligned.Fail("adc16 not aligned, bitmap: " + toHexString(bitmap));
         } else {
            fCheckAdc16Aligned.Ok();
         }
      } else {
         fCheckAdc16Aligned.Ok();
      }

      if (fConfAdc32Enable) {
         int bitmap = 0;
         for (int i=0; i<4; i++) {
            if (data["board"].ba["fmc_locked"][i])
               bitmap |= (1<<i);

            if (1) {
               uint32_t fmc_locked_cnt = data["board"].ia["fmc_locked_cnt"][i];
               if (fmc_locked_cnt != fFmc32LockedCnt[i]) {
                  if (!fLmkFirstTime) {
                     fMfe->Msg(MERROR, "Check", "%s: FMC-ADC32[%d] lock count changed from %d to %d", fOdbName.c_str(), i, fFmc32LockedCnt[i], fmc_locked_cnt);
                  }
                  fFmc32LockedCnt[i] = fmc_locked_cnt;
               }
            }
            
            if (1) {
               uint32_t fmc_aligned_cnt = data["board"].ia["fmc_aligned_cnt"][i];
               if (fmc_aligned_cnt != fFmc32AlignedCnt[i]) {
                  if (!fLmkFirstTime) {
                     fMfe->Msg(MERROR, "Check", "%s: FMC-ADC32[%d] aligned count changed from %d to %d, increment %d", fOdbName.c_str(), i, fFmc32AlignedCnt[i], fmc_aligned_cnt, fmc_aligned_cnt-fFmc32AlignedCnt[i]);
                  }
                  fFmc32AlignedCnt[i] = fmc_aligned_cnt;
               }
            }
         }
         if (bitmap != 0xF) {
            fCheckAdc32Locked.Fail("adc32 not locked, bitmap: " + toHexString(bitmap));
         } else {
            fCheckAdc32Locked.Ok();
         }
      } else {
         fCheckAdc32Locked.Ok();
      }

      if (fConfAdc32Enable) {
         int bitmap = 0;
         for (int i=0; i<4; i++) {
            if (data["board"].ba["fmc_aligned"][i])
               bitmap |= (1<<i);
         }
         if (bitmap != 0xF) {
            fCheckAdc32Aligned.Fail("adc32 not aligned, bitmap: " + toHexString(bitmap));
         } else {
            fCheckAdc32Aligned.Ok();
         }
      } else {
         fCheckAdc32Aligned.Ok();
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

      std::vector<double> *sensor_temp = &data["board"].da["sensor_temp"];

      if (sensor_temp->size() == 9) {
         fSensorTempBoard  = (*sensor_temp)[8];
         fSensorTempAmpMin = (*sensor_temp)[0];
         fSensorTempAmpMax = (*sensor_temp)[0];

         for (unsigned i=0; i<8; i++) {
            double t = (*sensor_temp)[i];
            if (t > fSensorTempAmpMax)
               fSensorTempAmpMax = t;
            if (t < fSensorTempAmpMin)
               fSensorTempAmpMin = t;
         }
      } else {
         fSensorTempBoard = 0;
         fSensorTempAmpMin = 0;
         fSensorTempAmpMax = 0;
      }

      std::string sfp_vendor_pn = data["sfp"].s["vendor_pn"];
      if (sfp_vendor_pn == "AFBR-57M5APZ")
         fSfpVendorPn = 1;
      else
         fSfpVendorPn = 0;
      fSfpTemp = data["sfp"].d["temp"];
      fSfpVcc = data["sfp"].d["vcc"];
      fSfpTxBias = data["sfp"].d["tx_bias"];
      fSfpTxPower = data["sfp"].d["tx_power"];
      fSfpRxPower = data["sfp"].d["rx_power"];

      fTrigEsataCnt = data["board"].i["trig_esata_cnt"];

      fLmkDac = data["board"].i["lmk_dac"];

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
      std::string image_location;
      std::string image_selected;
      std::string page_select;
      std::string sel_page;
      image_location = fEsper->Read(fMfe, "update", "image_location");
      image_selected = fEsper->Read(fMfe, "update", "image_selected");

      fMfe->Msg(MLOG, "Identify", "%s: remote update status: wdtimer: %s, nconfig: %s, runconfig: %s, nstatus: %s, crcerror: %s, watchdog_ena: %s, application: %s, image_location: %s, image_selected: %s, page_select: %s, sel_page: %s", fOdbName.c_str(),
                wdtimer_src.c_str(),
                nconfig_src.c_str(),
                runconfig_src.c_str(),
                nstatus_src.c_str(),
                crcerror_src.c_str(),
                watchdog_ena.c_str(),
                application.c_str(),
                image_location.c_str(),
                image_selected.c_str(),
                page_select.c_str(),
                sel_page.c_str()
                );
   }

   void RebootAdcLocked()
   {
      if (fEsper) {
         fEsper->Write(fMfe, "update", "reconfigure", "y", true);
      }
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

      if (fEsper->fFailed) {
         fCheckComm.Fail(fEsper->fFailedMessage);
         fCheckId.Fail("esper failure");
         return false;
      }

      if (!(elf_buildtime.length() > 0)) {
         fCheckId.Fail("cannot read board.elf_buildtime");
         return false;
      }

      std::string sw_qsys_ts = fEsper->Read(fMfe, "board", "sw_qsys_ts", &fLastErrmsg);

      if (!(sw_qsys_ts.length() > 0)) {
         fCheckId.Fail("cannot read board.sw_qsys_ts");
         return false;
      }

      std::string hw_qsys_ts = fEsper->Read(fMfe, "board", "hw_qsys_ts", &fLastErrmsg);

      if (!(hw_qsys_ts.length() > 0)) {
         fCheckId.Fail("cannot read board.hw_qsys_ts");
         return false;
      }

      std::string fpga_build = fEsper->Read(fMfe, "board", "fpga_build", &fLastErrmsg);

      if (!(fpga_build.length() > 0)) {
         fCheckId.Fail("cannot read board.fpga_build");
         return false;
      }

      std::string image_location_str = fEsper->Read(fMfe, "update", "image_location", &fLastErrmsg);

      if (!(image_location_str.length() > 0)) {
         fCheckId.Fail("cannot read update.image_location");
         return false;
      }

      std::string image_selected_str = fEsper->Read(fMfe, "update", "image_selected", &fLastErrmsg);
      if (!(image_selected_str.length() > 0)) {
         fCheckId.Fail("cannot read update.image_selected_str");
         return false;
      }

      fEpcqPage = xatoi(image_location_str.c_str());

      uint32_t elf_ts = xatoi(elf_buildtime.c_str());
      uint32_t qsys_sw_ts = xatoi(sw_qsys_ts.c_str());
      uint32_t qsys_hw_ts = xatoi(hw_qsys_ts.c_str());
      uint32_t sof_ts = xatoi(fpga_build.c_str());
      //uint32_t image_location = xatoi(image_location_str.c_str());
      //uint32_t image_selected = xatoi(image_selected_str.c_str());

      fMfe->Msg(MLOG, "Identify", "%s: firmware: elf 0x%08x, qsys_sw 0x%08x, qsys_hw 0x%08x, sof 0x%08x, epcq page %d", fOdbName.c_str(), elf_ts, qsys_sw_ts, qsys_hw_ts, sof_ts, fEpcqPage);

      ReportAdcUpdateLocked();

      fUserPage = false;

      if (fEpcqPage == 0) {
         // factory page
         fUserPage = false;
      } else if (fEpcqPage == 16777216) {
         // user page
         fUserPage = true;
      } else {
         fMfe->Msg(MERROR, "Identify", "%s: unexpected value of update.image_location: %s", fOdbName.c_str(), image_location_str.c_str());
         fCheckId.Fail("incompatible firmware, update.image_location: " + image_location_str);
         return false;
      }

      bool boot_load_only = false;

      if (0) {
      //} else if (elf_ts == 0x59555815) {
      //   boot_load_only = true;
      //} else if (elf_ts == 0x59baf6f8) {
      //   boot_load_only = true;
      //} else if (elf_ts == 0x59e552ef) {
      //   boot_load_only = true;
      //} else if (elf_ts == 0x59eea9d4) { // added module_id
      //} else if (elf_ts == 0x5a8cd478) { // BShaw build rel-20180220_fixed_temperature_sense
      //} else if (elf_ts == 0x5a8f07b0) { // BShaw build rel-20180220_fixed_temperature_sense, unknown build
      //} else if (elf_ts == 0x5a8f5628) { // BShaw build rel-20180220_fixed_temperature_sense
      //} else if (elf_ts == 0x5ab05ba4) { // merge bshaw branch, rebuild using scripts
      //} else if (elf_ts == 0x5ab9753c) { // add adc discriminator threshold
      //} else if (elf_ts == 0x5ac5586b) { // bshaw
      //} else if (elf_ts == 0x5ace87c6) { // KO - fix write to factory page
      //} else if (elf_ts == 0x5ae77ef4) { // KO - implement DAC control
      //} else if (elf_ts == 0x5aea45a3) { // KO - DAC runs at 125 MHz
      } else if (elf_ts == 0x5aecb3a5) { // KO - fix limits on adc16 max number of samples 511->699
         boot_load_only = true;
      } else if (elf_ts == 0x5ba2bc11) { // FMC-ADC32 rev 1.1
         boot_load_only = true;
      } else if (elf_ts == 0x5bac1c0c) { // FMC-ADC32 rev 1.1
      } else {
         fMfe->Msg(MERROR, "Identify", "%s: firmware is not compatible with the daq, elf_buildtime 0x%08x", fOdbName.c_str(), elf_ts);
         fCheckId.Fail("incompatible firmware, elf_buildtime: " + elf_buildtime);
         return false;
      }

      if (qsys_sw_ts != qsys_hw_ts) {
         boot_load_only = true;
         fMfe->Msg(MERROR, "Identify", "%s: firmware is not compatible with the daq, qsys mismatch, sw 0x%08x vs hw 0x%08x", fOdbName.c_str(), qsys_sw_ts, qsys_hw_ts);
         fCheckId.Fail("incompatible firmware, qsys timestamp mismatch, sw: " + sw_qsys_ts + ", hw: " + hw_qsys_ts);
         return false;
      }

      if (0) {
      //} else if (sof_ts == 0x594b603a) {
      //   boot_load_only = true;
      //} else if (sof_ts == 0x59d96d5a) {
      //   boot_load_only = true;
      //} else if (sof_ts == 0x59e691dc) {
      //   boot_load_only = true;
      //} else if (sof_ts == 0x59e7d5f2) {
      //   boot_load_only = true;
      //} else if (sof_ts == 0x59eeae46) { // added module_id and adc16 discriminators
      //} else if (sof_ts == 0x5a839e66) { // added trigger thresholds via module_id upper 4 bits, added adc32 discriminators
      //} else if (sof_ts == 0x5a8cd5af) { // BShaw build rel-20180220_fixed_temperature_sense
      //} else if (sof_ts == 0x5a8f1b17) { // BShaw build rel-20180220_fixed_temperature_sense
      //} else if (sof_ts == 0x5ab05bd6) { // merge bshaw branch, rebuild using scripts
      //} else if (sof_ts == 0x5ababacb) { // add adc discriminator threshold
      //} else if (sof_ts == 0x5ac5587c) { // bshaw
      //} else if (sof_ts == 0x5ace8836) { // KO - fix write to factory page
      //} else if (sof_ts == 0x5ae1329b) { // KO - drive sas links on fmc-adc32-rev1
      //} else if (sof_ts == 0x5ae79d6d) { // KO - implement DAC control
      //} else if (sof_ts == 0x5aea344a) { // KO - implement DAC control and calibration pulse
      } else if (sof_ts == 0x5aeb85b8) { // KO - DAC runs at 125 MHz
         boot_load_only = true;
      } else if (sof_ts == 0x5af0dece) { // KO - ramp DAC output
         boot_load_only = true;
      } else if (sof_ts == 0x5af311a2) { // KO - improve ramp DAC output
         boot_load_only = true;
      } else if (sof_ts == 0x5af4aae2) { // KO - improve ramp DAC output
         boot_load_only = true;
      } else if (sof_ts == 0x5af53bc0) { // KO - fix DAC_D LVDS drivers
         boot_load_only = true;
      } else if (sof_ts == 0x5b07356b) { // rel-20180524-ko
         boot_load_only = true;
      //} else if (sof_ts == 0x5ba2bc2d) { // FMC-ADC32 rev 1.1
      //} else if (sof_ts == 0x5bac1c1e) { // FMC-ADC32 rev 1.1
      //} else if (sof_ts == 0x5bac34c3) { // test
      //} else if (sof_ts == 0x5bad3538) { // test
      } else if (sof_ts == 0x5bad4c7d) { // rel-20180927-ko - negative and positive discriminator thresholds
      } else {
         fMfe->Msg(MERROR, "Identify", "%s: firmware is not compatible with the daq, sof fpga_build  0x%08x", fOdbName.c_str(), sof_ts);
         fCheckId.Fail("incompatible firmware, fpga_build: " + fpga_build);
         return false;
      }

      bool boot_from_user_page = false;

      fEq->fOdbEqSettings->RBAI("ADC/boot_user_page", fOdbIndex, &boot_from_user_page);

#if 0
      fMfe->Msg(MERROR, "Identify", "%s: rebooting to user page: boot_from_user_page: %d, fUserPage %d, fRebootingToUserPage %d", fOdbName.c_str(),
                boot_from_user_page,
                fUserPage,
                fRebootingToUserPage);
#endif

      if (boot_from_user_page != fUserPage) {
         if (boot_from_user_page) {
            if (fRebootingToUserPage) {
               fMfe->Msg(MERROR, "Identify", "%s: failed to boot the epcq user page", fOdbName.c_str());
               fRebootingToUserPage = false;
               ReportAdcUpdateLocked();
               return false;
            }

            fMfe->Msg(MERROR, "Identify", "%s: rebooting to the epcq user page", fOdbName.c_str());
            fEsper->Write(fMfe, "update", "image_selected", "1");
            RebootAdcLocked();
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
      fCheckComm.Ok();
      fUnusable = false;

      return true;
   }

   bool fConfAdc16Enable = true;
   bool fConfAdc32Enable = false;

   int fNumBanksAdc16 = 0;
   int fNumBanksAdc32 = 0;

   bool InitAdcLocked()
   {
      bool ok = IdentifyAdcLocked();
      if (ok) {
         ok = ConfigureAdcLocked();
      }
      ReadAndCheckAdcLocked();
      return ok;
   }

   bool ConfigureAdcDacLocked()
   {
      printf("ConfigureAdcDacLocked [%s]\n", fOdbName.c_str());
      assert(fEsper);

      bool ok = true;

      // configure the DAC

      bool dac_enable = false;
      bool fw_pulser = false;
      int dac_baseline = 0;
      int dac_amplitude = 0;
      int dac_seliq = 0;
      int dac_xor = 0;
      int dac_torb = 0;
      bool pulser_enable = false;
      bool ramp_enable = false;
      int ramp_up_rate = 0;
      int ramp_down_rate = 0;
      int ramp_top_len = 0;
      
      bool fw_pulser_enable = false;
      fEq->fOdbEqSettings->RB("FwPulserEnable", &fw_pulser_enable, true);
      
      fEq->fOdbEqSettings->RBAI("ADC/DAC/dac_enable", fOdbIndex, &dac_enable);
      fEq->fOdbEqSettings->RBAI("ADC/DAC/fw_pulser",  fOdbIndex, &fw_pulser);
      
      fEq->fOdbEqSettings->RIAI("ADC/DAC/dac_baseline",  fOdbIndex, &dac_baseline);
      fEq->fOdbEqSettings->RIAI("ADC/DAC/dac_amplitude", fOdbIndex, &dac_amplitude);
      
      fEq->fOdbEqSettings->RIAI("ADC/DAC/dac_seliq", fOdbIndex, &dac_seliq);
      fEq->fOdbEqSettings->RIAI("ADC/DAC/dac_xor",   fOdbIndex, &dac_xor);
      fEq->fOdbEqSettings->RIAI("ADC/DAC/dac_torb",  fOdbIndex, &dac_torb);
      
      fEq->fOdbEqSettings->RBAI("ADC/DAC/pulser_enable", fOdbIndex, &pulser_enable);
      fEq->fOdbEqSettings->RBAI("ADC/DAC/ramp_enable",   fOdbIndex, &ramp_enable);
      
      fEq->fOdbEqSettings->RIAI("ADC/DAC/ramp_up_rate",   fOdbIndex, &ramp_up_rate);
      fEq->fOdbEqSettings->RIAI("ADC/DAC/ramp_down_rate", fOdbIndex, &ramp_down_rate);
      fEq->fOdbEqSettings->RIAI("ADC/DAC/ramp_top_len",   fOdbIndex, &ramp_top_len);
      
      if (fw_pulser && !fw_pulser_enable) {
         dac_enable = false;
      }
      
      if (!dac_enable) {
         fMfe->Msg(MLOG, "ADC::Configure", "%s: configure: dac disabled: dac_enable %d, fw_pulser %d, fw_pulser_enable %d", fOdbName.c_str(), dac_enable, fw_pulser, fw_pulser_enable);
         ok &= fEsper->Write(fMfe, "ag", "dac_data", "0"); // DAC output value 0
         ok &= fEsper->Write(fMfe, "ag", "dac_ctrl", "0"); // DAC power down state
         printf("ConfigureAdcDacLocked [%s] dac disabled\n", fOdbName.c_str());
         return ok;
      }

      uint32_t dac_data = 0;
      uint32_t dac_ctrl = 0;
      
      dac_data |= (dac_amplitude&0xFFFF) << 0;
      dac_data |= (dac_baseline&0xFFFF) << 16;
      
      if (dac_enable)
         dac_ctrl |= (1<<0); // ~DAC_PD
      
      if (dac_seliq)
         dac_ctrl |= (1<<1); // DAC_SELIQ
      
      if (dac_xor)
         dac_ctrl |= (1<<2); // DAC_XOR
      
      if (dac_torb)
         dac_ctrl |= (1<<3); // DAC_TORB
      
      if (pulser_enable)
         dac_ctrl |= (1<<4); // pulser_enable
      
      if (ramp_enable)
         dac_ctrl |= (1<<5); // ramp_enable
      
      // bit 6 not used
      // bit 7 not used
      
      dac_ctrl |= ((ramp_top_len&0xFF)<<8);
      dac_ctrl |= ((ramp_down_rate&0xFF)<<16);
      dac_ctrl |= ((ramp_up_rate&0xFF)<<24);
      
      fMfe->Msg(MINFO, "ADC::Configure", "%s: configure: dac_data 0x%08x, dac_ctrl 0x0x%08x", fOdbName.c_str(), dac_data, dac_ctrl);
      ok &= fEsper->Write(fMfe, "ag", "dac_data", toString(dac_data).c_str());
      ok &= fEsper->Write(fMfe, "ag", "dac_ctrl", toString(dac_ctrl).c_str());

      printf("ConfigureAdcDacLocked [%s] done\n", fOdbName.c_str());

      return ok;
   }

   bool ConfigureAdcLocked()
   {
      assert(fEsper);

      if (fEsper->fFailed) {
         printf("Configure %s: failed flag\n", fOdbName.c_str());
         return false;
      }

      fEq->fOdbEqSettings->RI("PeriodAdc", &fConfPollSleep, true);
      //fEq->fOdbEqSettings->RI("PollAdc", &fConfFailedSleep, true);

      int udp_port = 0;

      //fMfe->fOdbRoot->RI("Equipment/XUDP/Settings/udp_port_adc", &udp_port);
      fMfe->fOdbRoot->RI("Equipment/ADC_UDP/Settings/udp_port", &udp_port);

      int adc16_samples = 700;
      int adc16_trig_delay = 0;
      int adc16_trig_start = 200;

      fEq->fOdbEqSettings->RI("ADC/adc16_samples",    &adc16_samples, true);
      fEq->fOdbEqSettings->RI("ADC/adc16_trig_delay", &adc16_trig_delay, true);
      fEq->fOdbEqSettings->RI("ADC/adc16_trig_start", &adc16_trig_start, true);
      fEq->fOdbEqSettings->RBAI("ADC/adc16_enable",   fOdbIndex, &fConfAdc16Enable);

      int adc32_samples = 511;
      int adc32_trig_delay = 0;
      int adc32_trig_start = 175;

      fEq->fOdbEqSettings->RI("ADC/adc32_samples",    &adc32_samples, true);
      fEq->fOdbEqSettings->RI("ADC/adc32_trig_delay", &adc32_trig_delay, true);
      fEq->fOdbEqSettings->RI("ADC/adc32_trig_start", &adc32_trig_start, true);
      fEq->fOdbEqSettings->RBAI("ADC/adc32_enable", fOdbIndex, &fConfAdc32Enable);
      
      //int thr = 1;
      //fEq->fOdbEqSettings->RI("ADC/trig_threshold", 0, &thr, true);

      uint32_t module_id = 0;
      module_id |= ((fOdbIndex)&0x0F);
      //module_id |= ((thr<<4)&0xF0);

      int adc16_threshold = 0x2000;
      fEq->fOdbEqSettings->RI("ADC/adc16_threshold", &adc16_threshold, true);

      int adc32_threshold = 0x2000;
      fEq->fOdbEqSettings->RI("ADC/adc32_threshold", &adc32_threshold, true);

      fMfe->Msg(MINFO, "ADC::Configure", "%s: configure: udp_port %d, adc16 enable %d, samples %d, trig_delay %d, trig_start %d, thr %d, adc32 enable %d, samples %d, trig_delay %d, trig_start %d, thr %d, module_id 0x%02x", fOdbName.c_str(), udp_port, fConfAdc16Enable, adc16_samples, adc16_trig_delay, adc16_trig_start, adc16_threshold, fConfAdc32Enable, adc32_samples, adc32_trig_delay, adc32_trig_start, adc32_threshold, module_id);

      bool ok = true;

      ok &= StopAdcLocked();

      // make sure everything is stopped

      ok &= fEsper->Write(fMfe, "board", "force_run", "false");
      ok &= fEsper->Write(fMfe, "udp", "enable", "false");

      // switch clock to ESATA

      //ok &= fEsper->Write(fMfe, "board", "clk_lmk", "1");

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

      if (fConfAdc32Enable) {
         std::string json;
         json += "[";
         for (int i=0; i<32; i++) {
            json += toString(1);
            json += ",";
         }
         json += "]";
         
         ok &= fEsper->Write(fMfe, "fmc32", "dac_offset", json.c_str());
      }

      // program module id and trigger threshold

      ok &= fEsper->Write(fMfe, "board", "module_id", toString(module_id).c_str());

      // program adc threshold

      ok &= fEsper->Write(fMfe, "ag", "adc16_threshold", toString(adc16_threshold).c_str());
      ok &= fEsper->Write(fMfe, "ag", "adc32_threshold", toString(adc32_threshold).c_str());

      // program the IP address and port number in the UDP transmitter

      int udp_ip = 0;
      udp_ip |= (192<<24);
      udp_ip |= (168<<16);
      udp_ip |= (1<<8);
      udp_ip |= (1<<0);

      ok &= fEsper->Write(fMfe, "udp", "dst_ip", toString(udp_ip).c_str());
      ok &= fEsper->Write(fMfe, "udp", "dst_port", toString(udp_port).c_str());
      ok &= fEsper->Write(fMfe, "udp", "enable", "true");

      ok &= ConfigureAdcDacLocked();

#if 0
      // configure the DAC

      bool dac_enable = true;
      std::vector<std::string> dac_modules;
      dac_modules.push_back("adc18");
      uint32_t dac_data = 4000;
      uint32_t dac_ctrl = 18;

      fEq->fOdbEqSettings->RB("dac_enable", &dac_enable, true);
      fEq->fOdbEqSettings->RSA("dac_module", &dac_modules, true, 1, 32);
      fEq->fOdbEqSettings->RU32("dac_data", &dac_data, true);
      fEq->fOdbEqSettings->RU32("dac_ctrl", &dac_ctrl, true);

      bool enable_dac = false;
      
      for (unsigned i=0; i<dac_modules.size(); i++) {
         if (dac_modules[i] == fOdbName) {
            enable_dac = true;
         }
      }

      if (enable_dac && dac_enable) {
         fMfe->Msg(MINFO, "ADC::Configure", "%s: configure: dac_data 0x%08x, dac_ctrl 0x0x%08x", fOdbName.c_str(), dac_data, dac_ctrl);
         ok &= fEsper->Write(fMfe, "ag", "dac_data", toString(dac_data).c_str());
         ok &= fEsper->Write(fMfe, "ag", "dac_ctrl", toString(dac_ctrl).c_str());
      } else {
         ok &= fEsper->Write(fMfe, "ag", "dac_data", "0"); // DAC output value 0
         ok &= fEsper->Write(fMfe, "ag", "dac_ctrl", "0"); // DAC power down state
      }
#endif

      return ok;
   }

   bool StartAdcLocked()
   {
      assert(fEsper);
      bool ok = true;
      //ok &= fEsper->Write(fMfe, "board", "nim_ena", "true");
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
      while (!fMfe->fShutdownRequested) {
         if (fEsper->fFailed) {
            bool ok;
            {
               std::lock_guard<std::mutex> lock(fLock);
               ok = IdentifyAdcLocked();
               // fLock implicit unlock
            }
            if (!ok) {
               fState = ST_BAD_IDENTIFY;
               for (int i=0; i<fConfFailedSleep; i++) {
                  if (fMfe->fShutdownRequested)
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

         for (int i=0; i<fConfPollSleep; i++) {
            if (fMfe->fShutdownRequested)
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
      double t0 = TMFE::GetTime();
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
      fExtTrigCount0 = fExtTrigCount;
      fTriggerTotalRequested0 = fTriggerTotalRequested;
      fTriggerTotalAccepted0 = fTriggerTotalAccepted;
      fTriggerTotalDropped0 = fTriggerTotalDropped;
      fOffloadTxCnt0 = fOffloadTxCnt;
      if (start && enableAdcTrigger) {
         StartAdcLocked();
      }
      double t1 = TMFE::GetTime();
      printf("BeginRunAdcLocked: %s: thread start time %f, begin run time %f\n", fOdbName.c_str(), t0-gBeginRunStartThreadsTime, t1-t0);
   }
};

struct mv2calib
{
   double fFactor=0.;
   std::vector<double> fZero = {2<<14, 2<<14, 2<<14};    // {x, y, z} default zero at midscale
   std::map<int, std::map<int, std::vector<double> > > fZeros = {
      { 0, {                                             // pwb
            { 0, { 30676.33, 30676.33, 30678.00 } },     // range:{x,y,z}
            { 1, { 30715.00, 30716.33, 30716.67 } },
            { 2, { 30737.00, 30736.00, 30734.00 } },
            { 3, { 30742.67, 30740.33, 30743.67 } }
         }
      },
      { 2, {
            { 0, { 30863.00, 30862.67, 30863.00 } },
            { 1, { 30832.67, 30832.33, 30830.33 } },
            { 2, { 30826.33, 30824.67, 30826.67 } },
            { 3, { 30822.67, 30821.67, 30819.67 } }
         }
      },
      { 4, {
            { 0, { 30600.00, 30602.67, 30599.33 } },
            { 1, { 30656.33, 30656.67, 30657.00 } },
            { 2, { 30680.00, 30681.00, 30680.67 } },
            { 3, { 30689.33, 30690.00, 30690.33 } }
         }
      },
      { 19, {
            { 0, { 30642.33, 30640.33, 30644.33 } },
            { 1, { 30699.67, 30699.33, 30700.00 } },
            { 2, { 30723.67, 30724.67, 30724.33 } },
            { 3, { 30733.67, 30734.33, 30734.67 } }
         }
      },
      { 29, {
            { 0, { 30811.33, 30811.00, 30808.67 } },
            { 1, { 30885.67, 30885.33, 30886.33 } },
            { 2, { 30915.00, 30915.00, 30914.67 } },
            { 3, { 30928.00, 30928.33, 30927.00 } }
         }
      },
      { 33, {
            { 0, { 30971.00, 30968.00, 30968.33 } },
            { 1, { 31038.00, 31036.33, 31038.00 } },
            { 2, { 31067.67, 31064.67, 31064.67 } },
            { 3, { 31076.33, 31078.33, 31077.33 } }
         }
      },
      { 47, {
            { 0, { 30771.67, 30768.67, 30770.33 } },
            { 1, { 30759.00, 30758.00, 30759.00 } },
            { 2, { 30759.00, 30759.00, 30758.00 } },
            { 3, { 30757.67, 30758.33, 30757.67 } }
         }
      },
      { 65, {
            { 0, { 31113.67, 31111.67, 31112.67 } },
            { 1, { 31174.33, 31170.67, 31171.00 } },
            { 2, { 31197.33, 31197.33, 31198.67 } },
            { 3, { 31208.00, 31210.33, 31207.67 } }
         }
      }
   };

   std::map<int, std::map<int, double > > fFactors = { // FIXME: range 0 and 1 calibrations are bogus,
                                                       // probe saturated
      { 0, {                                           // NMR not measured, used avg of PWBs 2,33,47,65
            { 0, 0.021573 },
            { 1, 0.023288 },
            { 2, 0.046022 },
            { 3, 0.139539 }
         }
      },
      { 2, {
            { 0, 0.021513 },
            { 1, 0.023052 },
            { 2, 0.051473 },
            { 3, 0.152942 }
         }
      },
      { 4, {                                           // NMR not measured, used avg of PWBs 2,33,47,65
            { 0, 0.021558 },
            { 1, 0.022663 },
            { 2, 0.044912 },
            { 3, 0.133685 }
         }
      },
      { 19, {                                           // NMR not measured, used avg of PWBs 2,33,47,65
            { 0, 0.021712 },
            { 1, 0.023700 },
            { 2, 0.046123 },
            { 3, 0.139309 }
         }
      },
      { 29, {                                           // NMR not measured, used avg of PWBs 2,33,47,65
            { 0, 0.021474 },
            { 1, 0.022295 },
            { 2, 0.044120 },
            { 3, 0.130385 }
         }
      },
      { 33, {
            { 0, 0.021383 },
            { 1, 0.023513 },
            { 2, 0.049780 },
            { 3, 0.152249 }
         }
      },
      { 47, {
            { 0, 0.021322 },
            { 1, 0.023206 },
            { 2, 0.048614 },
            { 3, 0.144540 }
         }
      },
      { 65, {
            { 0, 0.021476 },
            { 1, 0.022948 },
            { 2, 0.048572 },
            { 3, 0.146628 }
         }
      },
      { 999, {             // default scale factor
            { 0, 0.02 }, 
            { 1, 0.02 },
            { 2, 0.05 },
            { 3, 0.15 }
         }
      },
   };
   

   mv2calib(int pwb, int range)
   {
      if(fZeros.find(pwb) != fZeros.end()){
         if(fZeros[pwb].find(range) != fZeros[pwb].end()){
            fZero = fZeros[pwb][range];
         }
      }
      bool factFound(false);
      if(fFactors.find(pwb) != fFactors.end()){
         if(fFactors[pwb].find(range) != fFactors[pwb].end()){
            fFactor = fFactors[pwb][range];
            factFound = true;
         }
      }
      if(!factFound){
         fFactor = fFactors[999][range];
      }
      fFactor *= 0.001;
   }
   double Bcalib( int axis, int& b )
   {
      return (fFactor*(double(b)-fZero[axis]));
   }
};

class PwbCtrl;
PwbCtrl* FindPwbMate(const PwbCtrl* pwb);

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

   int fState = ST_EMPTY_SLOT_F;

   std::mutex fLock;

   // configuration from ODB

   int  fConfPollSleep = 10;
   int  fConfFailedSleep = 10;
   bool fConfTrigger = false;

   bool fSataLinkSlave = false;
   bool fSataLinkMaster = false;

   // firmware version-dependant functions

   bool fHaveBootLoadOnly = false; // this firmware can only be used as bootloader from factory to user page, cannot be used for daq.
   bool fHaveHwUdp = false; // have UDP offloader
   bool fHaveSataTrigger = false; // have sata link trigger
   bool fHaveSataLink = false; // have sata link data communications
   bool fHaveDataSuppression = false; // have channel suppression
   bool fHaveChannelBitmap = false; // channel enable and force are a bitmap
   bool fHaveChangeDelays = true; // firmware default delays must be changed

   // internal use

   std::string fLastErrmsg;
   bool fEnablePwbTrigger = true;

   int fNumBanks = 0;

   Fault fCheckState;
   Fault fCheckComm;
   Fault fCheckId;
   //Fault fCheckEsata0;
   //Fault fCheckEsataLock;
   Fault fCheckClockSelect;
   Fault fCheckPllLock;
   Fault fCheckUdpState;
   Fault fCheckRunState;
   Fault fCheckLink;
   Fault fCheckVp2;
   Fault fCheckVp5;
   Fault fCheckVsca12;
   Fault fCheckVsca34;
   Fault fCheckIp2;
   Fault fCheckIp5;
   Fault fCheckIsca12;
   Fault fCheckIsca34;
   Fault fCheckTempFpga;
   Fault fCheckTempBoard;
   Fault fCheckTempScaA;
   Fault fCheckTempScaB;
   Fault fCheckTempScaC;
   Fault fCheckTempScaD;

public:
   PwbCtrl(TMFE* xmfe, TMFeEquipment* xeq, const char* xodbname, int xodbindex)
   {
      fMfe = xmfe;
      fEq = xeq;
      fOdbName = xodbname;
      fOdbIndex = xodbindex;

      fCheckState.Setup(fMfe, fEq, fOdbName.c_str(), "state");
      fCheckComm.Setup(fMfe, fEq, fOdbName.c_str(), "communication");
      fCheckId.Setup(fMfe, fEq, fOdbName.c_str(), "identification");
      //fCheckEsata0.Setup(fMfe, fEq, fOdbName.c_str(), "no ESATA clock");
      //fCheckEsataLock.Setup(fMfe, fEq, fOdbName.c_str(), "ESATA clock lock");
      fCheckClockSelect.Setup(fMfe, fEq, fOdbName.c_str(), "clock select");
      fCheckPllLock.Setup(fMfe, fEq, fOdbName.c_str(), "PLL lock");
      fCheckUdpState.Setup(fMfe, fEq, fOdbName.c_str(), "UDP state");
      fCheckRunState.Setup(fMfe, fEq, fOdbName.c_str(), "run state");
      fCheckLink.Setup(fMfe, fEq, fOdbName.c_str(), "sata link"); // ,20
      fCheckVp2.Setup(fMfe, fEq, fOdbName.c_str(), "power 2V");
      fCheckVp5.Setup(fMfe, fEq, fOdbName.c_str(), "power 5V");
      fCheckVsca12.Setup(fMfe, fEq, fOdbName.c_str(), "sca12 4V");
      fCheckVsca34.Setup(fMfe, fEq, fOdbName.c_str(), "sca34 4V");
      fCheckIp2.Setup(fMfe, fEq, fOdbName.c_str(), "2V current");
      fCheckIp5.Setup(fMfe, fEq, fOdbName.c_str(), "5V current");
      fCheckIsca12.Setup(fMfe, fEq, fOdbName.c_str(), "sca12 current");
      fCheckIsca34.Setup(fMfe, fEq, fOdbName.c_str(), "sca34 current");
      fCheckTempFpga.Setup(fMfe, fEq, fOdbName.c_str(), "FPGA temperature");
      fCheckTempBoard.Setup(fMfe, fEq, fOdbName.c_str(), "Board temperature");
      fCheckTempScaA.Setup(fMfe, fEq, fOdbName.c_str(), "SCA A temperature");
      fCheckTempScaB.Setup(fMfe, fEq, fOdbName.c_str(), "SCA B temperature");
      fCheckTempScaC.Setup(fMfe, fEq, fOdbName.c_str(), "SCA C temperature");
      fCheckTempScaD.Setup(fMfe, fEq, fOdbName.c_str(), "SCA D temperature");
   }

   void Lock()
   {
      fLock.lock();
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

      bool ok = true;

      for (unsigned i=0; i<modules.size(); i++) {
         //if (modules[i] == "signalproc")
         //   continue;
         e = fEsper->ReadVariables(fMfe, fEq, fOdbName.c_str(), modules[i], &(*data)[modules[i]]);

         if (e.error)
            ok = false;
      }

      return ok;
   }

   int fUpdateCount = 0;

   uint32_t fEpcqPage = 0;
   bool     fUserPage = false;

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

   int    fSfpVendorPn = 0;
   double fSfpTemp = 0;
   double fSfpVcc  = 0;
   double fSfpTxBias  = 0;
   double fSfpTxPower = 0;
   double fSfpRxPower = 0;

   int fExtTrigCount0 = 0;
   int fTriggerTotalRequested0 = 0;
   int fTriggerTotalAccepted0 = 0;
   int fTriggerTotalDropped0 = 0;
   int fOffloadTxCnt0 = 0;

   int fExtTrigCount = 0;
   int fTriggerTotalRequested = 0;
   int fTriggerTotalAccepted = 0;
   int fTriggerTotalDropped = 0;
   int fOffloadTxCnt = 0;

   int fLastLmkLockCnt = 0;
   int fLastUptime = 0;
   
   // MV2 Hall probe reading
   bool fMV2enable=false;
   int fMV2range=-1;
   double fMV2_xaxis=-1.;
   double fMV2_yaxis=-1.;
   double fMV2_zaxis=-1.;
   double fMV2_taxis=-1.;

   std::string fStateMessage;

   void SetState(int state, const char* message)
   {
      if (state != fState || message != fStateMessage) {
         fMfe->Msg(MLOG, "SetState", "%s: state %d -> %d: %s", fOdbName.c_str(), fState, state, message);
      }
      fState = state;
      fStateMessage = message;
      //fCheckState.Fail(message);
      if (gPwbState) {
         gPwbState->WS(fOdbName.c_str(), message);
      }
   }

   bool fFirstCheck = true;

   bool CheckPwbLocked(EsperNodeData data, bool first_time)
   {
      assert(fEsper);

      if (fEsper->fFailed) {
         return false;
      }

      if (fFirstCheck) {
         fFirstCheck = false;
         first_time = true;
      }
      
      int run_state = 0;
      int transition_in_progress = 0;
      fMfe->fOdbRoot->RI("Runinfo/State", &run_state);
      fMfe->fOdbRoot->RI("Runinfo/Transition in progress", &transition_in_progress);

      bool running = (run_state == 3);

      if (!fEnablePwbTrigger)
         running = false;

      if (!fConfTrigger)
         running = false;

      //printf("%s: run_state %d, running %d, transition_in_progress %d\n", fOdbName.c_str(), run_state, running, transition_in_progress);

      bool ok = true;

      fEpcqPage = data["update"].i["image_location"];

      ok &= UpdateUserPagePwbLocked();

      if (CheckRebootToUserPagePwbLocked()) {
         fCheckId.Fail("not booted to epcq user page");
         ok = false;
      }

      bool plls_locked = data["clockcleaner"].b["plls_locked"];
      bool holdover_on = data["clockcleaner"].b["holdover_on"];
      bool sfp_sel = data["clockcleaner"].b["sfp_sel"];
      bool osc_sel = data["clockcleaner"].b["osc_sel"];
      bool sata_sel = data["clockcleaner"].b["sata_sel"];
      int freq_sfp  = data["board"].i["freq_sfp"];
      //int freq_sata = data["board"].i["freq_sata"];
      bool force_run = data["signalproc"].b["force_run"];

      if (0) {
         int chip_id = data["board"].i["chip_id"];
         int delay = (chip_id & 0xF); // from PWB firmware bootloader.c
         //int delay = 0;
         //for (int i=0; i<8; i++) {
         //   delay += (chip_id & 0xF);
         //   chip_id = chip_id >> 4;
         //}
         //delay &= 0xF;
         fMfe->Msg(MLOG, "CheckPwbLocked", "%s: chip_id 0x%x, delay %2d, index %2d, column %d\n", fOdbName.c_str(), chip_id, delay, fOdbIndex, fOdbIndex/8);
      }

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

      if (fVoltP2 < 2.0) {
         fCheckVp2.Fail("Voltage is low: " + doubleToString("%.2fV", fVoltP2), true);
         ok = false;
      } else {
         fCheckVp2.Ok();
      }

      if (fVoltP5 < 4.0) {
         fCheckVp5.Fail("Voltage is low: " + doubleToString("%.2fV", fVoltP5), true);
         ok = false;
      } else {
         fCheckVp5.Ok();
      }

      if (fVoltSca12 < 4.0) {
         fCheckVsca12.Fail("Voltage is low: " + doubleToString("%.2fV", fVoltSca12), true);
         ok = false;
      } else {
         fCheckVsca12.Ok();
      }

      if (fVoltSca34 < 4.0) {
         fCheckVsca34.Fail("Voltage is low: " + doubleToString("%.2fV", fVoltSca34), true);
         ok = false;
      } else {
         fCheckVsca34.Ok();
      }

      fCurrSca12 = data["board"].d["i_sca12"];
      fCurrSca34 = data["board"].d["i_sca34"];
      fCurrP2 = data["board"].d["i_p2"];
      fCurrP5 = data["board"].d["i_p5"];

      if (fCurrP2 <  950.0 || fCurrP2 > 1500) {
         fCheckIp2.Fail("out of range: " + doubleToString("%.1fmA", fCurrP2), true);
         ok = false;
      } else {
         fCheckIp2.Ok();
      }

      if (fCurrP5 < 2400 || fCurrP5 > 3000) {
         fCheckIp5.Fail("out of range: " + doubleToString("%.1fmA", fCurrP5), true);
         if (fOdbName == "pwb38") {
            if (fCurrP5 > 3400)
               ok = false;
         } else if (fOdbName == "pwb15") {
            if (fCurrP5 > 3600)
               ok = false;
         } else {
            ok = false;
         }
      } else {
         fCheckIp5.Ok();
      }

      if (fCurrSca12 < 500 || fCurrSca12 > 650) {
         fCheckIsca12.Fail("out of range: " + doubleToString("%.1fmA", fCurrSca12), true);
         //ok = false;
      } else {
         fCheckIsca12.Ok();
      }

      if (fCurrSca34 < 500 || fCurrSca34 > 650) {
         fCheckIsca34.Fail("out of range: " + doubleToString("%.1fmA", fCurrSca34), true);
         //ok = false;
      } else {
         fCheckIsca34.Ok();
      }

      //printf("%s: currents: sca12 %6.1f, sca34 %6.1f, P2 %6.1f, P5 %6.1f\n", fOdbName.c_str(), fCurrSca12, fCurrSca34, fCurrP2, fCurrP5);

      std::string sfp_vendor_pn = data["sfp"].s["vendor_pn"];
      if (sfp_vendor_pn == "AFBR-57M5APZ")
         fSfpVendorPn = 1;
      else
         fSfpVendorPn = 0;
      fSfpTemp = data["sfp"].d["temp"];
      fSfpVcc = data["sfp"].d["vcc"];
      fSfpTxBias = data["sfp"].d["tx_bias"];
      fSfpTxPower = data["sfp"].d["tx_power"];
      fSfpRxPower = data["sfp"].d["rx_power"];

      int lmk_lock_cnt = data["clockcleaner"].i["lmk_lock_cnt"];
      if (lmk_lock_cnt != fLastLmkLockCnt) {
         if (!first_time) {
            fMfe->Msg(MERROR, "Check", "%s: LMK PLL lock count changed from %d to %d", fOdbName.c_str(), fLastLmkLockCnt, lmk_lock_cnt);
         }
         fLastLmkLockCnt = lmk_lock_cnt;
      }

      int uptime = data["system"].i["uptime"];

      //printf("%s: uptime %d, +%d\n", fOdbName.c_str(), uptime, uptime-fLastUptime);

      if (!first_time) {
         if (uptime < fLastUptime) {
            fCheckId.Fail("unexpected reboot");
            ok = false;
            fMfe->Msg(MERROR, "Check", "%s: unexpected reboot, uptime %d -> %d", fOdbName.c_str(), fLastUptime, uptime);
         }
      }

      if (fCheckId.fFailed) {
         ok = false;
      }

      fLastUptime = uptime;

      fExtTrigCount = data["trigger"].i["ext_trig_requested"];

      if (fExtTrigCount == 0) {
         // fixup for old firmware
         fExtTrigCount = data["signalproc"].i["trig_cnt_ext"];
      }

      fTriggerTotalRequested = data["trigger"].i["total_requested"];
      fTriggerTotalAccepted = data["trigger"].i["total_accepted"];
      fTriggerTotalDropped = data["trigger"].i["total_dropped"];
      fOffloadTxCnt = data["offload"].i["tx_cnt"];

      if (first_time) {
         fExtTrigCount0 = fExtTrigCount;
         fTriggerTotalRequested0 = fTriggerTotalRequested;
         fTriggerTotalAccepted0 = fTriggerTotalAccepted;
         fTriggerTotalDropped0 = fTriggerTotalDropped;
         fOffloadTxCnt0 = fOffloadTxCnt;
      }

      printf("%s: fpga temp: %.1f %.1f %.1f %.1f %1.f %.1f, freq_sfp: %d, pll locked %d, holdover %d, sata_sel %d, sfp_sel %d, osc_sel %d, run %d\n",
             fOdbName.c_str(),
             fTempFpga,
             fTempBoard,
             fTempScaA,
             fTempScaB,
             fTempScaC,
             fTempScaD,
             freq_sfp,
             plls_locked,
             holdover_on,
             sata_sel,
             sfp_sel,
             osc_sel,
             force_run);

      //if (fTempFpga < 10) {
      //   fCheckTempFpga.Fail("FPGA temperature low: " + toString(fTempFpga));
      //} else if (fTempFpga > 10) {
      //   fCheckTempFpga.Fail("FPGA temperature high: " + toString(fTempFpga));
      //} else {
         fCheckTempFpga.Ok();
      //}

      if (fTempBoard < 30) {
         fCheckTempBoard.Fail("Board temperature low: " + toString(fTempBoard), true);
      } else if (fTempBoard > 50) {
         fCheckTempBoard.Fail("Board temperature high: " + toString(fTempBoard), true);
      } else {
         fCheckTempBoard.Ok();
      }

      if (fTempScaA < 30) {
         fCheckTempScaA.Fail("SCA A temperature low: " + toString(fTempScaA), true);
      } else if (fTempScaA > 50) {
         fCheckTempScaA.Fail("SCA A temperature high: " + toString(fTempScaA), true);
      } else {
         fCheckTempScaA.Ok();
      }

      if (fTempScaB < 30) {
         fCheckTempScaB.Fail("SCA B temperature low: " + toString(fTempScaB), true);
      } else if (fTempScaB > 50) {
         fCheckTempScaB.Fail("SCA B temperature high: " + toString(fTempScaB), true);
      } else {
         fCheckTempScaB.Ok();
      }

      if (fTempScaC < 30) {
         fCheckTempScaC.Fail("SCA C temperature low: " + toString(fTempScaC), true);
      } else if (fTempScaC > 50) {
         fCheckTempScaC.Fail("SCA C temperature high: " + toString(fTempScaC), true);
      } else {
         fCheckTempScaC.Ok();
      }

      if (fTempScaD < 30) {
         fCheckTempScaD.Fail("SCA D temperature low: " + toString(fTempScaD), true);
      } else if (fTempScaD > 50) {
         fCheckTempScaD.Fail("SCA D temperature high: " + toString(fTempScaD), true);
      } else {
         fCheckTempScaD.Ok();
      }

      if (!plls_locked) {
         fCheckPllLock.Fail("PLLs not locked");
         ok = false;
      } else {
         fCheckPllLock.Ok();
      }

      if (sfp_sel) {
         fCheckClockSelect.Fail("wrong clock selected: SFP clock");
         ok = false;
      } else if (osc_sel) {
         fCheckClockSelect.Fail("wrong clock selected: internal oscillator");
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

      if (1) {
         bool link_status = data["link"].b["link_status"];
         if (!link_status) {
            fCheckLink.Fail("bad link status");
            //if (fSataLinkMaster || fSataLinkSlave) {
            //   ok = false;
            //}
         } else {
            fCheckLink.Ok();
         }
      }

      // MV2 Hall probe reading, if enabled
      try
         {
            fMV2enable = data["board"].b["mv2_enable"];
         }
      catch( const std::out_of_range& oor)
         {
            fMV2enable = false;
         }

      if( fMV2enable )
         {
            fMV2range=data["board"].i["mv2_range"];
            mv2calib cal(fModule, fMV2range);
            fMV2_xaxis=cal.Bcalib(0, data["board"].i["mv2_xaxis"]);
            fMV2_yaxis=cal.Bcalib(1, data["board"].i["mv2_yaxis"]);
            fMV2_zaxis=cal.Bcalib(2, data["board"].i["mv2_zaxis"]);
            fMV2_taxis=data["board"].i["mv2_taxis"];
         }

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

   void RebootPwbLocked()
   {
      assert(fEsper);
      bool ok = true;

      SetState(ST_INITIAL, "reboot to factory page...");

      if (fSataLinkMaster) {
         PwbCtrl* mate = FindPwbMate(this);
         if (mate) {
            std::lock_guard<std::mutex> lock(mate->fLock);
            fMfe->Msg(MLOG, "RebootPwbLocked", "%s: switching slave \"%s\" clock to local oscillator", fOdbName.c_str(), mate->fOdbName.c_str());
            mate->fEsper->Write(fMfe, "clockcleaner", "clkin_sel", "2"); // switch to local oscillator
         }
         ok &= fEsper->Write(fMfe, "link", "link_ctrl", "0"); // turn off sata link master mode
      }

      if (fSataLinkSlave) {
         PwbCtrl* mate = FindPwbMate(this);
         if (mate) {
            std::lock_guard<std::mutex> lock(mate->fLock);
            fMfe->Msg(MLOG, "RebootPwbLocked", "%s: switching master \"%s\" clock to local oscillator", fOdbName.c_str(), mate->fOdbName.c_str());
            mate->fEsper->Write(fMfe, "clockcleaner", "clkin_sel", "2"); // switch to local oscillator
         }
      }

      ok &= fEsper->Write(fMfe, "clockcleaner", "clkin_sel", "2"); // switch to local oscillator

      // NB: reboot from user page to user page does not work
      //if (fUserPage) {
      //   fCheckId.Fail("rebooting to user epcq page");
      //   // write a zero, then write a one
      //   fEsper->Write(fMfe, "update", "image_selected", "0");
      //   fEsper->Write(fMfe, "update", "image_selected", "1");
      //} else {
         fCheckId.Fail("rebooting to factory epcq page");
         ok &= fEsper->Write(fMfe, "update", "image_selected", "0");
      //}
      ok &= fEsper->Write(fMfe, "update", "reconfigure", "y", true, true);
      fLastUptime = 0;
   }

   bool RebootToUserPagePwbLocked()
   {
      bool ok = true;
      fMfe->Msg(MINFO, "Identify", "%s: rebooting to the epcq user page", fOdbName.c_str());

      if (fSataLinkMaster) {
         PwbCtrl* mate = FindPwbMate(this);
         if (mate) {
            std::lock_guard<std::mutex> lock(mate->fLock);
            fMfe->Msg(MLOG, "RebootToUserPagePwbLocked", "%s: switching slave \"%s\" clock to local oscillator", fOdbName.c_str(), mate->fOdbName.c_str());
            mate->fEsper->Write(fMfe, "clockcleaner", "clkin_sel", "2"); // switch to local oscillator
         }
         ok &= fEsper->Write(fMfe, "link", "link_ctrl", "0"); // turn off sata link master mode
      }

      if (fSataLinkSlave) {
         PwbCtrl* mate = FindPwbMate(this);
         if (mate) {
            std::lock_guard<std::mutex> lock(mate->fLock);
            fMfe->Msg(MLOG, "RebootToUserPagePwbLocked", "%s: switching master \"%s\" clock to local oscillator", fOdbName.c_str(), mate->fOdbName.c_str());
            mate->fEsper->Write(fMfe, "clockcleaner", "clkin_sel", "2"); // switch to local oscillator
         }
      }

      ok &= fEsper->Write(fMfe, "clockcleaner", "clkin_sel", "2"); // switch to local oscillator

      ok &= fEsper->Write(fMfe, "update", "image_selected", "1");
      ok &= fEsper->Write(fMfe, "update", "reconfigure", "y", true, true);

      if (0&&fSataLinkSlave) {
         PwbCtrl* mate = FindPwbMate(this);
         if (mate) {
            std::lock_guard<std::mutex> lock(mate->fLock);
            fMfe->Msg(MLOG, "RebootToUserPagePwbLocked", "%s: disabling sata link master \"%s\"", fOdbName.c_str(), mate->fOdbName.c_str());
            mate->fEsper->Write(fMfe, "link", "link_ctrl", "0");
         }
      }

      fLastUptime = 0;
      return ok;
   }

   bool UpdateUserPagePwbLocked()
   {
      fUserPage = false;
      
      if (fEpcqPage == 0) {
         // factory page
         fUserPage = false;
      } else if (fEpcqPage == 16777216) {
         // user page
         fUserPage = true;
      } else {
         fMfe->Msg(MERROR, "Identify", "%s: unexpected value of epcq page: %d", fOdbName.c_str(), fEpcqPage);
         return false;
      }

      return true;
   }

   bool CheckRebootToUserPagePwbLocked()
   {
      bool enable_boot_from_user_page = false;
      fEq->fOdbEqSettings->RB("PWB/enable_boot_user_page", &enable_boot_from_user_page, true);

      bool boot_from_user_page = false;
      fEq->fOdbEqSettings->RBAI("PWB/per_pwb_slot/boot_user_page", fOdbIndex, &boot_from_user_page);

      if (boot_from_user_page != fUserPage) {
         if (enable_boot_from_user_page && boot_from_user_page) {
            return true;
         }
      }
      
      return false;
   }

   bool fUseSataTrigger = false;

   bool PingPwbLocked()
   {
      if (fEsper->fFailed) {
         fEsper->fFailed = false;
         //} else {
         //fLastErrmsg = "";
      }

      std::string elf_buildtime = fEsper->Read(fMfe, "board", "elf_buildtime", &fLastErrmsg);
      if (elf_buildtime.length() > 0)
         return true;
      else
         return false;
   }

   bool IdentifyPwbLocked()
   {
      assert(fEsper);

      if (fEsper->fFailed) {
         fEsper->fFailed = false;
      } else {
         fLastErrmsg = "";
      }

      std::string elf_buildtime = fEsper->Read(fMfe, "board", "elf_buildtime", &fLastErrmsg);

      if (fEsper->fFailed) {
         fCheckId.Fail("esper failure");
         return false;
      }

      if (!(elf_buildtime.length() > 0)) {
         fCheckId.Fail("cannot read board.elf_buildtime");
         return false;
      }

      std::string sw_qsys_ts = fEsper->Read(fMfe, "board", "sw_qsys_ts", &fLastErrmsg);

      if (!(sw_qsys_ts.length() > 0)) {
         fCheckId.Fail("cannot read board.sw_qsys_ts");
         return false;
      }

      std::string hw_qsys_ts = fEsper->Read(fMfe, "board", "hw_qsys_ts", &fLastErrmsg);

      if (!(hw_qsys_ts.length() > 0)) {
         fCheckId.Fail("cannot read board.hw_qsys_ts");
         return false;
      }

      std::string quartus_buildtime = fEsper->Read(fMfe, "board", "quartus_buildtime", &fLastErrmsg);

      if (quartus_buildtime.length() > 0) {
         // good
      } else {
         fMfe->Msg(MERROR, "Identify", "%s: cannot read board.quartus_buildtime", fOdbName.c_str());
         fCheckId.Fail("incompatible firmware, no board.quartus_buildtime");
         return false;
      }

      fUserPage = false;
      fEpcqPage = 0;

      std::string image_location_str = fEsper->Read(fMfe, "update", "image_location", &fLastErrmsg);
      
      if (!(image_location_str.length() > 0)) {
         fCheckId.Fail("cannot read update.image_location");
         return false;
      }
      
      fEpcqPage = xatoi(image_location_str.c_str());
      
      if (fEpcqPage == 0) {
         // factory page
         fUserPage = false;
      } else if (fEpcqPage == 16777216) {
         // user page
         fUserPage = true;
      } else {
         fMfe->Msg(MERROR, "Identify", "%s: unexpected value of update.image_location: %s", fOdbName.c_str(), image_location_str.c_str());
         fCheckId.Fail("incompatible firmware, update.image_location: " + image_location_str);
         return false;
      }

      uint32_t elf_ts = xatoi(elf_buildtime.c_str());
      uint32_t qsys_sw_ts = xatoi(sw_qsys_ts.c_str());
      uint32_t qsys_hw_ts = xatoi(hw_qsys_ts.c_str());
      uint32_t sof_ts = xatoi(quartus_buildtime.c_str());

      fMfe->Msg(MLOG, "Identify", "%s: firmware: elf 0x%08x, qsys_sw 0x%08x, qsys_hw 0x%08x, sof 0x%08x, epcq page %d", fOdbName.c_str(), elf_ts, qsys_sw_ts, qsys_hw_ts, sof_ts, fUserPage);

      fHaveBootLoadOnly = false;
      fHaveHwUdp = false;
      fHaveChangeDelays = true;
      fHaveDataSuppression = false;
      fHaveSataLink = false;
      fHaveChannelBitmap = false;

      if (elf_ts == 0xdeaddead) {
         fHaveBootLoadOnly = true;
      } else if (elf_ts == 0x59cc3664) { // has clock select, no udp
         fHaveBootLoadOnly = true;
      } else if (elf_ts == 0x59f227ec) { // has udp, but no clock select
         fHaveBootLoadOnly = true;
      } else if (elf_ts == 0x5a1de902) { // current good, bad data alignement
      } else if (elf_ts == 0x5a2850a5) { // current good
      } else if (elf_ts == 0x5a5d21a8) { // K.O. build
      } else if (elf_ts == 0x5a7ce8c5) { // B.Shaw UDP
         fHaveBootLoadOnly = true;
      } else if (elf_ts == 0x5aa1aef3) { // B.Shaw UDP
         fHaveBootLoadOnly = true;
      } else if (elf_ts == 0x5aa70a15) { // B.Shaw UDP
         fHaveBootLoadOnly = true;
      } else if (elf_ts == 0x5ab342a2) { // B.Shaw UDP
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
      } else if (elf_ts == 0x5af36d6d) { // B.Shaw UDP
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
      } else if (elf_ts == 0x5ace807b) { // feam-2018-04-06-bootloader
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
      } else if (elf_ts == 0x5afb85b2) { // feam-2018-05-16-test
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
      } else if (elf_ts == 0x5b1043e7) { // pwb_rev1_20180531_cabf9d3d_bryerton
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
      } else if (elf_ts == 0x5b21bc40) { // test
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
      } else if (elf_ts == 0x5b2ad5f8) { // test
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
      } else if (elf_ts == 0x5b352678) { // better link status detection
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;                  // triggers passed over the backup link
         fHaveDataSuppression = true;
      } else if (elf_ts == 0x5b6b5a91) { // pwb_rev1_20180808_0f5edf1b_bryerton
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveDataSuppression = false;
      } else if (elf_ts == 0x5b984b33) { // pwb_rev1_20180912_6c3810a7_bryerton
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
      } else if (elf_ts == 0x5b9ad3d5) { // pwb_rev1_20180913_a8b51569_bryerton
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
      } else if (elf_ts == 0x5d8252dd) { // KO test
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
      } else if (elf_ts == 0x5d854f78) { // pwb_rev1_20191007_ko, eth flow control enabled
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
      } else if (elf_ts == 0x5da2612b) { // KO test, eth flow control enabled, faster DDR read
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
      } else if (elf_ts == 0x5db8ad7a) { // KO test, eth flow control enabled, faster DDR read
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
      } else if (elf_ts == 0x5dba1894) { // KO test, eth flow control enabled, faster DDR read
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
      } else if (elf_ts == 0x5dbcde0e) { // pwb_rev1_20191101_ko, eth flow control enabled, faster DDR read
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
      } else if (elf_ts == 0x5dc1e7b8) { // sata link tests
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
      } else if (elf_ts == 0x5dc30d95) { // sata link tests
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
      } else if (elf_ts == 0x5dc4addb) { // sata link tests, udp xlink, eth xlink
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
      } else if (elf_ts == 0x5dc4d01d) { // sata link tests, udp xlink, eth xlink
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
      } else if (elf_ts == 0x5dc60a7e) { // sata link tests, udp xlink, eth xlink
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
      } else if (elf_ts == 0x5dc61dff) { // sata link tests, udp xlink, eth xlink
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
      } else if (elf_ts == 0x5dc6c9fe) { // sata link tests, udp xlink, eth xlink
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
      } else if (elf_ts == 0x5dc6d402) { // sata link tests, udp xlink, eth xlink
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
      } else if (elf_ts == 0x5dcb21db) { // sata link tests, udp xlink, eth xlink
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
      } else if (elf_ts == 0x5dcde1ad) { // sata link tests, udp xlink, eth xlink
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
      } else if (elf_ts == 0x5dd5d3f3) { // 100MHz
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
      } else if (elf_ts == 0x5DD87F47) { // pwb_rev1_20191125_ko, 100MHz, infinite dhcp
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
      } else if (elf_ts == 0x5E39FDE0) { // short bitmap for channel enable and channel force
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
         fHaveChannelBitmap = true;
      } else if (elf_ts == 0x5e3c6cdf) { // short bitmap for channel enable and channel force
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
         fHaveChannelBitmap = true;
      } else if (elf_ts == 0x5e45c231) { // debug 128-bit align of memory slots
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
         fHaveChannelBitmap = true;
      } else if (elf_ts == 0x5e45da83) { // 128-bit align the memory slots
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
         fHaveChannelBitmap = true;
      } else if (elf_ts == 0x5e45e286) { // 128-bit align the memory slots
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
         fHaveChannelBitmap = true;
      } else if (elf_ts == 0x5e470576) { // debug sata link
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
         fHaveChannelBitmap = true;
      } else if (elf_ts == 0x5e4749cc) { // debug sata link
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
         fHaveChannelBitmap = true;
      } else if (elf_ts == 0x5e4da1c4) { // debug watchdog
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
         fHaveChannelBitmap = true;
      } else if (elf_ts == 0x5e4db559) { // debug watchdog
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
         fHaveChannelBitmap = true;
      } else if (elf_ts == 0x5e4f4c1e) { // debug watchdog
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
         fHaveChannelBitmap = true;
      } else if (elf_ts == 0x5e503c69) { // pwb_rev1_20200221_ko, working watchdog
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
         fHaveChannelBitmap = true;
      } else if (elf_ts == 0x5f4844b3) { // do not switch to ext clock, random delay on boot, no hang of jtag uart when fifo full
         fHaveHwUdp = true;
         fHaveDataSuppression = true;
         fHaveSataLink = true;
         fHaveChannelBitmap = true;
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
      } else if (sof_ts == 0x5a7ce8fa) {
         fHaveBootLoadOnly = true;
      } else if (sof_ts == 0x5aa1998f) {
         fHaveBootLoadOnly = true;
      } else if (sof_ts == 0x5aa70240) {
         fHaveBootLoadOnly = true;
      } else if (sof_ts == 0x5ab342c1) {
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
      } else if (sof_ts == 0x5af36d74) {
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
      } else if (sof_ts == 0x5ace8094) { // feam-2018-04-06-bootloader
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
      } else if (sof_ts == 0x5afb85b9) { // feam-2018-05-16-test
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
      } else if (sof_ts == 0x5b1043f0) { // pwb_rev1_20180531_cabf9d3d_bryerton
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
      } else if (sof_ts == 0x5b21aa9d) { // test
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
      } else if (sof_ts == 0x5b2aca45) { // test
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
      } else if (sof_ts == 0x5b352797) { // better link status detection
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;                  // triggers passed over the backup link
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5b6b5a9a) { // pwb_rev1_20180808_0f5edf1b_bryerton
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
         //fHaveBootLoadOnly = true;
      } else if (sof_ts == 0x5b984b3d) { // pwb_rev1_20180912_6c3810a7_bryerton
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5b9ad3de) { // pwb_rev1_20180913_a8b51569_bryerton
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5d8252e8) { // KO test
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5d854f88) { // KO test, eth flow control enabled
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5d97f86a) { // KO signaltap test, eth flow control enabled
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5d9b7bbb) { // KO signaltap test, eth flow control enabled
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5d9bd33c) { // pwb_rev1_20191007_ko
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5da3db35) { // faster DDR3 read
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5db8b381) { // faster DDR3 read
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5db8d637) { // faster DDR3 read
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dba1b8a) { // faster DDR3 read
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dba407a) { // faster DDR3 read
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dbb783d) { // faster DDR3 read
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dbcf9dc) { // pwb_rev1_20191101_ko, faster DDR3 read
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5d9bd33c) { // sata link tests
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dc0cc94) { // sata link tests
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dc21582) { // sata link tests
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dc30dbb) { // sata link tests
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dc4aea0) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dc5ba58) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dc62533) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dcb23c5) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dcb3c1b) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dcbaac2) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dcc784a) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dcc8b3a) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dccd195) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dcdc3ba) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dcde1ad) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dce0e10) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dcf49cc) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dd33d14) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dd427cb) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dd470f5) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dd4900f) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dd4abc5) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dd4eee5) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dd5bb94) { // sata link tests, udp xlink, eth xlink
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5dd5d4fb) { // 100MHz
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5DD5FE28) { // 100MHz
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5DD61E92) { // 100MHz
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5DD6CE0A) { // 100MHz
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5DD726AC) { // 100MHz
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5DD8281F) { // 100MHz, cleanup sata link
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5ddae7bf) { // 100MHz, cleanup sata link
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5DDAF8E2) { // 100MHz, cleanup sata link
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5DDBFC84) { // pwb_rev1_20191125_ko, 100MHz, cleanup sata link, re-add sata link ctrl enable and disable bits
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5e3089b4) { // fix bouncing of link_status, 10 sec
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5e30b783) { // pwb_rev1_20200128_ko, fix bouncing of link_status, 1 sec
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5e33234b) { // adc test mode
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5e334922) { // adc test mode sequential pattern
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
      } else if (sof_ts == 0x5E39FE31) { // short bitmap for channel enable and channel force
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
         fHaveChannelBitmap = true;
      } else if (sof_ts == 0x5e3b6661) { // DDR signaltap
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
         fHaveChannelBitmap = true;
      } else if (sof_ts == 0x5e3c721f) { // DDR signaltap
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
         fHaveChannelBitmap = true;
      } else if (sof_ts == 0x5e3cbf75) { // debug data suppression
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
         fHaveChannelBitmap = true;
      } else if (sof_ts == 0x5E4441C0) { // additional test modes
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
         fHaveChannelBitmap = true;
      } else if (sof_ts == 0x5e44ca4b) { // additional test modes
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
         fHaveChannelBitmap = true;
      } else if (sof_ts == 0x5e45e3e6) { // debug sata link
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
         fHaveChannelBitmap = true;
      } else if (sof_ts == 0x5e4605ad) { // debug sata link
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
         fHaveChannelBitmap = true;
      } else if (sof_ts == 0x5e472d2b) { // add ethernet mux output timeout
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
         fHaveChannelBitmap = true;
      } else if (sof_ts == 0x5e5083b7) { // pwb_rev1_20200221_ko, rebuild
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
         fHaveChannelBitmap = true;
      } else if (sof_ts == 0x5effd177) { // network test
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
         fHaveChannelBitmap = true;
      } else if (sof_ts == 0x5f03ac42) { // network test
         fHaveBootLoadOnly = true;
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
         fHaveChannelBitmap = true;
      } else if (sof_ts == 0x5f03e652) { // pwb_rev1_20200706_ko, udp delay, new channel suppression
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
         fHaveChannelBitmap = true;
      } else if (sof_ts == 0x5f4969b0) { // udp delay expanded from 16 to 20 bits
         fHaveHwUdp = true;
         fHaveChangeDelays = false;
         fHaveSataTrigger = true;
         fHaveChannelBitmap = true;
      } else {
         fMfe->Msg(MERROR, "Identify", "%s: firmware is not compatible with the daq, sof quartus_buildtime  0x%08x", fOdbName.c_str(), sof_ts);
         fCheckId.Fail("incompatible firmware, quartus_buildtime: " + quartus_buildtime);
         return false;
      }

      if (fHaveBootLoadOnly) {
         fMfe->Msg(MERROR, "Identify", "%s: firmware is not compatible with the daq, usable as boot loader only", fOdbName.c_str());
         fCheckId.Fail("boot loader only");
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

      DWORD t0 = ss_millitime();

      fEq->fOdbEqSettings->RI("PeriodPwb", &fConfPollSleep, true);

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
      // clockcleaner/pll1_wnd_size values:
      //
      // 0 = 5.5 ns
      // 1 = 10 ns
      // 2 = 18.6 ns
      // 3 = 40 ns
      //
      int pll1_wnd_size = 3;

      //
      // signalproc/trig_delay
      //
      int trig_delay = 312;
      int sata_trig_delay = 275;

      // 
      // sca/gain values:
      //
      // 0 = 120 fC
      // 1 = 240 fC
      // 2 = 360 fC
      // 3 = 600 fC
      //
      int sca_gain = 0;

      //
      // sca number of samples is 511
      //
      int sca_samples = 0;

      //
      // signalproc/sca_X_ch_threshold
      //
      int ch_threshold = 1;
      bool ch_enable = true;
      bool ch_force_global = false;
      bool ch_force_pwb = false;
      int start_delay = 13;
      int sca_ddelay = 200;

      bool suppress_reset = false;
      bool disable_reset1 = false;
      bool suppress_fpn   = false;
      bool suppress_pads  = false;

      double baseline_reset = 0;
      double baseline_fpn = 0;
      double baseline_pads = 0;

      double threshold_reset = 0;
      double threshold_fpn = 0;
      double threshold_pads = 0;

      bool udp_delay_enable = false;
      int  udp_delay_value  = 0;

      bool enable_test_mode = false;
      int test_mode = 0;
      int supp_mode = 0;

      fEq->fOdbEqSettings->RI("PWB/clkin_sel",     &clkin_sel, true);
      fEq->fOdbEqSettings->RI("PWB/pll1_wnd_size", &pll1_wnd_size, true);
      fEq->fOdbEqSettings->RI("PWB/trig_delay",    &trig_delay, true);
      fEq->fOdbEqSettings->RI("PWB/sata_trig_delay", &sata_trig_delay, true);
      fEq->fOdbEqSettings->RI("PWB/sca_gain",    &sca_gain, true);
      fEq->fOdbEqSettings->RI("PWB/sca_samples", &sca_samples, true);

      fEq->fOdbEqSettings->RB("PWB/enable_test_mode", &enable_test_mode, true);
      fEq->fOdbEqSettings->RI("PWB/test_mode", &test_mode, true);
      fEq->fOdbEqSettings->RI("PWB/supp_mode", &supp_mode, true);

      fEq->fOdbEqSettings->RB("PWB/ch_enable",    &ch_enable, true);
      fEq->fOdbEqSettings->RI("PWB/ch_threshold", &ch_threshold, true);
      fEq->fOdbEqSettings->RB("PWB/ch_force",     &ch_force_global, true);
      fEq->fOdbEqSettings->RBAI("PWB/per_pwb_slot/ch_force", fOdbIndex, &ch_force_pwb);

      fEq->fOdbEqSettings->RB("PWB/disable_reset1", &disable_reset1, true);

      fEq->fOdbEqSettings->RB("PWB/suppress_reset", &suppress_reset, true);
      fEq->fOdbEqSettings->RB("PWB/suppress_fpn",   &suppress_fpn, true);
      fEq->fOdbEqSettings->RB("PWB/suppress_pads",  &suppress_pads, true);

      fEq->fOdbEqSettings->RDAI("PWB/per_pwb_slot_thr/baseline_reset", fOdbIndex, &baseline_reset);
      fEq->fOdbEqSettings->RDAI("PWB/per_pwb_slot_thr/baseline_fpn",   fOdbIndex, &baseline_fpn);
      fEq->fOdbEqSettings->RDAI("PWB/per_pwb_slot_thr/baseline_pads",  fOdbIndex, &baseline_pads);

      fEq->fOdbEqSettings->RD("PWB/threshold_reset", &threshold_reset, true);
      fEq->fOdbEqSettings->RD("PWB/threshold_fpn",   &threshold_fpn, true);
      fEq->fOdbEqSettings->RD("PWB/threshold_pads",  &threshold_pads, true);

      fEq->fOdbEqSettings->RI("PWB/start_delay", &start_delay, true);
      fEq->fOdbEqSettings->RI("PWB/sca_ddelay",  &sca_ddelay, true);

      fEq->fOdbEqSettings->RB("PWB/udp_delay_enable", &udp_delay_enable, true);
      fEq->fOdbEqSettings->RI("PWB/udp_delay_value",  &udp_delay_value, true);

      int udp_port = 0;

      //fMfe->fOdbRoot->RI("Equipment/XUDP/Settings/udp_port_pwb", 0, &udp_port, false);

      static int x=0;
      x++;
      if ((x%4) == 0)
         fMfe->fOdbRoot->RI("Equipment/PWB_A_UDP/Settings/udp_port", &udp_port);
      else if ((x%4) == 1)
         fMfe->fOdbRoot->RI("Equipment/PWB_B_UDP/Settings/udp_port", &udp_port);
      else if ((x%4) == 2)
         fMfe->fOdbRoot->RI("Equipment/PWB_C_UDP/Settings/udp_port", &udp_port);
      else if ((x%4) == 3)
         fMfe->fOdbRoot->RI("Equipment/PWB_D_UDP/Settings/udp_port", &udp_port);
      else
         fMfe->fOdbRoot->RI("Equipment/PWB_A_UDP/Settings/udp_port", &udp_port);

      bool enable_trigger = false;
      fEq->fOdbEqSettings->RB("PWB/enable_trigger", &enable_trigger, true);

      bool enable_trigger_group_a = true;
      fEq->fOdbEqSettings->RB("PWB/enable_trigger_group_a", &enable_trigger_group_a, true);

      bool enable_trigger_group_b = true;
      fEq->fOdbEqSettings->RB("PWB/enable_trigger_group_b", &enable_trigger_group_b, true);

      int pwb_column = fOdbIndex/8;
      bool enable_trigger_column = false;
      fEq->fOdbEqSettings->RBAI("PWB/enable_trigger_column", pwb_column, &enable_trigger_column);

      bool trigger = false;
      fEq->fOdbEqSettings->RBAI("PWB/per_pwb_slot/trigger", fOdbIndex, &trigger);

      fEq->fOdbEqSettings->RBAI("PWB/per_pwb_slot/sata_trigger", fOdbIndex, &fUseSataTrigger);

      bool use_sata_clock = false;
      fEq->fOdbEqSettings->RBAI("PWB/per_pwb_slot/sata_clock", fOdbIndex, &use_sata_clock);

      if (use_sata_clock) {
         if (fSataLinkMaster) { // "MTC" configuration cannot run on sata clock!
            // NB: on restart of fectrl, this code with bump the clock -
            // because the mate is being configured, we will be unhappy here
            // and set the clock to internal oscillator even if it is already
            // set to sata clock. I am not changing this, better be safe than fast. K.O. aug 2020.
            //std::string x_clkin_sel_string = fEsper->Read(fMfe, "clockcleaner", "clkin_sel");
            //fMfe->Msg(MLOG, "ConfigurePwbLocked", "%s: MTC clock is [%s]", fOdbName.c_str(), x_clkin_sel_string.c_str());

            PwbCtrl* mate = FindPwbMate(this);
            if (mate) {
               if (mate->fState == ST_GOOD) {
                  // if sata link mate is in good state, we can try to use it's clock
                  clkin_sel = 1; // SATA clock
                  fMfe->Msg(MLOG, "ConfigurePwbLocked", "%s: sata link mate \"%s\" is ready, let's use the sata clock", fOdbName.c_str(), mate->fOdbName.c_str());
               } else {
                  // sata link mate down or not fully initialized yet,
                  // so we stay with our internal oscillator
                  clkin_sel = 2; // internal oscillator
                  fMfe->Msg(MLOG, "ConfigurePwbLocked", "%s: sata link mate \"%s\" not ready, we stay with the local oscillator", fOdbName.c_str(), mate->fOdbName.c_str());
               }
            } else {
               // misconfiguration: we are a sata link master, but cannot find sata mate
               // so we stay with our internal oscillator
               clkin_sel = 2; // internal oscillator
               fMfe->Msg(MLOG, "ConfigurePwbLocked", "%s: cannot find sata link mate, we stay with the local oscillator", fOdbName.c_str());
            }
         } else {
            clkin_sel = 1; // SATA clock
         }
      }

      bool group_a = false;
      bool group_b = false;

      int mid_group = 32;

      if (fOdbIndex < mid_group+2)
         group_a = true;

      if (fOdbIndex >= mid_group-2)
         group_b = true;

      bool trigger_a = (group_a && enable_trigger_group_a);
      bool trigger_b = (group_b && enable_trigger_group_b);

      bool ch_force = ch_force_global || ch_force_pwb;

      fConfTrigger = enable_trigger && enable_trigger_column && trigger && (trigger_a || trigger_b);;

      fMfe->Msg(MLOG, "ConfigurePwbLocked", "%s: configure: clkin_sel %d, trig_delay %d, sca gain %d, sca_samples %d, ch_enable %d, ch_threshold %d, ch_force %d, start_delay %d, udp_port %d, trigger %d", fOdbName.c_str(), clkin_sel, trig_delay, sca_gain, sca_samples, ch_enable, ch_threshold, ch_force, start_delay, udp_port, fConfTrigger);

      DWORD t1 = ss_millitime();

      // make sure everything is stopped

      ok &= StopPwbLocked();

      DWORD t2 = ss_millitime();

      // before switching clocks, set pll1_wnd_size

      std::string x_pll1_wnd_size_string = fEsper->Read(fMfe, "clockcleaner", "pll1_wnd_size");
      int x_pll1_wnd_size = xatoi(x_pll1_wnd_size_string.c_str());

      if (x_pll1_wnd_size != pll1_wnd_size) {
         printf("%s: pll1_wnd_size: [%s] %d should be %d\n", fOdbName.c_str(), x_pll1_wnd_size_string.c_str(), x_pll1_wnd_size, pll1_wnd_size);

         fMfe->Msg(MLOG, "ConfigurePwbLocked", "%s: configure: switching pll1_wnd_size from %d to %d", fOdbName.c_str(), x_pll1_wnd_size, pll1_wnd_size);

         ok &= fEsper->Write(fMfe, "clockcleaner", "pll1_wnd_size", toString(pll1_wnd_size).c_str());

         if (!ok) {
            fMfe->Msg(MERROR, "ConfigurePwbLocked", "%s: configure failed on write to clock cleaner pll1_wnd_size", fOdbName.c_str());
            return false;
         }
      }

      // switch clock to external clock

      std::string x_clkin_sel_string = fEsper->Read(fMfe, "clockcleaner", "clkin_sel");
      int x_clkin_sel = xatoi(x_clkin_sel_string.c_str());

      if (x_clkin_sel != clkin_sel) {
         printf("%s: clkin_sel: [%s] %d should be %d\n", fOdbName.c_str(), x_clkin_sel_string.c_str(), x_clkin_sel, clkin_sel);

         fMfe->Msg(MLOG, "ConfigurePwbLocked", "%s: configure: switching the clock source clkin_sel from %d to %d", fOdbName.c_str(), x_clkin_sel, clkin_sel);

         ok &= fEsper->Write(fMfe, "clockcleaner", "clkin_sel", toString(clkin_sel).c_str());

         if (!ok) {
            fMfe->Msg(MERROR, "ConfigurePwbLocked", "%s: configure failed on write to clock cleaner clkin_sel", fOdbName.c_str());
            return false;
         }
      }

      DWORD t3 = ss_millitime();

      // change delays

      if (fHaveHwUdp && fHaveChangeDelays) {
         std::string s_start_delay = "";
         s_start_delay += "[";
         s_start_delay += toString(start_delay);
         s_start_delay += ",";
         s_start_delay += toString(start_delay);
         s_start_delay += ",";
         s_start_delay += toString(start_delay);
         s_start_delay += ",";
         s_start_delay += toString(start_delay);
         s_start_delay += "]";
         //printf("writing %s\n", s_start_delay.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "start_delay", s_start_delay.c_str());
         ok &= fEsper->Write(fMfe, "clockcleaner", "sca_ddelay", toString(sca_ddelay).c_str());
      }

      // configure the trigger

      if (fHaveHwUdp) {
         ok &= fEsper->Write(fMfe, "trigger", "ext_trig_delay", toString(trig_delay).c_str());
         if (fHaveSataTrigger) {
            ok &= fEsper->Write(fMfe, "trigger", "link_trig_delay", toString(sata_trig_delay).c_str());
         }
      } else {
         ok &= fEsper->Write(fMfe, "signalproc", "trig_delay", toString(trig_delay).c_str());
      }

      // configure the SCAs

      ok &= fEsper->Write(fMfe, "sca0", "gain", toString(sca_gain).c_str());
      ok &= fEsper->Write(fMfe, "sca1", "gain", toString(sca_gain).c_str());
      ok &= fEsper->Write(fMfe, "sca2", "gain", toString(sca_gain).c_str());
      ok &= fEsper->Write(fMfe, "sca3", "gain", toString(sca_gain).c_str());

      // set number of samples

      if (sca_samples > 0) {
         ok &= fEsper->Write(fMfe, "signalproc", "sca_samples", toString(sca_samples).c_str());
      }

      // set test mode

      if (enable_test_mode) {
         fMfe->Msg(MLOG, "ConfigurePwbLocked", "%s: configure: enabled ADC test pattern mode", fOdbName.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "test_mode", "true");
      } else {
         ok &= fEsper->Write(fMfe, "signalproc", "test_mode", "false");
      }

      DWORD t4 = ss_millitime();

      // configure channel suppression

      if (fHaveChannelBitmap) {
         std::string sch_enable = "";
         std::string sch_force = "";

         uint32_t ch_a_ctrl = 0;
         uint32_t ch_b_ctrl = 0;
         uint32_t ch_c_ctrl = 0;
         uint32_t ch_d_ctrl = 0;

         ch_a_ctrl |= (ch_threshold & 0xFFF) << 0;
         ch_b_ctrl |= (ch_threshold & 0xFFF) << 0;
         ch_c_ctrl |= (ch_threshold & 0xFFF) << 0;
         ch_d_ctrl |= (ch_threshold & 0xFFF) << 0;

         if (enable_test_mode) {
            ch_a_ctrl |= ((test_mode & 0x7) << 12);
            ch_b_ctrl |= ((test_mode & 0x7) << 12);
            ch_c_ctrl |= ((test_mode & 0x7) << 12);
            ch_d_ctrl |= ((test_mode & 0x7) << 12);
         }

         ch_a_ctrl |= ((supp_mode & 0x1) << 15);
         ch_b_ctrl |= ((supp_mode & 0x1) << 15);
         ch_c_ctrl |= ((supp_mode & 0x1) << 15);
         ch_d_ctrl |= ((supp_mode & 0x1) << 15);

         //fMfe->Msg(MLOG, "ConfigurePwbLocked", "%s: configure: test mode %d and %d, ctrl 0x%08x", fOdbName.c_str(), enable_test_mode, test_mode, ch_a_ctrl);

         bool ach_enable[3*32];
         bool ach_force[3*32];

         memset(ach_enable, 0, sizeof(ach_enable));
         memset(ach_force, 0, sizeof(ach_force));

         assert(79 <= 3*32); // yes. ensure array size.

         for (int ri=1; ri<=79; ri++) {
            bool xch_enable = false;
            bool xch_force = false;

            xch_enable |= ch_enable;
            xch_force |= ch_force;

            if (ri == 1 || ri == 2 || ri == 3) { // reset channels
               if (ri == 1 && disable_reset1) {
                  xch_enable = false;
                  xch_force = false;
               } else {
                  xch_force |= !suppress_reset;
               }
            } else if (ri == 16 || ri == 29 || ri == 54 || ri == 67) { // FPN channels
               xch_force |= (!suppress_fpn);
            } else {
               xch_force |= (!suppress_pads);
            }

            ach_enable[ri] = xch_enable;
            ach_force[ri]  = xch_force;
         }

         sch_enable += "[";
         sch_force += "[";

         int ri = 1;
         for (int i=0; i<3; i++) {
            if (i>0) {
               sch_enable += ",";
               sch_force += ",";
            }

            uint32_t xenable = 0;
            uint32_t xforce  = 0;

            for (int j=0; j<32; j++) {
               if (ach_enable[ri])
                  xenable |= (1<<j);
               if (ach_force[ri])
                  xforce  |= (1<<j);
               ri++;
            }

            sch_enable += toHexString(xenable);
            sch_force  += toHexString(xforce);
         }

         sch_force += "]";
         sch_enable += "]";

         ok &= fEsper->Write(fMfe, "signalproc", "sca_a_ch_enable_bitmap", sch_enable.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_b_ch_enable_bitmap", sch_enable.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_c_ch_enable_bitmap", sch_enable.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_d_ch_enable_bitmap", sch_enable.c_str());

         ok &= fEsper->Write(fMfe, "signalproc", "sca_a_ch_force_bitmap", sch_force.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_b_ch_force_bitmap", sch_force.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_c_ch_force_bitmap", sch_force.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_d_ch_force_bitmap", sch_force.c_str());

         ok &= fEsper->Write(fMfe, "signalproc", "sca_a_ch_ctrl", toString(ch_a_ctrl).c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_b_ch_ctrl", toString(ch_b_ctrl).c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_c_ch_ctrl", toString(ch_c_ctrl).c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_d_ch_ctrl", toString(ch_d_ctrl).c_str());
      } else if (fHaveDataSuppression) {
         std::string sch_enable = "";
         std::string sch_force = "";
         std::string sch_threshold = "";

         sch_enable += "[";
         sch_force += "[";
         sch_threshold += "[";

         for (int ri=1; ri<=79; ri++) {
            bool xch_enable = false;
            bool xch_force = false;

            xch_enable |= ch_enable;
            xch_force |= ch_force;

            int xch_threshold = 0;

            if (ri == 1 || ri == 2 || ri == 3) { // reset channels
               if (ri == 1 && disable_reset1) {
                  xch_enable = false;
                  xch_force = false;
                  xch_threshold = 0;
               } else {
                  xch_force |= !suppress_reset;
                  xch_threshold = baseline_reset - threshold_reset;
               }
            } else if (ri == 16 || ri == 29 || ri == 54 || ri == 67) { // FPN channels
               xch_force |= (!suppress_fpn);
               xch_threshold = baseline_fpn - threshold_fpn;
            } else {
               xch_force |= (!suppress_pads);
               xch_threshold = baseline_pads - threshold_pads;
            }

            if (ri>0)
               sch_enable += ",";

            sch_enable += boolToString(xch_enable);

            if (ri>1)
               sch_force += ",";

            sch_force += boolToString(xch_force);

            if (ch_threshold != 0)
               xch_threshold = ch_threshold;

            if (ri>1)
               sch_threshold += ",";
            sch_threshold += toString(xch_threshold);
         }

         sch_threshold += "]";
         sch_force += "]";
         sch_enable += "]";

         ok &= fEsper->Write(fMfe, "signalproc", "sca_a_ch_enable", sch_enable.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_b_ch_enable", sch_enable.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_c_ch_enable", sch_enable.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_d_ch_enable", sch_enable.c_str());

         ok &= fEsper->Write(fMfe, "signalproc", "sca_a_ch_force", sch_force.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_b_ch_force", sch_force.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_c_ch_force", sch_force.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_d_ch_force", sch_force.c_str());

         ok &= fEsper->Write(fMfe, "signalproc", "sca_a_ch_threshold", sch_threshold.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_b_ch_threshold", sch_threshold.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_c_ch_threshold", sch_threshold.c_str());
         ok &= fEsper->Write(fMfe, "signalproc", "sca_d_ch_threshold", sch_threshold.c_str());
      }

      DWORD t5 = ss_millitime();

      // program the IP address and port number in the UDP transmitter

      if (fHaveHwUdp) {
         //fMfe->Msg(MLOG, "ConfigurePwbLocked", "%s: configuring UDP", fOdbName.c_str());

         if (udp_port == 0) {
            fMfe->Msg(MLOG, "ConfigurePwbLocked", "%s: error configuring UDP: invalid UDP port %d", fOdbName.c_str(), udp_port);
            return false;
         }

         fSataLinkSlave = false;
         fSataLinkMaster = false;
         bool sataLinkEth = false;
         //uint32_t sataOffloadIp = 0;
         int sataMate = 0;

         fEq->fOdbEqSettings->RBAI("PWB/per_pwb_slot/sata_master", fOdbIndex, &fSataLinkMaster);
         fEq->fOdbEqSettings->RBAI("PWB/per_pwb_slot/sata_master", fOdbIndex, &sataLinkEth);
         fEq->fOdbEqSettings->RBAI("PWB/per_pwb_slot/sata_slave",  fOdbIndex, &fSataLinkSlave);
         //fEq->fOdbEqSettings->RU32AI("PWB/per_pwb_slot/sata_offload_ip",  fOdbIndex, &sataOffloadIp);
         fEq->fOdbEqSettings->RIAI("PWB/per_pwb_slot/sata_mate",  fOdbIndex, &sataMate);

         //printf("sata_offload_ip[%d] is %d (0x%08x)\n", fOdbIndex, sataOffloadIp, sataOffloadIp);
         //while (1) { ::sleep(1); }

         uint32_t slave_src_ip = 0;
         slave_src_ip |= (192<<24);
         slave_src_ip |= (168<<16);
         slave_src_ip |= (1<<8);
         //slave_src_ip |= (0xFF & sataOffloadIp);
         slave_src_ip |= (100 + sataMate%100);

         int slave_dst_port = udp_port;

         int udp_ip = 0;
         udp_ip |= (192<<24);
         udp_ip |= (168<<16);
         udp_ip |= (1<<8);
         udp_ip |= (1<<0);

         ok &= fEsper->Write(fMfe, "offload", "enable", "false");

         if (fSataLinkSlave) {
            // disable UDP offload to ethernet
            ok &= fEsper->Write(fMfe, "offload", "dst_ip", "0");
            ok &= fEsper->Write(fMfe, "offload", "dst_port", "0");
         } else {
            // normal configuration: UDP offload to ethernet
            ok &= fEsper->Write(fMfe, "offload", "dst_ip", toString(udp_ip).c_str());
            ok &= fEsper->Write(fMfe, "offload", "dst_port", toString(udp_port).c_str());
            ok &= fEsper->Write(fMfe, "offload", "enable", "true");
         }

         if (fSataLinkMaster) {
            ok &= fEsper->Write(fMfe, "offload_sata", "enable", "false");
            ok &= fEsper->Write(fMfe, "offload_sata", "src_ip", toString(slave_src_ip).c_str());
            ok &= fEsper->Write(fMfe, "offload_sata", "dst_ip", toString(udp_ip).c_str());
            ok &= fEsper->Write(fMfe, "offload_sata", "dst_port", toString(slave_dst_port).c_str());
            ok &= fEsper->Write(fMfe, "offload_sata", "enable", "true");
         } else if (fHaveSataLink) {
            ok &= fEsper->Write(fMfe, "offload_sata", "enable", "false");
            ok &= fEsper->Write(fMfe, "offload_sata", "dst_ip", "0");
            ok &= fEsper->Write(fMfe, "offload_sata", "dst_port", "0");
         }

         // enable the data paths in the link_ctrl register bits:
         //
         // 0x0001 - enable UDP path from sata link channel 2 to offload_sata
         // 0x0002 - disable UDP path from sca to offload
         //

         if (fHaveSataLink) {

            uint32_t link_ctrl = 0;

            link_ctrl |= (1<<12); // disable sata->nios
            link_ctrl |= (1<<13); // disable nios->sata
               
            if (fSataLinkMaster && fSataLinkSlave) {
               // both slave and master throught the sata link loopback
               link_ctrl |= 3;
            } else if (fSataLinkMaster) {
               fMfe->Msg(MLOG, "ConfigurePwbLocked", "%s: configure: enable sata link master mode, mate pwb%02d, slave IP 0x%08x", fOdbName.c_str(), sataMate, slave_src_ip);
               link_ctrl |= (1<<0);  // enable  sata->OFFLOAD_SATA
            } else if (fSataLinkSlave) {
               fMfe->Msg(MLOG, "ConfigurePwbLocked", "%s: configure: enable sata link slave mode", fOdbName.c_str());
               link_ctrl |= (1<<1);  // enable SCA->sata
               link_ctrl &= ~(1<<12); // disable sata->nios
               link_ctrl &= ~(1<<13); // disable nios->sata
            }

            if (sataLinkEth) {
               fMfe->Msg(MLOG, "ConfigurePwbLocked", "%s: configure: enable sata link ethernet bridge mode", fOdbName.c_str());
               link_ctrl |= (1<<2); // enable sata->eth
               link_ctrl |= (1<<3); // enable eth->sata
            }

            if (fSataLinkMaster || fSataLinkSlave || sataLinkEth) {
               link_ctrl |= (1<<4);    // enable flow control stop_our_tx
               link_ctrl |= (1<<6);    // enable flow control stop_remote_tx
            }

            if (udp_delay_enable) {
               link_ctrl |= (1<<10);   // enable delay between udp packets
               link_ctrl |= ((udp_delay_value&0xFF)<<24);   // delay between udp packets in units of 256*16 ns (top 8 bits of a 16-bit counter)
            }

            std::string link_status_str = fEsper->Read(fMfe, "link", "link_status");

            fMfe->Msg(MLOG, "ConfigurePwbLocked", "%s: configure: link status [%s]", fOdbName.c_str(), link_status_str.c_str());

            ok &= fEsper->Write(fMfe, "link", "link_ctrl", toString(link_ctrl).c_str());

            if (fSataLinkSlave) {
               int timeout = fEsper->s->fReadTimeoutMilliSec;
               fEsper->s->fReadTimeoutMilliSec = 10000;
               ok &= fEsper->Write(fMfe, "link", "stop_eth", "true");
               fEsper->s->fReadTimeoutMilliSec = timeout;
            }
         }
      }

      DWORD t6 = ss_millitime();

      // configure MV2
      int mv2enabled, mv2range, mv2res;
      fEq->fOdbEqSettings->RIAI("PWB/per_pwb_slot_mv2/mv2_enabled",    fOdbIndex, &mv2enabled);
      fEq->fOdbEqSettings->RIAI("PWB/per_pwb_slot_mv2/mv2_range",      fOdbIndex, &mv2range);
      fEq->fOdbEqSettings->RIAI("PWB/per_pwb_slot_mv2/mv2_resolution", fOdbIndex, &mv2res);
      
      if( mv2enabled )
         {
            fMfe->Msg(MLOG, "ConfigurePwbLocked", "MV2 Hall Probe enabled on %s, with range %d and resolution %d",
                   fOdbName.c_str(),mv2range,mv2res);
            ok &= fEsper->Write(fMfe, "board", "mv2_enable", "true");
            ok &= fEsper->Write(fMfe, "board", "mv2_range", std::to_string(mv2range).c_str());
            ok &= fEsper->Write(fMfe, "board", "mv2_res", std::to_string(mv2res).c_str());
         }
      else {
         ok &= fEsper->Write(fMfe, "board", "mv2_enable", "false");
      }

      DWORD te = ss_millitime();

      fMfe->Msg(MLOG, "ConfigurePwbLocked", "%s: configure %d in %d ms: odb %d, stop %d, clock %d, config %d, supp %d, udp %d, mv %d", fOdbName.c_str(), ok, te-t0, t1-t0, t2-t1, t3-t2, t4-t3, t5-t4, t6-t5, te-t6);

      return ok;
   }

   bool StartPwbLocked()
   {
      assert(fEsper);
      bool ok = true;
      if (!fConfTrigger) {
         fMfe->Msg(MLOG, "StartPwbLocked", "%s: started, trigger disabled", fOdbName.c_str());
         return ok;
      }
      if (fHaveHwUdp) {
         if (fUseSataTrigger) {
            ok &= fEsper->Write(fMfe, "trigger", "link_trig_ena", "true");
         } else {
            ok &= fEsper->Write(fMfe, "trigger", "ext_trig_ena", "true");
         }
         ok &= fEsper->Write(fMfe, "trigger", "enable_all", "true");
         ok &= fEsper->Write(fMfe, "signalproc", "force_run", "true");
      } else {
         ok &= fEsper->Write(fMfe, "signalproc", "ext_trig_ena", "true");
         ok &= fEsper->Write(fMfe, "signalproc", "force_run", "true");
      }
      fMfe->Msg(MLOG, "StartPwbLocked", "%s: started", fOdbName.c_str());
      return ok;
   }

   bool StopPwbLocked()
   {
      assert(fEsper);
      bool ok = true;
      ok &= fEsper->Write(fMfe, "signalproc", "force_run", "false");
      if (fHaveHwUdp) {
         ok &= fEsper->Write(fMfe, "trigger", "enable_all", "false");
         ok &= fEsper->Write(fMfe, "trigger", "ext_trig_ena", "false");
         ok &= fEsper->Write(fMfe, "trigger", "man_trig_ena", "false");
         ok &= fEsper->Write(fMfe, "trigger", "intp_trig_ena", "false");
         ok &= fEsper->Write(fMfe, "trigger", "extp_trig_ena", "false");
         if (fHaveSataTrigger) {
            ok &= fEsper->Write(fMfe, "trigger", "link_trig_ena", "false");
         } else {
            ok &= fEsper->Write(fMfe, "trigger", "udp_trig_ena", "false");
         }
      } else {
         ok &= fEsper->Write(fMfe, "signalproc", "ext_trig_ena", "false");
      }
      //fMfe->Msg(MLOG, "StopPwbLocked", "%s: stopped", fOdbName.c_str());
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
      double reboot_start_time = 0;
      double slave_start_time = 0;
      double fast_ping_start_time = 0;
      PwbCtrl* mate = NULL;
      //SetState(ST_INITIAL, "thread...");
      while (!fMfe->fShutdownRequested) {
         int sleep = fConfPollSleep;
         int sleep_slow_ping = 10;
         int sleep_final = 15;
         int sleep_reboot = 1;
         int sleep_read = 5;
         int sleep_fast_ping = 2;
         int fast_ping_timeout = 10;
         int reboot_timeout = 30;
         int slave_timeout = reboot_timeout;
         int wait_configure = 1;
         int wait_first_read = 1;
         {
            std::lock_guard<std::mutex> lock(fLock);
            //printf("%s: state %d\n", fOdbName.c_str(), fState);
            switch (fState) {
            case ST_EMPTY_SLOT_F: sleep = sleep_final; break;
            case ST_INITIAL: SetState(ST_SLOW_PING, "no ping"); sleep = 0; break;
            case ST_SLOW_PING: {
               bool ok = PingPwbLocked();
               if (ok) {
                  SetState(ST_INIT, "init...");
                  sleep = 0;
               } else {
                  sleep = sleep_slow_ping;
               }
               break;
            }
            case ST_INIT: {
               bool ok = IdentifyPwbLocked();
               if (fHaveBootLoadOnly) {
                  bool do_reboot = CheckRebootToUserPagePwbLocked();
                  if (do_reboot) {
                     SetState(ST_REBOOT, "reboot...");
                     sleep = 0;
                  } else {
                     SetState(ST_BAD_CONFIGURE_F, "cannot identify!");
                     sleep = 0;
                  }
               } else if (ok) {
                  bool do_reboot = CheckRebootToUserPagePwbLocked();
                  if (do_reboot) {
                     SetState(ST_REBOOT, "reboot to user page...");
                     sleep = 0;
                  } else {
                     SetState(ST_CONFIGURE, "configure...");
                     sleep = wait_configure;
                  }
               } else {
                  SetState(ST_BAD_IDENTIFY_F, "cannot identify!");
                  sleep = 0;
               }
               break;
            }
            case ST_BAD_IDENTIFY_F: sleep = sleep_final; break;
            case ST_REBOOT: {
               bool ok = RebootToUserPagePwbLocked();
               if (ok) {
                  SetState(ST_REBOOTING, "reboot to user epcq page...");
                  reboot_start_time = TMFE::GetTime();
                  sleep = 0;
               } else {
                  SetState(ST_BAD_REBOOT_F, "cannot reboot to user epcq page!");
                  sleep = 0;
               }
               break;
            }
            case ST_REBOOTING: {
               bool ok = PingPwbLocked();
               if (ok) {
                  ok = IdentifyPwbLocked();
                  if (ok) {
                     bool do_reboot = CheckRebootToUserPagePwbLocked();
                     if (do_reboot) {
                        SetState(ST_BAD_REBOOT_F, "cannot reboot to user epcq page!");
                        sleep = 0;
                     } else {
                        SetState(ST_CONFIGURE, "configure...");
                        sleep = wait_configure;
                     }
                  } else {
                     SetState(ST_BAD_IDENTIFY_F, "cannot identify!");
                     sleep = 0;
                  }
               } else {
                  double now = TMFE::GetTime();
                  if (now - reboot_start_time > reboot_timeout) {
                     SetState(ST_INITIAL, "timeout waiting for reboot...");
                     sleep = 0;
                  } else {
                     sleep = sleep_reboot;
                  }
               }
               break;
            }
            case ST_BAD_REBOOT_F: sleep = sleep_final; break;
            case ST_CONFIGURE: {
               bool ok = ConfigurePwbLocked();
               if (ok) {
                  SetState(ST_FIRST_READ, "configure ok...");
                  sleep = wait_first_read;
                  if (fSataLinkMaster) {
                     mate = FindPwbMate(this);
                     SetState(ST_WAIT_SLAVE, "wait for sata slave...");
                     sleep = 0;
                     slave_start_time = TMFE::GetTime();
                  }
               } else {
                  SetState(ST_BAD_CONFIGURE_F, "cannot configure!");
                  sleep = 0;
               }
               break;
            }
            case ST_BAD_CONFIGURE_F: sleep = sleep_final; break;
            case ST_WAIT_SLAVE: {
               if (!mate) {
                  // why are we here?
                  SetState(ST_FIRST_READ, "first read...");
                  sleep = 0;
                  break;
               }
               if (mate->fState == ST_GOOD || mate->fState == ST_BAD_CHECK) {
                  // sata slave is running
                  SetState(ST_FIRST_READ, "first read...");
                  fCheckId.Ok();
                  sleep = 2;
               } else {
                  double now = TMFE::GetTime();
                  if (now - slave_start_time > slave_timeout) {
                     SetState(ST_FIRST_READ, "first read...");
                     sleep = 0;
                     fMfe->Msg(MERROR, "ThreadPwb", "%s: timeout waiting for sata link slave", fOdbName.c_str());
                     fCheckId.Ok();
                  } else {
                     sleep = 1;
                  }
               }
               break;
            }
            case ST_FIRST_READ: // fall through
            case ST_READ: // fall through
            case ST_GOOD: // fall through
            case ST_BAD_CHECK: {
               EsperNodeData e;
               bool ok = ReadPwbLocked(&e);
               if (ok) {
                  bool first_time = (fState == ST_FIRST_READ);
                  ok = CheckPwbLocked(e, first_time);
                  if (ok) {
                     SetState(ST_GOOD, "ok");
                     sleep = sleep_read;
                  } else {
                     SetState(ST_BAD_CHECK, "check error");
                     sleep = sleep_read;
                  }
               } else {
                  SetState(ST_BAD_READ, "read error");
                  sleep = 0;
               }
               break;
            }
            case ST_BAD_READ: {
               bool ok = PingPwbLocked();
               if (ok) {
                  SetState(ST_READ, "read...");
                  sleep = sleep_read;
               } else {
                  SetState(ST_FAST_PING, "fast ping...");
                  sleep = 0;
                  fast_ping_start_time = TMFE::GetTime();
               }
            }
            case ST_FAST_PING: {
               bool ok = PingPwbLocked();
               if (ok) {
                  SetState(ST_READ, "fast ping after read error...");
                  sleep = 0;
               } else {
                  double now = TMFE::GetTime();
                  if (now - fast_ping_start_time > fast_ping_timeout) {
                     SetState(ST_SLOW_PING, "ping timeout");
                     sleep = 0;
                     fMfe->Msg(MERROR, "ThreadPwb", "%s: communication timeout", fOdbName.c_str());
                  } else {
                     sleep = sleep_fast_ping;
                  }
               }
               break;
            }
            }
         } // implicit unlock of fLock

         if (sleep > 0) {
            for (int i=0; i<sleep; i++) {
               if (fMfe->fShutdownRequested)
                  break;
               ::sleep(1);
            }
         }
      }

      printf("thread for %s shutdown\n", fOdbName.c_str());
   }

#if 0
   void ThreadPwb()
   {
      printf("thread for %s started\n", fOdbName.c_str());
      assert(fEsper);
      while (!fMfe->fShutdownRequested) {
         if (fEsper->fFailed) {
            bool ok;
            {
               std::lock_guard<std::mutex> lock(fLock);
               ok = IdentifyPwbLocked();
               // fLock implicit unlock
            }
            if (!ok) {
               fState = ST_BAD_IDENTIFY;
               for (int i=0; i<fConfFailedSleep; i++) {
                  if (fMfe->fShutdownRequested)
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

         for (int i=0; i<fConfPollSleep; i++) {
            if (fMfe->fShutdownRequested)
               break;
            sleep(1);
         }
      }
      printf("thread for %s shutdown\n", fOdbName.c_str());
   }
#endif

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
         return;
      }

      EsperNodeData e;

      bool ok = ReadPwbLocked(&e);
      if (!ok) {
         SetState(ST_BAD_READ, "bad read"); // must match state machine in ThreadPwb()
         return;
      }

      ok = CheckPwbLocked(e, false);
      if (!ok) {
         SetState(ST_BAD_CHECK, "bad check"); // must match state machine in ThreadPwb()
         return;
      }

      SetState(ST_GOOD, "ok"); // must match state machine in ThreadPwb()
   }

   void InitPwbLocked()
   {
      if (!fEsper) {
         return;
      }

      SetState(fState, "ping...");

      bool ok = PingPwbLocked();
      if (!ok) {
         SetState(ST_INITIAL, "no ping"); // must be consistent with state machine in ThreadPwb()
         return;
      }

      SetState(fState, "identify...");

      ok = IdentifyPwbLocked();
      if (!ok) {
         SetState(ST_INITIAL, "no identify"); // must be consistent with state machine in ThreadPwb()
         return;
      }

      SetState(fState, "check reboot to user page...");

      bool need_reboot = CheckRebootToUserPagePwbLocked();
      if (need_reboot) {
         SetState(ST_REBOOT, "reboot to user epcq page..."); // must be consistent with state machine in ThreadPwb()
         return;
      }

      SetState(fState, "configure...");

      ok = ConfigurePwbLocked();
      if (!ok) {
         SetState(ST_INITIAL, "bad configure"); // must be consistent with state machine in ThreadPwb()
         return;
      }

      SetState(fState, "read and check...");

      ReadAndCheckPwbLocked();
   }

   void BeginRunPwbLocked(bool start, bool enablePwbTrigger)
   {
      double t0 = TMFE::GetTime();
      if (!fEsper)
         return;
      fEnablePwbTrigger = enablePwbTrigger;
      InitPwbLocked();
      if (fState != ST_GOOD) {
         fMfe->Msg(MERROR, "BeginRunPwbLocked", "%s: not started because in bad state %d", fOdbName.c_str(), fState);
         return;
      }
      fExtTrigCount0 = fExtTrigCount;
      fTriggerTotalRequested0 = fTriggerTotalRequested;
      fTriggerTotalAccepted0 = fTriggerTotalAccepted;
      fTriggerTotalDropped0 = fTriggerTotalDropped;
      fOffloadTxCnt0 = fOffloadTxCnt;
      double tc = TMFE::GetTime();
      //WriteVariables();
      if (start && enablePwbTrigger) {
         StartPwbLocked();
      }
      double t1 = TMFE::GetTime();
      fMfe->Msg(MLOG, "BeginRunPwbLocked", "%s: thread start time %.3f sec, begin run time %.3f sec: init %.3f, start %.3f", fOdbName.c_str(), t0-gBeginRunStartThreadsTime, t1-t0, tc-t0, t1-tc);
   }
};

#include "GrifComm.h"

#include "atpacket.h"

typedef std::vector<char> TrgData;

std::vector<TrgData*> gTrgDataBuf;
std::mutex gTrgDataBufLock;

class TrgCtrl
{
public:
   TMFE* fMfe = NULL;
   TMFeEquipment* fEq = NULL;
   std::string fOdbName;
   GrifComm* fComm = NULL;
   MVOdb* fStatus = NULL;

public:

   int fModule = 1;

   bool fVerbose = false;

   bool fOk = true;

   int fConfPollSleep = 10;
   int fConfFailedSleep = 10;

   int fNumBanks = 1;

   std::mutex fLock;

   int fUpdateCount = 0;

   int fDebug = 0;

   Fault fCheckComm;

public:
   TrgCtrl(TMFE* mfe, TMFeEquipment* eq, const char* hostname, const char* odbname)
   {
      fMfe = mfe;
      fEq = eq;
      fOdbName = odbname;

      fComm = new GrifComm();
      fComm->mfe = mfe;
      fComm->fHostname = hostname;
      fComm->OpenSockets();
      
      fCheckComm.Setup(fMfe, fEq, fOdbName.c_str(), "communication");

      fStatus = fEq->fOdbEq->Chdir("Status", true);
   }

   void Lock()
   {
      fLock.lock();
   }

   std::string fLastCommError;

   bool IdentifyTrgLocked()
   {
      //fComm->fFailed = false;

      bool ok = true;

      uint32_t timestamp = 0;
      uint32_t sysreset_ts = 0;

      std::string errstr;

      ok &= fComm->try_read_param(0x1F, 0xFFFF, &timestamp, &errstr);

      if (!ok) {
         if (errstr != fLastCommError) {
            fMfe->Msg(MERROR, "Identify", "%s: communication failure: %s", fOdbName.c_str(), errstr.c_str());
            fLastCommError = errstr;
         }
         fCheckComm.Fail("try_read_param error");
         return false;
      }

      ok &= fComm->try_read_param(0x3C, 0xFFFF, &sysreset_ts, &errstr);

      fComm->fFailed = false;

      time_t ts = (time_t)timestamp;
      const struct tm* tptr = localtime(&ts);
      char tstampbuf[256];
      strftime(tstampbuf, sizeof(tstampbuf), "%d%b%g_%H:%M", tptr);

      fMfe->Msg(MLOG, "Identify", "%s: firmware timestamp 0x%08x (%s), sysreset_ts 0x%08x", fOdbName.c_str(), timestamp, tstampbuf, sysreset_ts);

      fCheckComm.Ok();

      return true;
   }

   //bool fConfCosmicEnable = false;
   bool fConfSwPulserEnable = false;
   double fConfSwPulserFreq = 1.0;
   int    fConfSyncCount = 5;
   double fConfSyncPeriodSec = 1.5;
   double fConfSyncPeriodIncrSec = 0.1;

   // pulser configuration
   double fConfPulserClockFreq = 62.5*1e6; // 62.5 MHz
   int fConfPulserWidthClk = 5;
   int fConfPulserPeriodClk = 0;
   double fConfPulserFreq = 10.0; // 10 Hz
   bool fConfRunPulser = true;
   bool fConfOutputPulser = true;

   bool fConfTrigPulser = false;
   bool fConfTrigEsataNimGrandOr = false;

   bool fConfTrigAdc16GrandOr = false;
   bool fConfTrigAdc32GrandOr = false;

   bool fConfTrigAdcGrandOr = false;

   bool fConfTrig1ormore = false;
   bool fConfTrig2ormore = false;
   bool fConfTrig3ormore = false;
   bool fConfTrig4ormore = false;

   bool fConfTrigAw1ormore = false;
   bool fConfTrigAw2ormore = false;
   bool fConfTrigAw3ormore = false;
   bool fConfTrigAw4ormore = false;

   bool fConfTrigAwCoincA = false;
   bool fConfTrigAwCoincB = false;
   bool fConfTrigAwCoincC = false;
   bool fConfTrigAwCoincD = false;

   bool fConfTrigAwCoinc = false;

   bool fConfTrigBscGrandOr = false;
   bool fConfTrigBscMult    = false;

   bool fConfTrigCoinc      = false;

   uint32_t fConfAwCoincA = 0;
   uint32_t fConfAwCoincB = 0;
   uint32_t fConfAwCoincC = 0;
   uint32_t fConfAwCoincD = 0;

   bool fConfTrigAwMLU = false;

   uint32_t fConfNimMask = 0;
   uint32_t fConfEsataMask = 0;

   int fSasLinkModId[16];
   std::string fSasLinkModName[16];

   bool fConfPassThrough = false;

   int fConfPeriodScalers = 10;

   int fConfClockSelect = 0;
   int fConfScaledown = 0;

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

   bool WriteLatch()
   {
      bool ok = fComm->write_param(0x2B, 0xFFFF, (1<<1)); // write conf_latch
      return ok;
   }

   bool WriteTrigEnable(uint32_t trig_enable)
   {
      //if (fConfClockSelect) {
      //   trig_enable |= (1<<20);
      //}
      bool ok = fComm->write_param(0x25, 0xFFFF, trig_enable);
      fMfe->Msg(MINFO, "WriteTrigEnable", "%s: write conf_trig_enable 0x%08x ok %d", fOdbName.c_str(), trig_enable, ok);
      return ok;
   }

   bool LoadMluLocked()
   {
      bool ok = true;

      if (!fComm || fComm->fFailed) {
         printf("Configure %s: no communication\n", fOdbName.c_str());
         return false;
      }

      int mlu_selected_file = 0;
      fEq->fOdbEqSettings->RI("TRG/MluSelectedFile", &mlu_selected_file, true);

      if (mlu_selected_file < 0) {
         fMfe->Msg(MINFO, "Configure", "%s: MLU selected %d disables loading of MLU", fOdbName.c_str(), mlu_selected_file);
         return ok;
      }

      std::string mlu_dir = "/home/agdaq/online/src";

      fEq->fOdbEqSettings->RS("TRG/MluDir", &mlu_dir, true);

      std::string mlu_file = "mlu.txt";

      fEq->fOdbEqSettings->RSAI("TRG/MluFiles", mlu_selected_file, &mlu_file);

      mlu_file = mlu_dir + "/" + mlu_file;

      fMfe->Msg(MINFO, "Configure", "%s: MLU selected %d file \"%s\", TrigMLU: %d", fOdbName.c_str(), mlu_selected_file, mlu_file.c_str(), fConfTrigAwMLU);

      int addr = 0x37;

      if (0) {
         ok &= fComm->write_param(addr, 0xFFFF, 0x80000000); // reset the MLU
         //printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);
         ok &= fComm->write_param(addr, 0xFFFF, 0);
         //printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);
      }
      
      // write zero to address zero to make sure we do not trigger on empty events

      ok &= fComm->write_param(addr, 0xFFFF, 0x40000000);
      //printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);
      ok &= fComm->write_param(addr, 0xFFFF, 0);

      FILE *fp = fopen(mlu_file.c_str(), "r");

      if (!fp) {
         fMfe->Msg(MERROR, "Configure", "%s: Cannot open MLU file \"%s\", errno %d (%s)", fOdbName.c_str(), mlu_file.c_str(), errno, strerror(errno));
         return false;
      }

      int mlu[0x10000];
      memset(mlu, 0, sizeof(mlu));

      while (1) {
         char buf[256];
         char*s = fgets(buf, sizeof(buf), fp);
         if (!s)
            break;

         int vaddr = strtoul(s, &s, 0);
         int value = strtoul(s, &s, 0);

         //printf("addr 0x%08x, value %d, read: %s", addr, value, buf);

         mlu[vaddr&0xFFFF] = value;
      }

      fclose(fp);

      for (int i=0; i<0x10000; i++) {
         uint32_t v = 0x40000000;
         v |= ((mlu[i]&1)<<16);
         v |= (i&0xFFFF);
         ok &= fComm->write_param(addr, 0xFFFF, v);
         //printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);
      }

      ok &= fComm->write_param(addr, 0xFFFF, 0);
      //printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);

      fMfe->Msg(MINFO, "Configure", "%s: MLU file \"%s\" load status %d", fOdbName.c_str(), mlu_file.c_str(), ok);

      return ok;
   }

   bool ConfigureTrgLocked(bool enableAdcTrigger, bool enablePwbTrigger, bool enableTdcTrigger)
   {
      if (fComm->fFailed) {
         printf("Configure %s: no communication\n", fOdbName.c_str());
         return false;
      }

      fEq->fOdbEqSettings->RI("TRG/ClockSelect", &fConfClockSelect, true);


      fEq->fOdbEqSettings->RI("PeriodScalers", &fConfPeriodScalers, true);

      //fEq->fOdbEqSettings->RB("CosmicEnable", &fConfCosmicEnable, true);
      fEq->fOdbEqSettings->RB("SwPulserEnable", &fConfSwPulserEnable, true);
      fEq->fOdbEqSettings->RD("SwPulserFreq",   &fConfSwPulserFreq, true);

      // settings for the synchronization sequence

      fEq->fOdbEqSettings->RI("SyncCount",      &fConfSyncCount, true);
      if (enablePwbTrigger)
         fEq->fOdbEqSettings->RD("PwbSyncPeriodSec",  &fConfSyncPeriodSec, true);
      else
         fEq->fOdbEqSettings->RD("SyncPeriodSec",  &fConfSyncPeriodSec, true);

      fEq->fOdbEqSettings->RD("SyncPeriodIncrSec", &fConfSyncPeriodIncrSec, true);
      
      // settings for the TRG pulser

      fEq->fOdbEqSettings->RD("Pulser/ClockFreqHz",     &fConfPulserClockFreq, true);
      fEq->fOdbEqSettings->RI("Pulser/PulseWidthClk",   &fConfPulserWidthClk, true);
      fEq->fOdbEqSettings->RI("Pulser/PulsePeriodClk",  &fConfPulserPeriodClk, true);
      fEq->fOdbEqSettings->RD("Pulser/PulseFreqHz",     &fConfPulserFreq, true);
      fEq->fOdbEqSettings->RB("Pulser/Enable",          &fConfRunPulser, true);
      fEq->fOdbEqSettings->RB("Pulser/OutputEnable",    &fConfOutputPulser, true);

      fEq->fOdbEqSettings->RB("TrigSrc/TrigPulser",     &fConfTrigPulser, true);
      fEq->fOdbEqSettings->RB("TrigSrc/TrigEsataNimGrandOr", &fConfTrigEsataNimGrandOr, true);

      fEq->fOdbEqSettings->RB("TrigSrc/TrigAdc16GrandOr",  &fConfTrigAdc16GrandOr, true);
      fEq->fOdbEqSettings->RB("TrigSrc/TrigAdc32GrandOr",  &fConfTrigAdc32GrandOr, true);

      fEq->fOdbEqSettings->RB("TrigSrc/TrigAdcGrandOr",    &fConfTrigAdcGrandOr, true);

      fEq->fOdbEqSettings->RB("TrigSrc/Trig1ormore",  &fConfTrig1ormore, true);
      fEq->fOdbEqSettings->RB("TrigSrc/Trig2ormore",  &fConfTrig2ormore, true);
      fEq->fOdbEqSettings->RB("TrigSrc/Trig3ormore",  &fConfTrig3ormore, true);
      fEq->fOdbEqSettings->RB("TrigSrc/Trig4ormore",  &fConfTrig4ormore, true);

      //fEq->fOdbEqSettings->RB("TrigSrc/TrigAdc16Coinc",  &fConfTrigAdc16Coinc, true);

      fEq->fOdbEqSettings->RB("TrigSrc/TrigAwCoincA",  &fConfTrigAwCoincA, true);
      fEq->fOdbEqSettings->RB("TrigSrc/TrigAwCoincB",  &fConfTrigAwCoincB, true);
      fEq->fOdbEqSettings->RB("TrigSrc/TrigAwCoincC",  &fConfTrigAwCoincC, true);
      fEq->fOdbEqSettings->RB("TrigSrc/TrigAwCoincD",  &fConfTrigAwCoincD, true);

      fEq->fOdbEqSettings->RB("TrigSrc/TrigAwCoinc",  &fConfTrigAwCoinc, true);

      fEq->fOdbEqSettings->RU32("TRG/AwCoincA", &fConfAwCoincA, true);
      fEq->fOdbEqSettings->RU32("TRG/AwCoincB", &fConfAwCoincB, true);
      fEq->fOdbEqSettings->RU32("TRG/AwCoincC", &fConfAwCoincC, true);
      fEq->fOdbEqSettings->RU32("TRG/AwCoincD", &fConfAwCoincD, true);

      fEq->fOdbEqSettings->RB("TrigSrc/TrigAw1ormore", &fConfTrigAw1ormore, true);
      fEq->fOdbEqSettings->RB("TrigSrc/TrigAw2ormore", &fConfTrigAw2ormore, true);
      fEq->fOdbEqSettings->RB("TrigSrc/TrigAw3ormore", &fConfTrigAw3ormore, true);
      fEq->fOdbEqSettings->RB("TrigSrc/TrigAw4ormore", &fConfTrigAw4ormore, true);

      fEq->fOdbEqSettings->RB("TrigSrc/TrigAwMLU",  &fConfTrigAwMLU, true);

      fEq->fOdbEqSettings->RB("TrigSrc/TrigBscGrandOr",  &fConfTrigBscGrandOr, true);
      fEq->fOdbEqSettings->RB("TrigSrc/TrigBscMult",  &fConfTrigBscMult, true);

      fEq->fOdbEqSettings->RB("TrigSrc/TrigCoinc",  &fConfTrigCoinc, true);

      fEq->fOdbEqSettings->RU32("TRG/NimMask",    &fConfNimMask,   true);
      fEq->fOdbEqSettings->RU32("TRG/EsataMask",  &fConfEsataMask, true);

      fEq->fOdbEqSettings->RB("TRG/PassThrough",  &fConfPassThrough, true);

      bool aw16_from_adc16 = false;
      bool aw16_from_adc32a = false;
      bool aw16_from_adc32b = false;

      fEq->fOdbEqSettings->RB("TRG/Aw16FromAdc16",   &aw16_from_adc16,  true);
      fEq->fOdbEqSettings->RB("TRG/Aw16FromAdc32a",  &aw16_from_adc32a, true);
      fEq->fOdbEqSettings->RB("TRG/Aw16FromAdc32b",  &aw16_from_adc32b, true);

      bool bsc_from_adc16_a = false;
      bool bsc_from_adc16_b = false;

      fEq->fOdbEqSettings->RB("TRG/BscFromAdc16a",  &bsc_from_adc16_a, true);
      fEq->fOdbEqSettings->RB("TRG/BscFromAdc16b",  &bsc_from_adc16_b, true);

      bool bsc_bot_only = false;
      bool bsc_top_only = false;
      bool bsc_bot_top_or = true;
      bool bsc_bot_top_and = false;

      fEq->fOdbEqSettings->RB("TRG/BscBotOnly",   &bsc_bot_only, true);
      fEq->fOdbEqSettings->RB("TRG/BscTopOnly",   &bsc_top_only, true);
      fEq->fOdbEqSettings->RB("TRG/BscBotTopOr",  &bsc_bot_top_or, true);
      fEq->fOdbEqSettings->RB("TRG/BscBotTopAnd", &bsc_bot_top_and, true);

      int bsc_multiplicity = 1;

      fEq->fOdbEqSettings->RI("TRG/BscMultiplicity",  &bsc_multiplicity, true);

      uint32_t coinc_start = 0;
      uint32_t coinc_require = 0;
      int coinc_window = 16;

      fEq->fOdbEqSettings->RU32("TRG/CoincStart",   &coinc_start,   true);
      fEq->fOdbEqSettings->RU32("TRG/CoincRequire", &coinc_require, true);
      fEq->fOdbEqSettings->RI("TRG/CoincWindow",    &coinc_window,  true);

      bool ok = true;

      ok &= StopTrgLocked(false);

      ok &= WriteTrigEnable(0); // disable all triggers

      int trg_udp_packet_size = 44;
      fEq->fOdbEqSettings->RI("TRG/UdpPacketSize", &trg_udp_packet_size, true);

      ok &= fComm->write_param(0x08, 0xFFFF, trg_udp_packet_size-2*4); // AT packet size in bytes minus the last 0xExxxxxxx word

      fMfe->Msg(MINFO, "Configure", "%s: enableAdcTrigger: %d", fOdbName.c_str(), enableAdcTrigger);
      fMfe->Msg(MINFO, "Configure", "%s: enablePwbTrigger: %d", fOdbName.c_str(), enablePwbTrigger);

      // control register

      uint32_t conf_control = 0;

      if (fConfClockSelect)
         conf_control |= (1<<0);

      if (aw16_from_adc16)
         conf_control |= (1<<1);
      
      if (aw16_from_adc32a)
         conf_control |= (1<<2);
      
      if (aw16_from_adc32b)
         conf_control |= (1<<3);
      
      int conf_mlu_prompt = 64;
      fEq->fOdbEqSettings->RI("TRG/MluPrompt", &conf_mlu_prompt, true);

      int conf_mlu_wait = 128;
      fEq->fOdbEqSettings->RI("TRG/MluWait", &conf_mlu_wait, true);

      conf_control |= (conf_mlu_prompt&0xFF)<<16;
      conf_control |= (conf_mlu_wait&0xFF)<<24;

      uint32_t read_control = 0;
      ok &= fComm->read_param(0x34, 0xFFFF, &read_control);

      if (conf_control == read_control) {
         fMfe->Msg(MINFO, "Configure", "%s: conf_control: 0x%08x unchanged", fOdbName.c_str(), conf_control);
      } else {

         ok &= fComm->write_param(0x34, 0xFFFF, conf_control);

         fMfe->Msg(MINFO, "Configure", "%s: conf_control: 0x%08x -> 0x%08x updated", fOdbName.c_str(), read_control, conf_control);
      }

      int conf_counter_adc_select = 0;
      fEq->fOdbEqSettings->RI("TRG/ConfCounterAdcSelect",  &conf_counter_adc_select, true);
      ok &= SelectAdcLocked(conf_counter_adc_select);

      // configure Barrel Scintillator trigger

      if (1) {
         uint32_t conf_bsc_control = 0;

         if (bsc_from_adc16_a)
            conf_bsc_control |= (1<<0);

         if (bsc_from_adc16_b)
            conf_bsc_control |= (1<<1);

         if (bsc_bot_only)
            conf_bsc_control |= (1<<2);

         if (bsc_top_only)
            conf_bsc_control |= (1<<3);

         if (bsc_bot_top_or)
            conf_bsc_control |= (1<<4);

         if (bsc_bot_top_and)
            conf_bsc_control |= (1<<5);

         conf_bsc_control |= ((bsc_multiplicity&0xFF)<<8);

         fMfe->Msg(MINFO, "Configure", "%s: conf_bsc_control: 0x%08x", fOdbName.c_str(), conf_bsc_control);
         ok &= fComm->write_param(0x40, 0xFFFF, conf_bsc_control);
      }

      // configure coincidence trigger

      if (1) {
         uint32_t conf_coinc_control = 0;

         conf_coinc_control |= ((coinc_require&0xFF)<<0);
         conf_coinc_control |= ((coinc_window&0xFF)<<8);
         conf_coinc_control |= ((coinc_start&0xFF)<<16);

         fMfe->Msg(MINFO, "Configure", "%s: conf_coinc_control: 0x%08x", fOdbName.c_str(), conf_coinc_control);
         ok &= fComm->write_param(0x41, 0xFFFF, conf_coinc_control);
      }

      // configure the 62.5 MHz clock section

      int drift_width = 10;
      fEq->fOdbEqSettings->RI("TRG/DriftWidthClk", &drift_width, true);
      ok &= fComm->write_param(0x35, 0xFFFF, drift_width);

      fEq->fOdbEqSettings->RI("TRG/Scaledown", &fConfScaledown, true);
      ok &= fComm->write_param(0x36, 0xFFFF, 0); // disable scaledown while we run the sync sequence

      int trig_delay = 0;
      fEq->fOdbEqSettings->RI("TRG/TrigDelayClk", &trig_delay, true);

      int mlu_trig_delay = 0;
      fEq->fOdbEqSettings->RI("TRG/MluTrigDelayClk", &mlu_trig_delay, true);

      if (fConfTrigAwMLU)
         trig_delay = mlu_trig_delay;

      ok &= fComm->write_param(0x38, 0xFFFF, trig_delay);

      int trig_width = 10;
      fEq->fOdbEqSettings->RI("TRG/TrigWidthClk", &trig_width, true);
      ok &= fComm->write_param(0x20, 0xFFFF, trig_width);

      int busy_width = 6250;
      fEq->fOdbEqSettings->RI("TRG/BusyWidthClk", &busy_width, true);

      int tdc_busy_width = 6250;
      fEq->fOdbEqSettings->RI("TRG/TdcBusyWidthClk", &tdc_busy_width, true);

      int adc_busy_width = 6250;
      fEq->fOdbEqSettings->RI("TRG/AdcBusyWidthClk", &adc_busy_width, true);

      int pwb_busy_width = 208000;
      fEq->fOdbEqSettings->RI("TRG/PwbBusyWidthClk", &pwb_busy_width, true);

      if (enableTdcTrigger) {
         if (tdc_busy_width > busy_width)
            busy_width = tdc_busy_width;
      }

      if (enableAdcTrigger) {
         if (adc_busy_width > busy_width)
            busy_width = adc_busy_width;
      }

      if (enablePwbTrigger) {
         if (pwb_busy_width > busy_width)
            busy_width = pwb_busy_width;
      }

      ok &= fComm->write_param(0x21, 0xFFFF, busy_width);

      fMfe->Msg(MINFO, "Configure", "%s: 62.5MHz section: drift blank-off %d, scaledown %d, trigger delay %d, width %d, busy %d (adc %d, pwb %d, tdc %d)", fOdbName.c_str(), drift_width, fConfScaledown, trig_delay, trig_width, busy_width, adc_busy_width, pwb_busy_width, tdc_busy_width);

      // configure the pulser

      ok &= fComm->write_param(0x22, 0xFFFF, fConfPulserWidthClk);

      if (fConfPulserFreq) {
         int clk = (1.0/fConfPulserFreq)*fConfPulserClockFreq;
         fComm->write_param(0x23, 0xFFFF, clk);
         fMfe->Msg(MINFO, "Configure", "%s: pulser freq %f Hz, period %d clocks", fOdbName.c_str(), fConfPulserFreq, clk);
      } else {
         fComm->write_param(0x23, 0xFFFF, fConfPulserPeriodClk);
         fMfe->Msg(MINFO, "Configure", "%s: pulser period %d clocks, frequency %f Hz", fOdbName.c_str(), fConfPulserPeriodClk, fConfPulserFreq/fConfPulserPeriodClk);
      }

      // NIM and ESATA masks

      fComm->write_param(0x29, 0xFFFF, fConfNimMask);
      fComm->write_param(0x2A, 0xFFFF, fConfEsataMask);

      // ADC16 masks

      std::vector<int> adc16_mask;
      fEq->fOdbEqSettings->RIA("TRG/adc16_masks", &adc16_mask, true, 16);

      for (int i=0; i<8; i++) {
         int m0 = adc16_mask[2*i+0];
         int m1 = adc16_mask[2*i+1];
         uint32_t m = ((0xFFFF&((uint32_t)m1))<<16)|(0xFFFF&(uint32_t)m0);
         fComm->write_param(0x200+i, 0xFFFF, m);
      }

      // ADC32 masks

      std::vector<int> adc32_mask;
      fEq->fOdbEqSettings->RIA("TRG/adc32_masks", &adc32_mask, true, 16);

      for (int i=0; i<16; i++) {
         fComm->write_param(0x300+i, 0xFFFF, adc32_mask[i]);
      }

      ReadSasBitsLocked();

      fComm->write_param(0x2C, 0xFFFF, fConfAwCoincA);
      fComm->write_param(0x2D, 0xFFFF, fConfAwCoincB);
      fComm->write_param(0x2E, 0xFFFF, fConfAwCoincC);
      fComm->write_param(0x2F, 0xFFFF, fConfAwCoincD);

      fMfe->Msg(MINFO, "Configure", "%s: AW ConfCoincA,B,C,D: 0x%08x, 0x%08x, 0x%08x, 0x%08x, Enabled: %d, %d, %d, %d, TrigAwCoinc: %d", fOdbName.c_str(), fConfAwCoincA, fConfAwCoincB, fConfAwCoincC, fConfAwCoincD, fConfTrigAwCoincA, fConfTrigAwCoincB, fConfTrigAwCoincC, fConfTrigAwCoincD, fConfTrigAwCoinc);

      if (fConfTrigAwCoinc) {
         if (fConfTrigAwCoincA) {
            fMfe->Msg(MINFO, "Configure", "%s: ConfAwCoincA: 0x%08x: (%s) * (%s)", fOdbName.c_str(), fConfAwCoincA, LinkMaskToString((fConfAwCoincA>>16)&0xFFFF).c_str(),LinkMaskToString(fConfAwCoincA&0xFFFF).c_str());
         }
         if (fConfTrigAwCoincB) {
            fMfe->Msg(MINFO, "Configure", "%s: ConfAwCoincB: 0x%08x: (%s) * (%s)", fOdbName.c_str(), fConfAwCoincB, LinkMaskToString((fConfAwCoincB>>16)&0xFFFF).c_str(),LinkMaskToString(fConfAwCoincB&0xFFFF).c_str());
         }
         if (fConfTrigAwCoincC) {
            fMfe->Msg(MINFO, "Configure", "%s: ConfAwCoincC: 0x%08x: (%s) * (%s)", fOdbName.c_str(), fConfAwCoincC, LinkMaskToString((fConfAwCoincC>>16)&0xFFFF).c_str(),LinkMaskToString(fConfAwCoincC&0xFFFF).c_str());
         }
         if (fConfTrigAwCoincD) {
            fMfe->Msg(MINFO, "Configure", "%s: ConfAwCoincD: 0x%08x: (%s) * (%s)", fOdbName.c_str(), fConfAwCoincD, LinkMaskToString((fConfAwCoincD>>16)&0xFFFF).c_str(),LinkMaskToString(fConfAwCoincD&0xFFFF).c_str());
         }
         
         if (fConfTrigAwCoincA) {
            fMfe->Msg(MINFO, "Configure", "%s: ConfAwCoincA: 0x%08x: (%s) * (%s)", fOdbName.c_str(), fConfAwCoincA, LinkMaskToAdcString((fConfAwCoincA>>16)&0xFFFF).c_str(),LinkMaskToAdcString(fConfAwCoincA&0xFFFF).c_str());
         }
         if (fConfTrigAwCoincB) {
            fMfe->Msg(MINFO, "Configure", "%s: ConfAwCoincB: 0x%08x: (%s) * (%s)", fOdbName.c_str(), fConfAwCoincB, LinkMaskToAdcString((fConfAwCoincB>>16)&0xFFFF).c_str(),LinkMaskToAdcString(fConfAwCoincB&0xFFFF).c_str());
         }
         if (fConfTrigAwCoincC) {
            fMfe->Msg(MINFO, "Configure", "%s: ConfAwCoincC: 0x%08x: (%s) * (%s)", fOdbName.c_str(), fConfAwCoincC, LinkMaskToAdcString((fConfAwCoincC>>16)&0xFFFF).c_str(),LinkMaskToAdcString(fConfAwCoincC&0xFFFF).c_str());
         }
         if (fConfTrigAwCoincD) {
            fMfe->Msg(MINFO, "Configure", "%s: ConfAwCoincD: 0x%08x: (%s) * (%s)", fOdbName.c_str(), fConfAwCoincD, LinkMaskToAdcString((fConfAwCoincD>>16)&0xFFFF).c_str(),LinkMaskToAdcString(fConfAwCoincD&0xFFFF).c_str());
         }
      }

      ok &= LoadMluLocked();

      return ok;
   }

   bool UpdatePulserFreqTrgLocked()
   {
      if (fComm->fFailed) {
         printf("Configure %s: no communication\n", fOdbName.c_str());
         return false;
      }

      fEq->fOdbEqSettings->RD("Pulser/ClockFreqHz",     &fConfPulserClockFreq, true);
      fEq->fOdbEqSettings->RI("Pulser/PulseWidthClk",   &fConfPulserWidthClk, true);
      fEq->fOdbEqSettings->RI("Pulser/PulsePeriodClk",  &fConfPulserPeriodClk, true);
      fEq->fOdbEqSettings->RD("Pulser/PulseFreqHz",     &fConfPulserFreq, true);
      fEq->fOdbEqSettings->RB("Pulser/Enable",          &fConfRunPulser, true);
      fEq->fOdbEqSettings->RB("Pulser/OutputEnable",    &fConfOutputPulser, true);

      if (fConfPulserFreq) {
         int clk = (1.0/fConfPulserFreq)*fConfPulserClockFreq;
         fComm->write_param(0x23, 0xFFFF, clk);
         fMfe->Msg(MINFO, "Configure", "%s: pulser freq %f Hz, period %d clocks", fOdbName.c_str(), fConfPulserFreq, clk);
      } else {
         fComm->write_param(0x23, 0xFFFF, fConfPulserPeriodClk);
         fMfe->Msg(MINFO, "Configure", "%s: pulser period %d clocks, frequency %f Hz", fOdbName.c_str(), fConfPulserPeriodClk, fConfPulserFreq/fConfPulserPeriodClk);
      }

      return true;
   }

   bool fRunning = false;
   int  fSyncPulses = 0;
   double fSyncPeriodSec = 0;

   bool StartTrgLocked()
   {
      bool ok = true;

      fSyncPulses = fConfSyncCount;
      fSyncPeriodSec = fConfSyncPeriodSec;

      fRunning = false;

      ok &= fComm->write_param(0x36, 0xFFFF, 0); // disable scaledown while we run the sync sequence
      ok &= fComm->write_param(0x43, 0xFFFF, 1250000000); // enable trigger timeout 10 sec

      uint32_t trig_enable = 0;

      if (fSyncPulses) {
         trig_enable |= (1<<0); // enable software trigger
      }

      if (!fConfPassThrough) {
         trig_enable |= (1<<13); // enable udp packets
         trig_enable |= (1<<14); // enable busy counter
      }

      ok &= WriteTrigEnable(trig_enable);

      fComm->write_drq(); // request udp packet data

      return ok;
   }

   bool XStartTrg()
   {
      bool ok = true;
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
      
      //if (fConfTrigAdc16Coinc)
      //trig_enable |= (1<<12);
      
      if (!fConfPassThrough) {
         trig_enable |= (1<<13);
         trig_enable |= (1<<14);
      }
      
      if (fConfTrigAdcGrandOr)
         trig_enable |= (1<<15);

      if (fConfTrigAwCoincA)
         trig_enable |= (1<<16);
      if (fConfTrigAwCoincB)
         trig_enable |= (1<<17);
      if (fConfTrigAwCoincC)
         trig_enable |= (1<<18);
      if (fConfTrigAwCoincD)
         trig_enable |= (1<<19);
      if (fConfTrigAwCoinc)
         trig_enable |= (1<<21);

      if (fConfTrigAwMLU)
         trig_enable |= (1<<22);
      
      if (fConfTrigAw1ormore)
         trig_enable |= (1<<24);
      if (fConfTrigAw2ormore)
         trig_enable |= (1<<25);
      if (fConfTrigAw3ormore)
         trig_enable |= (1<<26);
      if (fConfTrigAw4ormore)
         trig_enable |= (1<<27);
      
      if (fConfTrigBscGrandOr)
         trig_enable |= (1<<28);
      if (fConfTrigBscMult)
         trig_enable |= (1<<29);
      
      if (fConfTrigCoinc)
         trig_enable |= (1<<30);

      if ((trig_enable&(~((1<<13)|(1<<14)|(1<<4)))) != 0)
         trig_enable |= (1<<23); // trigger timeout trigger
      
      fMfe->Msg(MINFO, "AtCtrl::Tread", "%s: Writing trig_enable 0x%08x", fOdbName.c_str(), trig_enable);
      
      {
         std::lock_guard<std::mutex> lock(fLock);
         ok &= fComm->write_param(0x36, 0xFFFF, fConfScaledown); // enable scaledown
         if (trig_enable & (1<<23)) {
            ok &= fComm->write_param(0x43, 0xFFFF, 1250000000); // enable trigger timeout 10 sec
         } else {
            ok &= fComm->write_param(0x43, 0xFFFF, 0); // disable timeout trigger
         }
         ok &= WriteTrigEnable(trig_enable);
      }

      return ok;
   }

   bool XStopTrgLocked()
   {
      bool ok = true;
      ok &= WriteTrigEnable(0); // disable all triggers
      return ok;
   }

   bool StopTrgLocked(bool send_extra_trigger)
   {
      bool ok = true;
      ok &= XStopTrgLocked();

      if (send_extra_trigger) {
         printf("AlphaTctrl::StopTrgLocked: sending an extra trigger!\n");
         uint32_t trig_enable = 0;
         trig_enable |= (1<<0); // enable software trigger
         if (!fConfPassThrough) {
            trig_enable |= (1<<13); // enable udp packets
            trig_enable |= (1<<14); // enable busy counter
         }
         ok &= WriteTrigEnable(trig_enable);
         ok &= SoftTriggerTrgLocked();
      }

      fComm->write_stop(); // stop sending udp packet data
      fRunning = false;
      fSyncPulses = 0;
      return ok;
   }

   bool SelectAdcLocked(int conf_counter_adc_select)
   {
      bool ok = true;
      fMfe->Msg(MINFO, "SelectAdcLocked", "%s: conf_counter_adc_select 0x%08x", fOdbName.c_str(), conf_counter_adc_select);
      ok &= fComm->write_param(0x3B, 0xFFFF, conf_counter_adc_select);
      return ok;
   }

   bool SoftTriggerTrgLocked()
   {
      printf("AlphaTctrl::SoftTrigger!\n");
      bool ok = true;
      //ok &= fComm->write_param(0x24, 0xFFFF, 0);
      ok &= fComm->write_param(0x2B, 0xFFFF, (1<<2));
      return ok;
   }

   void ReadSasBitsLocked()
   {
      WriteLatch();

      for (int i=0; i<16; i++) {
         uint32_t v0;
         uint32_t v1;
         fComm->read_param(0x400+2*i+0, 0xFFFF, &v0);
         fComm->read_param(0x400+2*i+1, 0xFFFF, &v1);

         int modid = (v1&0xF0000000)>>28;

         if (v1 == 0)
            modid = -1;

         if (modid >= 0 && modid < 16) {
            fEq->fOdbEqSettings->RSAI("ADC/modules",  modid, &fSasLinkModName[i]);
         }

         fMfe->Msg(MINFO, "ReadSasBits", "%s: link %2d sas_bits: 0x%08x%08x, adc[%d], %s", fOdbName.c_str(), i, v1, v0, modid, fSasLinkModName[i].c_str());

         fSasLinkModId[i] = modid;

      }
   }

   void RebootTrgLocked()
   {
      uint32_t fw_rev = 0;
      fComm->read_param(0x1F, 0xFFFF, &fw_rev);
      fMfe->Msg(MINFO, "RebootTrgLocked", "%s: rebooting the TRG FPGA, old firmware timestamp 0x%08x", fOdbName.c_str(), fw_rev);
      fComm->write_param(0x42, 0xFFFF, ~fw_rev);
   }

   double fScPrevTime = 0;
   uint32_t fScPrevClk = 0;
   std::vector<int> fScPrev;
   std::vector<double> fScRatePrev;

   int fPrevPllLockCounter = 0;

   void ReadTrgLocked()
   {
      const int NSC = 16+16+32+16+16+64;
      uint32_t sas_sd = 0;
      std::vector<int> sas_sd_counters;
      std::vector<int> sas_bits;
      std::vector<int> sc;
      uint32_t pll_status = 0;
      std::string pll_status_string;
      std::string pll_status_colour;

      uint32_t fw_rev = 0;

      uint32_t conf_control = 0;

      uint32_t clk_counter = 0;
      uint32_t clk_625_counter = 0;
      double clk_625_freq = 0;

      uint32_t esata_clk_counter = 0;
      uint32_t esata_clk_esata_counter = 0;
      double clk_esata_freq = 0;

      uint32_t conf_counter_adc_select = 0;

      while (fScPrev.size() < NSC) {
         fScPrev.push_back(0);
      }

      while (fScRatePrev.size() < NSC) {
         fScRatePrev.push_back(0);
      }

      double t = 0;

      const int iclk = 0;

      {
         WriteLatch();

         t = TMFE::GetTime();

         bool ok = true;

         ok &= fComm->read_param(0x1F, 0xFFFF, &fw_rev);

         ok &= fComm->read_param(0x34, 0xFFFF, &conf_control);

         ok &= fComm->read_param(0x31, 0xFFFF, &pll_status);
         ok &= fComm->read_param(0x32, 0xFFFF, &clk_counter);
         ok &= fComm->read_param(0x33, 0xFFFF, &clk_625_counter);
         ok &= fComm->read_param(0x39, 0xFFFF, &esata_clk_counter);
         ok &= fComm->read_param(0x3A, 0xFFFF, &esata_clk_esata_counter);

         ok &= fComm->read_param(0x3B, 0xFFFF, &conf_counter_adc_select);

         if (!ok) {
            fMfe->Msg(MINFO, "ReadTrgLocked", "%s: errors reading the TRG", fOdbName.c_str());
            return;
         }

         double clk_freq = 125.0e6; // 125MHz

         double clk_time = clk_counter/clk_freq;
         if (clk_time > 0) {
            clk_625_freq = clk_625_counter/clk_time;
         }

         double esata_clk_time = esata_clk_counter/clk_freq;
         if (esata_clk_time > 0) {
            clk_esata_freq = esata_clk_esata_counter/esata_clk_time;
         }

         if (pll_status & (1<<31))
            pll_status_string += " Locked";
         if (pll_status & (1<<30))
            pll_status_string += " ExtClock";
         if (pll_status & (1<<29))
            pll_status_string += " EsataClkBad";
         if (pll_status & (1<<28))
            pll_status_string += " InternalClkBad";

         printf("clk_625: PLL status 0x%08x (%s), counters 0x%08x 0x%08x, time %f sec, freq %f\n", pll_status, pll_status_string.c_str(), clk_counter, clk_625_counter, clk_time, clk_625_freq);

         bool pll_ok = true;
         std::string pll_alarm_msg;

         if (!(pll_status & (1<<31))) {
            pll_ok = false;
            if (pll_alarm_msg.length() > 0)
               pll_alarm_msg += ", ";
            pll_alarm_msg += "TRG PLL is not locked";
         }

         if (pll_status & (1<<30) && (pll_status & (1<<29))) {
            pll_ok = false;
            if (pll_alarm_msg.length() > 0)
               pll_alarm_msg += ", ";
            pll_alarm_msg += "TRG External clock is bad";
         }

         if (pll_status & (1<<28)) {
            pll_ok = false;
            if (pll_alarm_msg.length() > 0)
               pll_alarm_msg += ", ";
            pll_alarm_msg += "TRG Internal clock is bad";
         }

         if (fabs(clk_625_freq) < 100) {
            pll_ok = false;
            if (pll_alarm_msg.length() > 0)
               pll_alarm_msg += ", ";
            pll_alarm_msg += "TRG External clock frequency is zero";
            pll_status_string += " (ExtClkZeroFreq)";
         }

         if (fabs(clk_625_freq - 62500000) > 20000) {
            pll_ok = false;
            if (pll_alarm_msg.length() > 0)
               pll_alarm_msg += ", ";
            pll_alarm_msg += "TRG External clock bad frequency";
            pll_status_string += " (ExtClkBadFreq)";
         }

         if (fConfClockSelect) {
            if (!(pll_status & (1<<30))) {
               pll_ok = false;
               if (pll_alarm_msg.length() > 0)
                  pll_alarm_msg += ", ";
               pll_alarm_msg += "TRG External clock not selected";
               pll_status_string += " (ExtClkNotSelected)";
            }
         }

         if (pll_ok) {
            pll_status_colour = "green";
            fMfe->ResetAlarm("Bad TRG PLL status");
         } else {
            pll_status_colour = "red";
            fMfe->TriggerAlarm("Bad TRG PLL status", pll_alarm_msg.c_str(), "Alarm");
         }

         int pll_lock_counter = pll_status & 0xFFFF;

         if (pll_lock_counter != fPrevPllLockCounter) {
            fMfe->Msg(MERROR, "ReadTrgLocked", "%s: External 62.5MHz clock PLL lock count changed from %d to %d", fOdbName.c_str(), fPrevPllLockCounter, pll_lock_counter);
            fPrevPllLockCounter = pll_lock_counter;
         }

         // read sas link status

         fComm->read_param(0x30, 0xFFFF, &sas_sd);

         for (int i=0; i<16; i++) {
            uint32_t v = 0;
            fComm->read_param(0x420+i, 0xFFFF, &v);
            sas_sd_counters.push_back(v);
         }

         for (int i=0; i<32; i++) {
            uint32_t v = 0;
            fComm->read_param(0x400+i, 0xFFFF, &v);
            sas_bits.push_back(v);
         }

         // 0..15 read the 16 base scalers

         for (int i=0; i<16; i++) {
            uint32_t v = 0;
            fComm->read_param(0x100+i, 0xFFFF, &v);
            sc.push_back(v);
         }

         // 16..31 read the adc16 scalers

         for (int i=0; i<16; i++) {
            uint32_t v = 0;
            fComm->read_param(0x430+i, 0xFFFF, &v);
            sc.push_back(v);
         }

         // 32..63 read the adc32 scalers

         for (int i=0; i<32; i++) {
            uint32_t v = 0;
            fComm->read_param(0x440+i, 0xFFFF, &v);
            sc.push_back(v);
         }

         // 64..79 read the 16 additional scalers

         for (int i=0; i<16; i++) {
            uint32_t v = 0;
            fComm->read_param(0x110+i, 0xFFFF, &v);
            sc.push_back(v);
         }

         // 80..95 read the 16 counters_adc_selected scalers

         for (int i=0; i<16; i++) {
            uint32_t v = 0;
            fComm->read_param(0x460+i, 0xFFFF, &v);
            sc.push_back(v);
         }

         // 96..159 read the 64 bar scalers

         for (int i=0; i<64; i++) {
            uint32_t v = 0;
            fComm->read_param(0x470+i, 0xFFFF, &v);
            sc.push_back(v);
         }

      }

      uint32_t clk = sc[iclk];

      uint32_t dclk = clk - fScPrevClk;
      double dclk_sec = dclk/(62.5*1e6);

      //printf("clk 0x%08x -> 0x%08x, dclk 0x%08x, time %f sec\n", fScPrevClk, clk, dclk, dclk_sec);

      fEq->fOdbEqVariables->WI("trg_fw_rev", fw_rev);
      fEq->fOdbEqVariables->WI("trg_conf_control", conf_control);
      fEq->fOdbEqVariables->WI("trg_pll_625_status", pll_status);
      fEq->fOdbEqVariables->WI("trg_clk_counter", clk_counter);
      fEq->fOdbEqVariables->WI("trg_clk_625_counter", clk_625_counter);
      fEq->fOdbEqVariables->WD("trg_clk_625_freq", clk_625_freq);
      fEq->fOdbEqVariables->WD("trg_clk_esata_freq", clk_esata_freq);
      fStatus->WS("trg_pll_status_string", pll_status_string.c_str());
      fStatus->WS("trg_pll_status_colour", pll_status_colour.c_str());

      fEq->fOdbEqVariables->WI("trg_conf_counter_adc_select", conf_counter_adc_select);
         
      fEq->fOdbEqVariables->WI("sas_sd", sas_sd);
      fEq->fOdbEqVariables->WIA("sas_sd_counters", sas_sd_counters);
      fEq->fOdbEqVariables->WIA("sas_bits", sas_bits);
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
            if (dt > 0 && diff >= 0 /* && diff < 1*1000*1000 */) {
               r = diff/dt;
            }

            if (diff != 0 && r == 0) {
               // scaler wrap-around
               //fMfe->Msg(MINFO, "xxx", "scaler %d: dt %f, value 0x%08x -> 0x%08x, diff %d, rate %f, prev %f", i, dt, fScPrev[i], sc[i], sc[i]-fScPrev[i], r, fScRatePrev[i]);
               r = fScRatePrev[i];
            }

            rate.push_back(r);
         }
         
         //int x = 0;
         //printf("scaler %d: dt %f, value 0x%08x -> 0x%08x, diff %d, rate %f, prev %f\n", x, dt, fScPrev[x], sc[x], sc[x]-fScPrev[x], rate[x], fScRatePrev[x]);

         dead_time = 0;
         if (rate[65] > 0) {
            dead_time = 1.0 - rate[1]/rate[65];
         }

         fEq->fOdbEqVariables->WD("trigger_dead_time", dead_time);
         fEq->fOdbEqVariables->WDA("scalers_rate", rate);

         for (int i=0; i<NSC; i++) {
            fScRatePrev[i] = rate[i];
         }
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
         fNextScalers += fConfPeriodScalers;
         std::lock_guard<std::mutex> lock(fLock);
         ReadTrgLocked();
      }
   }

   void ThreadTrg()
   {
      printf("thread for %s started\n", fOdbName.c_str());
      while (!fMfe->fShutdownRequested) {
         if (fComm->fFailed) {
            if (!fCheckComm.fFailed) {
               fCheckComm.Fail("see previous messages");
            }
            bool ok;
            {
               std::lock_guard<std::mutex> lock(fLock);
               ok = IdentifyTrgLocked();
               // fLock implicit unlock
            }
            if (ok) {
               fOk = true;
            } else {
               fOk = false;
               for (int i=0; i<fConfFailedSleep; i++) {
                  if (fMfe->fShutdownRequested)
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
               SoftTriggerTrgLocked();
            }

            if (fSyncPulses > 0) {
               fSyncPeriodSec += fConfSyncPeriodIncrSec;
               fSyncPulses--;
            }

            if (fSyncPulses == 0) {
               // after the last sync pulse, for max PWB busy time
               // before enabling the trigger.
               fSyncPeriodSec = 0.010;
            }

            double t0 = fMfe->GetTime();
            while (1) {
               double t1 = fMfe->GetTime();
               if (t1 - t0 > fSyncPeriodSec)
                  break;
               usleep(1000);
            };

            if (fSyncPulses == 0) {
               bool ok = true;
               ok &= XStartTrg();
               fRunning = true;
            }

         } else if (fRunning && fConfSwPulserEnable) {
            {
               std::lock_guard<std::mutex> lock(fLock);
               SoftTriggerTrgLocked();
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
            if (1) {
               sleep(1);
            } else {
               double t0 = fMfe->GetTime();
               while (1) {
                  double t1 = fMfe->GetTime();
                  if (t1 - t0 > 1.0) {
                     printf("S\n");
                     break;
                  }
                  uint32_t v;
                  double tt0 = fMfe->GetTime();
                  fComm->read_param(0x100, 0xFFFF, &v);
                  double tt1 = fMfe->GetTime();
                  double ttd = tt1-tt0;
                  int ttd_usec = (int)(ttd*1e6);
                  if (ttd_usec > 400) {
                     printf("R.%d.", ttd_usec);
                  }
               }
            }
         }
      }
      printf("thread for %s shutdown\n", fOdbName.c_str());
   }

   void ReadDataTrgThread()
   {
      const double clk625 = 62.5*1e6; // 62.5 MHz
      uint32_t tsprev = 0;
      double tprev = 0;
      int epoch = 0;

      uint32_t prev_packet_no = 0;
      uint32_t prev_trig_no = 0;
      uint32_t prev_ts_625 = 0;

      printf("data thread for %s started\n", fOdbName.c_str());
      while (!fMfe->fShutdownRequested) {
         std::string errstr;
         char replybuf[GrifSocket::kMaxPacketSize];
         int rd = fComm->fDataSocket.readmsg(replybuf, sizeof(replybuf), 10000, &errstr);
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
         
         TrgData *buf = new TrgData;
         buf->resize(rd);
         memcpy(buf->data(), replybuf, rd);
            
         {
            std::lock_guard<std::mutex> lock(gTrgDataBufLock);
            gTrgDataBuf.push_back(buf);
         }
      }
      printf("data thread for %s shutdown\n", fOdbName.c_str());
   }

   void ReadAndCheckTrgLocked()
   {
      //EsperNodeData e;
      //bool ok = ReadAll(&e);
      //if (ok) {
      //   ok = Check(e);
      //}
   }

   void BeginRunTrgLocked(bool start, bool enableAdcTrigger, bool enablePwbTrigger, bool enableTdcTrigger)
   {
      IdentifyTrgLocked();
      ConfigureTrgLocked(enableAdcTrigger, enablePwbTrigger, enableTdcTrigger);
      ReadAndCheckTrgLocked();
      //WriteVariables();
      //if (start) {
      //Start();
      //}
   }

   std::thread* fThread = NULL;
   std::thread* fDataThread = NULL;

   void StartThreads()
   {
      fThread = new std::thread(&TrgCtrl::ThreadTrg, this);
      fDataThread = new std::thread(&TrgCtrl::ReadDataTrgThread, this);
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

   TrgCtrl* fTrgCtrl = NULL;
   std::vector<AdcCtrl*> fAdcCtrl;
   std::vector<PwbCtrl*> fPwbCtrl;

   bool fConfEnableTdcTrigger = true;
   bool fConfEnableAdcTrigger = true;
   bool fConfEnablePwbTrigger = true;
   bool fConfTrigPassThrough = false;

   int fNumBanks = 0;

   void WVD(const char* name, const std::vector<double> &v)
   {
      if (fMfe->fShutdownRequested)
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
      if (fMfe->fShutdownRequested)
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
      MVOdb* odbProxy = fMfe->fOdbRoot->Chdir("WebServer/Proxy", true);
      assert(odbProxy);

      // check that LoadOdb() is not called twice
      assert(fTrgCtrl == NULL);

      int countTrg = 0;

      bool enable_trg = true;

      fEq->fOdbEqSettings->RB("TRG/Enable", &enable_trg, true);

      if (enable_trg) {
         std::vector<std::string> names;
         fEq->fOdbEqSettings->RSA("TRG/Modules", &names, true, 1, 32);

         if (names.size() > 0) {
            std::string name = names[0];
            if (name[0] != '#') {
               TrgCtrl* trg = new TrgCtrl(fMfe, fEq, name.c_str(), name.c_str());
               fTrgCtrl = trg;
               countTrg++;
            }
         }
      }

      printf("LoadOdb: TRG_MODULES: %d\n", countTrg);

      // check that Init() is not called twice
      assert(fAdcCtrl.size() == 0);

      bool enable_adc = true;

      fEq->fOdbEqSettings->RB("ADC/enable", &enable_adc, true);

      int countAdc = 0;
         
      if (enable_adc) {
         std::vector<std::string> modules;

         int num_adc = 16;

         fEq->fOdbEqSettings->RSA("ADC/modules", &modules, true, num_adc, 32);
         fEq->fOdbEqSettings->RBA("ADC/boot_user_page", NULL, true, num_adc);
         fEq->fOdbEqSettings->RBA("ADC/adc16_enable", NULL, true, num_adc);
         fEq->fOdbEqSettings->RBA("ADC/adc32_enable", NULL, true, num_adc);

         fEq->fOdbEqSettings->RBA("ADC/DAC/dac_enable", NULL, true, num_adc);
         fEq->fOdbEqSettings->RIA("ADC/DAC/dac_baseline", NULL, true, num_adc);
         fEq->fOdbEqSettings->RIA("ADC/DAC/dac_amplitude", NULL, true, num_adc);
         fEq->fOdbEqSettings->RIA("ADC/DAC/dac_seliq", NULL, true, num_adc);
         fEq->fOdbEqSettings->RIA("ADC/DAC/dac_xor", NULL, true, num_adc);
         fEq->fOdbEqSettings->RIA("ADC/DAC/dac_torb", NULL, true, num_adc);
         fEq->fOdbEqSettings->RBA("ADC/DAC/pulser_enable", NULL, true, num_adc);
         fEq->fOdbEqSettings->RBA("ADC/DAC/ramp_enable", NULL, true, num_adc);
         fEq->fOdbEqSettings->RIA("ADC/DAC/ramp_up_rate", NULL, true, num_adc);
         fEq->fOdbEqSettings->RIA("ADC/DAC/ramp_down_rate", NULL, true, num_adc);
         fEq->fOdbEqSettings->RIA("ADC/DAC/ramp_top_len", NULL, true, num_adc);
         fEq->fOdbEqSettings->RBA("ADC/DAC/fw_pulser", NULL, true, num_adc);

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
               
               adc->fEsper = new EsperComm(name.c_str(), s);
               countAdc++;
            }
               
            fAdcCtrl.push_back(adc);
         }
      }

      printf("LoadOdb: ADC_MODULES: %d\n", countAdc);

      int countPwb = 0;

      bool enable_pwb = true;

      fEq->fOdbEqSettings->RB("PWB/Enable", &enable_pwb, true);
         
      if (enable_pwb) {
         // check that Init() is not called twice
         assert(fPwbCtrl.size() == 0);
         
         std::vector<std::string> modules;

         const int num_pwb = 64;
         const int num_columns = 8;

         fEq->fOdbEqSettings->RBA("PWB/enable_trigger", NULL, true, 1);
         fEq->fOdbEqSettings->RBA("PWB/enable_trigger_column", NULL, true, num_columns);
         fEq->fOdbEqSettings->RSA("PWB/per_pwb_slot/modules", &modules, true, num_pwb, 32);
         fEq->fOdbEqSettings->RBA("PWB/per_pwb_slot/boot_user_page", NULL, true, num_pwb);
         fEq->fOdbEqSettings->RBA("PWB/per_pwb_slot/trigger", NULL, true, num_pwb);
         fEq->fOdbEqSettings->RBA("PWB/per_pwb_slot/ch_force", NULL, true, num_pwb);
         fEq->fOdbEqSettings->RBA("PWB/per_pwb_slot/sata_clock", NULL, true, num_pwb);
         fEq->fOdbEqSettings->RBA("PWB/per_pwb_slot/sata_trigger", NULL, true, num_pwb);
         fEq->fOdbEqSettings->RBA("PWB/per_pwb_slot/sata_master", NULL, true, num_pwb);
         fEq->fOdbEqSettings->RBA("PWB/per_pwb_slot/sata_slave", NULL, true, num_pwb);
         //fEq->fOdbEqSettings->RU32A("PWB/per_pwb_slot/sata_offload_ip", NULL, true, num_pwb);
         fEq->fOdbEqSettings->RIA("PWB/per_pwb_slot/sata_mate", NULL, true, num_pwb);
         fEq->fOdbEqSettings->RDA("PWB/per_pwb_slot_thr/baseline_reset", NULL, true, num_pwb);
         fEq->fOdbEqSettings->RDA("PWB/per_pwb_slot_thr/baseline_fpn", NULL, true, num_pwb);
         fEq->fOdbEqSettings->RDA("PWB/per_pwb_slot_thr/baseline_pads", NULL, true, num_pwb);

         double to_connect = 2.0;
         double to_read = 10.0;
         double to_write = 2.0;

         fEq->fOdbEqSettings->RD("PWB/connect_timeout", &to_connect, true);
         fEq->fOdbEqSettings->RD("PWB/read_timeout",    &to_read, true);
         fEq->fOdbEqSettings->RD("PWB/write_timeout",   &to_write, true);

         for (unsigned i=0; i<modules.size(); i++) {
            std::string name = modules[i];
         
            //printf("index %d name [%s]\n", i, name.c_str());
            
            PwbCtrl* pwb = new PwbCtrl(fMfe, fEq, name.c_str(), i);
            
            if (name.length() > 0 && name[0] != '#') {
               KOtcpConnection* s = new KOtcpConnection(name.c_str(), "http");
            
               s->fConnectTimeoutMilliSec = to_connect*1000;
               s->fReadTimeoutMilliSec = to_read*1000;
               s->fWriteTimeoutMilliSec = to_write*1000;
               s->fHttpKeepOpen = false;
               
               pwb->fEsper = new EsperComm(name.c_str(), s);

               pwb->SetState(ST_INITIAL, "starting...");

               odbProxy->WS(name.c_str(), (std::string("http://") + name).c_str());

               countPwb++;
            }
            
            fPwbCtrl.push_back(pwb);
         }

         fEq->fOdbEqSettings->RIA("PWB/per_pwb_slot_mv2/mv2_enabled",    NULL, true, num_pwb);
         fEq->fOdbEqSettings->RIA("PWB/per_pwb_slot_mv2/mv2_range",      NULL, true, num_pwb);
         fEq->fOdbEqSettings->RIA("PWB/per_pwb_slot_mv2/mv2_resolution", NULL, true, num_pwb);
      }

      printf("LoadOdb: PWB_MODULES: %d\n", countPwb);
         
      fMfe->Msg(MINFO, "LoadOdb", "Found in ODB: %d TRG, %d ADC, %d PWB modules", countTrg, countAdc, countPwb);
   }

   bool StopLocked(bool send_extra_trigger)
   {
      bool ok = true;
      printf("StopLocked!\n");

      if (fTrgCtrl) {
         ok &= fTrgCtrl->StopTrgLocked(send_extra_trigger);
      }

      printf("Creating threads!\n");
      std::vector<std::thread*> t;

      //if (fTrgCtrl) {
      //   t.push_back(new std::thread(&AlphaTctrl::StopAtLocked, fTrgCtrl));
      //}

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i] && fAdcCtrl[i]->fEsper) {
            t.push_back(new std::thread(&AdcCtrl::StopAdcLocked, fAdcCtrl[i]));
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i] && fPwbCtrl[i]->fEsper) {
            t.push_back(new std::thread(&PwbCtrl::StopPwbLocked, fPwbCtrl[i]));
         }
      }

      fMfe->Msg(MINFO, "StopLocked", "StopLocked: threads started!");

      printf("Joining threads!\n");
      for (unsigned i=0; i<t.size(); i++) {
         t[i]->join();
         delete t[i];
      }

      fMfe->Msg(MINFO, "StopLocked", "StopLocked: threads joined!");

      fMfe->Msg(MINFO, "StopLocked", "Stop ok %d", ok);
      return ok;
   }

   bool SoftTriggerLocked()
   {
      bool ok = true;

      if (fTrgCtrl) {
         ok &= fTrgCtrl->SoftTriggerTrgLocked();
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
      int count_trg = 0;
      int adc_countOk = 0;
      int adc_countBad = 0;
      int adc_countDead = 0;
      int pwb_countOk = 0;
      int pwb_countBad = 0;
      int pwb_countDead = 0;

      if (fTrgCtrl) {
         if (fTrgCtrl->fOk) {
            count_trg += 1;
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
            sprintf(buf, "%d TRG, %d ADC Ok, %d PWB Ok", count_trg, adc_countOk, pwb_countOk);
            fEq->SetStatus(buf, "#00FF00");
         } else {
            sprintf(buf, "%d TRG, %d/%d/%d ADC, %d/%d/%d PWB (G/B/D)", count_trg, adc_countOk, adc_countBad, adc_countDead, pwb_countOk, pwb_countBad, pwb_countDead);
            fEq->SetStatus(buf, "yellow");
         }
      }
   }

   void WriteVariables()
   {
      if (fAdcCtrl.size() > 0) {
         std::vector<double> adc_http_time;
         adc_http_time.resize(fAdcCtrl.size(), 0);

         std::vector<int> adc_user_page;
         adc_user_page.resize(fAdcCtrl.size(), 0);

         std::vector<double> adc_temp_fpga;
         adc_temp_fpga.resize(fAdcCtrl.size(), 0);
         
         std::vector<double> adc_temp_board;
         adc_temp_board.resize(fAdcCtrl.size(), 0);
         
         std::vector<double> adc_temp_amp_min;
         adc_temp_amp_min.resize(fAdcCtrl.size(), 0);
         
         std::vector<double> adc_temp_amp_max;
         adc_temp_amp_max.resize(fAdcCtrl.size(), 0);
         
         std::vector<int> adc_state;
         adc_state.resize(fAdcCtrl.size(), 0);

         std::vector<int> adc_sfp_vendor_pn;
         adc_sfp_vendor_pn.resize(fAdcCtrl.size(), 0);

         std::vector<double> adc_sfp_temp;
         adc_sfp_temp.resize(fAdcCtrl.size(), 0);

         std::vector<double> adc_sfp_vcc;
         adc_sfp_vcc.resize(fAdcCtrl.size(), 0);

         std::vector<double> adc_sfp_tx_bias;
         adc_sfp_tx_bias.resize(fAdcCtrl.size(), 0);

         std::vector<double> adc_sfp_tx_power;
         adc_sfp_tx_power.resize(fAdcCtrl.size(), 0);

         std::vector<double> adc_sfp_rx_power;
         adc_sfp_rx_power.resize(fAdcCtrl.size(), 0);

         std::vector<double> adc_trig_esata_cnt;
         adc_trig_esata_cnt.resize(fAdcCtrl.size(), 0);

         std::vector<double> adc_lmk_dac;
         adc_lmk_dac.resize(fAdcCtrl.size(), 0);

         for (unsigned i=0; i<fAdcCtrl.size(); i++) {
            if (fAdcCtrl[i]) {
               if (fAdcCtrl[i]->fEsper) {
                  adc_http_time[i] = fAdcCtrl[i]->fEsper->fMaxHttpTime;
                  fAdcCtrl[i]->fEsper->fMaxHttpTime = 0;
               }

               adc_state[i] = fAdcCtrl[i]->fState;
               adc_user_page[i] = fAdcCtrl[i]->fUserPage;
               adc_temp_fpga[i] = fAdcCtrl[i]->fFpgaTemp;
               adc_temp_board[i] = fAdcCtrl[i]->fSensorTempBoard;
               adc_temp_amp_min[i] = fAdcCtrl[i]->fSensorTempAmpMin;
               adc_temp_amp_max[i] = fAdcCtrl[i]->fSensorTempAmpMax;
               adc_sfp_vendor_pn[i] = fAdcCtrl[i]->fSfpVendorPn;
               adc_sfp_temp[i] = fAdcCtrl[i]->fSfpTemp;
               adc_sfp_vcc[i] = fAdcCtrl[i]->fSfpVcc;
               adc_sfp_tx_bias[i] = fAdcCtrl[i]->fSfpTxBias;
               adc_sfp_tx_power[i] = fAdcCtrl[i]->fSfpTxPower;
               adc_sfp_rx_power[i] = fAdcCtrl[i]->fSfpRxPower;
               adc_trig_esata_cnt[i] = fAdcCtrl[i]->fTrigEsataCnt - fAdcCtrl[i]->fExtTrigCount0;
               adc_lmk_dac[i] = fAdcCtrl[i]->fLmkDac;
            }
         }

         fEq->fOdbEqVariables->WDA("adc_http_time", adc_http_time);
         fEq->fOdbEqVariables->WIA("adc_state", adc_state);
         fEq->fOdbEqVariables->WIA("adc_user_page", adc_user_page);
         fEq->fOdbEqVariables->WDA("adc_temp_fpga", adc_temp_fpga);
         fEq->fOdbEqVariables->WDA("adc_temp_board", adc_temp_board);
         fEq->fOdbEqVariables->WDA("adc_temp_amp_min", adc_temp_amp_min);
         fEq->fOdbEqVariables->WDA("adc_temp_amp_max", adc_temp_amp_max);
         fEq->fOdbEqVariables->WIA("adc_sfp_vendor_pn", adc_sfp_vendor_pn);
         fEq->fOdbEqVariables->WDA("adc_sfp_temp", adc_sfp_temp);
         fEq->fOdbEqVariables->WDA("adc_sfp_vcc",  adc_sfp_vcc);
         fEq->fOdbEqVariables->WDA("adc_sfp_tx_bias",  adc_sfp_tx_bias);
         fEq->fOdbEqVariables->WDA("adc_sfp_tx_power", adc_sfp_tx_power);
         fEq->fOdbEqVariables->WDA("adc_sfp_rx_power", adc_sfp_rx_power);
         fEq->fOdbEqVariables->WDA("adc_trig_esata_cnt", adc_trig_esata_cnt);
         fEq->fOdbEqVariables->WDA("adc_lmk_dac", adc_lmk_dac);
      }

      if (fPwbCtrl.size() > 0) {
         std::vector<std::string> pwb_name;
         pwb_name.resize(fPwbCtrl.size(), "");

         std::vector<double> pwb_http_time;
         pwb_http_time.resize(fPwbCtrl.size(), 0);

         std::vector<int> pwb_user_page;
         pwb_user_page.resize(fPwbCtrl.size(), 0);

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

         std::vector<int> pwb_sfp_vendor_pn;
         pwb_sfp_vendor_pn.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_sfp_temp;
         pwb_sfp_temp.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_sfp_vcc;
         pwb_sfp_vcc.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_sfp_tx_bias;
         pwb_sfp_tx_bias.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_sfp_tx_power;
         pwb_sfp_tx_power.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_sfp_rx_power;
         pwb_sfp_rx_power.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_ext_trig_count;
         pwb_ext_trig_count.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_trigger_total_requested;
         pwb_trigger_total_requested.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_trigger_total_accepted;
         pwb_trigger_total_accepted.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_trigger_total_dropped;
         pwb_trigger_total_dropped.resize(fPwbCtrl.size(), 0);

         std::vector<double> pwb_offload_tx_cnt;
         pwb_offload_tx_cnt.resize(fPwbCtrl.size(), 0);
         
         std::vector<int> pwb_mv2_range;
         pwb_mv2_range.resize(fPwbCtrl.size(), -1);
         std::vector<double> pwb_mv2_xaxis;
         pwb_mv2_xaxis.resize(fPwbCtrl.size(), 0);
         std::vector<double> pwb_mv2_yaxis;
         pwb_mv2_yaxis.resize(fPwbCtrl.size(), 0);
         std::vector<double> pwb_mv2_zaxis;
         pwb_mv2_zaxis.resize(fPwbCtrl.size(), 0);
         std::vector<double> pwb_mv2_taxis;
         pwb_mv2_taxis.resize(fPwbCtrl.size(), 0);

         for (unsigned i=0; i<fPwbCtrl.size(); i++) {
            if (fPwbCtrl[i]) {
               if (fPwbCtrl[i]->fEsper) {
                  pwb_http_time[i] = fPwbCtrl[i]->fEsper->fMaxHttpTime;
                  fPwbCtrl[i]->fEsper->fMaxHttpTime = 0;
               }

               pwb_name[i] = fPwbCtrl[i]->fOdbName;

               pwb_state[i] = fPwbCtrl[i]->fState;

               pwb_user_page[i] = fPwbCtrl[i]->fUserPage;

               pwb_ext_trig_count[i] = fPwbCtrl[i]->fExtTrigCount - fPwbCtrl[i]->fExtTrigCount0;

               pwb_trigger_total_requested[i] = fPwbCtrl[i]->fTriggerTotalRequested - fPwbCtrl[i]->fTriggerTotalRequested0;
               pwb_trigger_total_accepted[i] = fPwbCtrl[i]->fTriggerTotalAccepted - fPwbCtrl[i]->fTriggerTotalAccepted0;
               pwb_trigger_total_dropped[i] = fPwbCtrl[i]->fTriggerTotalDropped - fPwbCtrl[i]->fTriggerTotalDropped0;
               pwb_offload_tx_cnt[i] = fPwbCtrl[i]->fOffloadTxCnt - fPwbCtrl[i]->fOffloadTxCnt0;
               
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

               pwb_sfp_vendor_pn[i] = fPwbCtrl[i]->fSfpVendorPn;
               pwb_sfp_temp[i] = fPwbCtrl[i]->fSfpTemp;
               pwb_sfp_vcc[i] = fPwbCtrl[i]->fSfpVcc;
               pwb_sfp_tx_bias[i] = fPwbCtrl[i]->fSfpTxBias;
               pwb_sfp_tx_power[i] = fPwbCtrl[i]->fSfpTxPower;
               pwb_sfp_rx_power[i] = fPwbCtrl[i]->fSfpRxPower;
               
               if( fPwbCtrl[i]->fMV2enable )
                  {
                     pwb_mv2_range[i] = fPwbCtrl[i]->fMV2range;
                     pwb_mv2_xaxis[i] = fPwbCtrl[i]->fMV2_xaxis;
                     pwb_mv2_yaxis[i] = fPwbCtrl[i]->fMV2_yaxis;
                     pwb_mv2_zaxis[i] = fPwbCtrl[i]->fMV2_zaxis;
                     pwb_mv2_taxis[i] = fPwbCtrl[i]->fMV2_taxis;
                  }
            }
         }
               
         //fEq->fOdbEqVariables->WSA("pwb_names", pwb_name, 32);
         fEq->fOdbEqVariables->WIA("pwb_state", pwb_state);
         WVD("pwb_http_time", pwb_http_time);

         fEq->fOdbEqVariables->WIA("pwb_user_page", pwb_user_page);

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

         fEq->fOdbEqVariables->WIA("pwb_sfp_vendor_pn", pwb_sfp_vendor_pn);
         fEq->fOdbEqVariables->WDA("pwb_sfp_temp", pwb_sfp_temp);
         fEq->fOdbEqVariables->WDA("pwb_sfp_vcc",  pwb_sfp_vcc);
         fEq->fOdbEqVariables->WDA("pwb_sfp_tx_bias",  pwb_sfp_tx_bias);
         fEq->fOdbEqVariables->WDA("pwb_sfp_tx_power", pwb_sfp_tx_power);
         fEq->fOdbEqVariables->WDA("pwb_sfp_rx_power", pwb_sfp_rx_power);

         fEq->fOdbEqVariables->WDA("pwb_ext_trig_count", pwb_ext_trig_count);

         fEq->fOdbEqVariables->WDA("pwb_trigger_total_requested", pwb_trigger_total_requested);
         fEq->fOdbEqVariables->WDA("pwb_trigger_total_accepted", pwb_trigger_total_accepted);
         fEq->fOdbEqVariables->WDA("pwb_trigger_total_dropped", pwb_trigger_total_dropped);

         fEq->fOdbEqVariables->WDA("pwb_offload_tx_cnt", pwb_offload_tx_cnt);

         fEq->fOdbEqVariables->WIA("pwb_mv2_range",pwb_mv2_range);
         fEq->fOdbEqVariables->WDA("pwb_mv2_xaxis",pwb_mv2_xaxis);
         fEq->fOdbEqVariables->WDA("pwb_mv2_yaxis",pwb_mv2_yaxis);
         fEq->fOdbEqVariables->WDA("pwb_mv2_zaxis",pwb_mv2_zaxis);
         fEq->fOdbEqVariables->WDA("pwb_mv2_temp", pwb_mv2_taxis);
      }
   }

   void ThreadPeriodic()
   {
      ThreadReadAndCheck();
      WriteVariables();
   }

   PwbCtrl* FindPwb(const char* name)
   {
      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i]) {
            if (fPwbCtrl[i]->fOdbName == name)
               return fPwbCtrl[i];
         }
      }
      return NULL;
   }

   AdcCtrl* FindAdc(const char* name)
   {
      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i]) {
            if (fAdcCtrl[i]->fOdbName == name)
               return fAdcCtrl[i];
         }
      }
      return NULL;
   }

   std::string HandleRpc(const char* cmd, const char* args)
   {
      fMfe->Msg(MINFO, "HandleRpc", "RPC cmd [%s], args [%s]", cmd, args);
      if (strcmp(cmd, "init_pwb") == 0) {
         PwbCtrl* pwb = FindPwb(args);
         if (pwb && pwb->fEsper) {
            pwb->fLock.lock();
            pwb->InitPwbLocked();
            pwb->fLock.unlock();
            WriteVariables();
            printf("init_pwb done!\n");
         }
      } else if (strcmp(cmd, "check_pwb") == 0) {
         PwbCtrl* pwb = FindPwb(args);
         if (pwb && pwb->fEsper) {
            pwb->fLock.lock();
            pwb->ReadAndCheckPwbLocked();
            pwb->fLock.unlock();
            WriteVariables();
         }
      } else if (strcmp(cmd, "check_pwb_all") == 0) {
         LockAll();
         
         printf("Creating threads!\n");
         std::vector<std::thread*> t;

         for (unsigned i=0; i<fPwbCtrl.size(); i++) {
            if (fPwbCtrl[i] && fPwbCtrl[i]->fEsper) {
               t.push_back(new std::thread(&PwbCtrl::ReadAndCheckPwbLocked, fPwbCtrl[i]));
            }
         }

         printf("Joining threads!\n");
         for (unsigned i=0; i<t.size(); i++) {
            t[i]->join();
            delete t[i];
         }

         UnlockAll();
         WriteVariables();
         printf("Done!\n");
      } else if (strcmp(cmd, "init_pwb_all") == 0) {
         LockAll();
         
         printf("Creating threads!\n");
         std::vector<std::thread*> t;

         for (unsigned i=0; i<fPwbCtrl.size(); i++) {
            if (fPwbCtrl[i] && fPwbCtrl[i]->fEsper) {
               t.push_back(new std::thread(&PwbCtrl::InitPwbLocked, fPwbCtrl[i]));
            }
         }

         printf("Joining threads!\n");
         for (unsigned i=0; i<t.size(); i++) {
            t[i]->join();
            delete t[i];
         }

         UnlockAll();
         WriteVariables();
         printf("Done!\n");
#if 0
      } else if (strcmp(cmd, "reboot_pwb_all") == 0) {
         //
         // NB: reboot_pwb_all has become impossible
         // after code was added to switch clocks to local
         // oscillator before reboot. each pwb has
         // to change the clock of it's sata mate. before
         // writing to the mate, it has to lock it,
         // but all pwb's are already locked and we become
         // stuck. (dead lock due to locking order voilation).
         // K.O. Aug 2020.
         // 


         LockAll();
         
         printf("Creating threads!\n");
         std::vector<std::thread*> t;

         for (unsigned i=0; i<fPwbCtrl.size(); i++) {
            if (fPwbCtrl[i] && fPwbCtrl[i]->fEsper) {
               t.push_back(new std::thread(&PwbCtrl::RebootPwbLocked, fPwbCtrl[i]));
            }
         }

         printf("Joining threads!\n");
         for (unsigned i=0; i<t.size(); i++) {
            t[i]->join();
            delete t[i];
         }

         UnlockAll();
         printf("Done!\n");
#endif
      } else if (strcmp(cmd, "reboot_pwb") == 0) {
         PwbCtrl* pwb = FindPwb(args);
         if (pwb && pwb->fEsper) {
            pwb->fLock.lock();
            pwb->RebootPwbLocked();
            pwb->fLock.unlock();
            WriteVariables();
         }
      } else if (strcmp(cmd, "init_adc") == 0) {
         AdcCtrl* adc = FindAdc(args);
         if (adc && adc->fEsper) {
            adc->fLock.lock();
            adc->InitAdcLocked();
            adc->fLock.unlock();
            WriteVariables();
         }
      } else if (strcmp(cmd, "init_adc_dac") == 0) {
         AdcCtrl* adc = FindAdc(args);
         if (adc && adc->fEsper) {
            adc->fLock.lock();
            adc->ConfigureAdcDacLocked();
            adc->fLock.unlock();
         }
      } else if (strcmp(cmd, "check_adc") == 0) {
         AdcCtrl* adc = FindAdc(args);
         if (adc && adc->fEsper) {
            adc->fLock.lock();
            adc->ReadAndCheckAdcLocked();
            adc->fLock.unlock();
            WriteVariables();
         }
      } else if (strcmp(cmd, "check_adc_all") == 0) {
         LockAll();
         
         printf("Creating threads!\n");
         std::vector<std::thread*> t;

         for (unsigned i=0; i<fAdcCtrl.size(); i++) {
            if (fAdcCtrl[i] && fAdcCtrl[i]->fEsper) {
               t.push_back(new std::thread(&AdcCtrl::ReadAndCheckAdcLocked, fAdcCtrl[i]));
            }
         }

         printf("Joining threads!\n");
         for (unsigned i=0; i<t.size(); i++) {
            t[i]->join();
            delete t[i];
         }

         UnlockAll();
         WriteVariables();
         printf("Done!\n");
      } else if (strcmp(cmd, "init_adc_all") == 0) {
         LockAll();
         
         printf("Creating threads!\n");
         std::vector<std::thread*> t;

         for (unsigned i=0; i<fAdcCtrl.size(); i++) {
            if (fAdcCtrl[i] && fAdcCtrl[i]->fEsper) {
               t.push_back(new std::thread(&AdcCtrl::InitAdcLocked, fAdcCtrl[i]));
            }
         }

         printf("Joining threads!\n");
         for (unsigned i=0; i<t.size(); i++) {
            t[i]->join();
            delete t[i];
         }

         UnlockAll();
         WriteVariables();
         printf("Done!\n");
      } else if (strcmp(cmd, "init_adc_dac_all") == 0) {
         LockAll();

#if 0         
         printf("Creating threads!\n");
         std::vector<std::thread*> t;
#endif

         for (unsigned i=0; i<fAdcCtrl.size(); i++) {
            //printf("slot %d, %p\n", i, fAdcCtrl[i]);
            if (fAdcCtrl[i] && fAdcCtrl[i]->fEsper) {
               printf("slot %d, %p, name [%s]\n", i, fAdcCtrl[i], fAdcCtrl[i]->fOdbName.c_str());
               //t.push_back(new std::thread(&AdcCtrl::ConfigureAdcDacLocked, fAdcCtrl[i]));
               fAdcCtrl[i]->ConfigureAdcDacLocked();
            }
         }

#if 0
         ::sleep(1);

         printf("Joining threads!\n");
         for (unsigned i=0; i<t.size(); i++) {
            printf("join %d\n", i);
            t[i]->join();
            printf("join %d done\n", i);
            delete t[i];
         }
#endif

         UnlockAll();
         printf("Done!\n");
      } else if (strcmp(cmd, "reboot_adc") == 0) {
         AdcCtrl* adc = FindAdc(args);
         if (adc && adc->fEsper) {
            adc->fLock.lock();
            adc->RebootAdcLocked();
            adc->fLock.unlock();
         }
      } else if (strcmp(cmd, "init_trg") == 0) {
         if (fTrgCtrl) {
            fTrgCtrl->fLock.lock();
            fTrgCtrl->ConfigureTrgLocked(fConfEnableAdcTrigger, fConfEnablePwbTrigger, fConfEnableTdcTrigger);
            fTrgCtrl->ReadTrgLocked();
            fTrgCtrl->fLock.unlock();
         }
      } else if (strcmp(cmd, "update_pulser_freq") == 0) {
         if (fTrgCtrl) {
            fTrgCtrl->fLock.lock();
            fTrgCtrl->UpdatePulserFreqTrgLocked();
            fTrgCtrl->ReadTrgLocked();
            fTrgCtrl->fLock.unlock();
         }
      } else if (strcmp(cmd, "start_trg") == 0) {
         if (fTrgCtrl) {
            fTrgCtrl->XStartTrg();
         }
      } else if (strcmp(cmd, "stop_trg") == 0) {
         if (fTrgCtrl) {
            fTrgCtrl->fLock.lock();
            fTrgCtrl->XStopTrgLocked();
            fTrgCtrl->fLock.unlock();
         }
      } else if (strcmp(cmd, "load_mlu_trg") == 0) {
         if (fTrgCtrl) {
            fTrgCtrl->fLock.lock();
            fTrgCtrl->LoadMluLocked();
            fTrgCtrl->fLock.unlock();
         }
      } else if (strcmp(cmd, "read_trg") == 0) {
         if (fTrgCtrl) {
            fTrgCtrl->fLock.lock();
            fTrgCtrl->ReadTrgLocked();
            fTrgCtrl->fLock.unlock();
         }
      } else if (strcmp(cmd, "reboot_trg") == 0) {
         if (fTrgCtrl) {
            fTrgCtrl->fLock.lock();
            fTrgCtrl->RebootTrgLocked();
            fTrgCtrl->fLock.unlock();
         }
      } else if (strcmp(cmd, "select_adc_trg") == 0) {
         int iadc = strtoul(args, NULL, 0);
         if (fTrgCtrl) {
            fTrgCtrl->fLock.lock();
            fTrgCtrl->SelectAdcLocked(iadc);
            fTrgCtrl->fLock.unlock();
         }
      }
      return "OK";
   }

   void LockAll()
   {
      printf("LockAll...\n");

      printf("Creating threads!\n");
      std::vector<std::thread*> t;

      if (fTrgCtrl) {
         t.push_back(new std::thread(&TrgCtrl::Lock, fTrgCtrl));
      }

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i]) {
            t.push_back(new std::thread(&AdcCtrl::Lock, fAdcCtrl[i]));
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i]) {
            t.push_back(new std::thread(&PwbCtrl::Lock, fPwbCtrl[i]));
         }
      }

      printf("Joining threads!\n");
      for (unsigned i=0; i<t.size(); i++) {
         t[i]->join();
         delete t[i];
      }

      printf("LockAll...done\n");
   }

   void UnlockAll()
   {
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

      if (fTrgCtrl) {
         fTrgCtrl->fLock.unlock();
      }

      printf("UnlockAll...done\n");
   }

   void WriteEvbConfigLocked()
   {
      const int ts625 =  62500000;
      //const int ts100 = 100000000;
      const int ts125 = 125000000;

      int countTrg = 0;
      int countAdc = 0;
      int countPwb = 0;
      int countTdc = 0;

      std::vector<std::string> name;
      std::vector<int> type;
      std::vector<int> module;
      std::vector<int> nbanks;
      std::vector<double> tsfreq;

      if (fTrgCtrl) {
         name.push_back(fTrgCtrl->fOdbName);
         type.push_back(1);
         module.push_back(fTrgCtrl->fModule);
         nbanks.push_back(1);
         tsfreq.push_back(ts625);
         countTrg++;
      }

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i] && fAdcCtrl[i]->fEsper) {
            if (fAdcCtrl[i]->fNumBanks < 1)
               continue;
            if (fAdcCtrl[i]->fModule < 1)
               continue;
            if (!fConfEnableAdcTrigger)
               continue;
            if (fAdcCtrl[i]->fConfAdc16Enable) {
               name.push_back(fAdcCtrl[i]->fOdbName);
               type.push_back(2);
               module.push_back(fAdcCtrl[i]->fModule);
               nbanks.push_back(fAdcCtrl[i]->fNumBanksAdc16);
               tsfreq.push_back(ts125);
               countAdc++;
            }
            if (fAdcCtrl[i]->fConfAdc32Enable) {
               name.push_back(fAdcCtrl[i]->fOdbName + "/adc32");
               type.push_back(2);
               module.push_back(fAdcCtrl[i]->fModule + 100);
               nbanks.push_back(fAdcCtrl[i]->fNumBanksAdc32);
               tsfreq.push_back(ts125);
               countAdc++;
            }
         }
      }

      if (fConfEnableTdcTrigger) {
         name.push_back("tdc01");
         type.push_back(6);
         module.push_back(0);
         nbanks.push_back(1);
         tsfreq.push_back(97656.25); // 200MHz/(2<<11)
         countTdc++;
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i] && fPwbCtrl[i]->fEsper) {
            if (fPwbCtrl[i]->fNumBanks < 1)
               continue;
            if (fPwbCtrl[i]->fModule < 0)
               continue;
            if (!fPwbCtrl[i]->fConfTrigger)
               continue;
            if (fPwbCtrl[i]->fState != ST_GOOD)
               continue;
            if (!fConfEnablePwbTrigger)
               continue;
            if (fPwbCtrl[i]->fHaveHwUdp) {
               for (int j=0; j<4; j++) {
                  std::string xname = fPwbCtrl[i]->fOdbName + "/" + toString(j);
                  name.push_back(xname);
                  type.push_back(5);
                  module.push_back(fPwbCtrl[i]->fModule);
                  nbanks.push_back(1);
                  tsfreq.push_back(ts125);
                  countPwb++;
               }
            } else {
               name.push_back(fPwbCtrl[i]->fOdbName);
               type.push_back(4);
               module.push_back(fPwbCtrl[i]->fModule);
               nbanks.push_back(fPwbCtrl[i]->fNumBanks);
               tsfreq.push_back(ts125);
               countPwb++;
            }
         }
      }

      gEvbC->WSA("name", name, 32);
      gEvbC->WIA("type", type);
      gEvbC->WIA("module", module);
      gEvbC->WIA("nbanks", nbanks);
      gEvbC->WDA("tsfreq", tsfreq);

      fMfe->Msg(MINFO, "WriteEvbConfig", "Wrote EVB configuration to ODB: %d TRG, %d ADC, %d TDC, %d PWB slots", countTrg, countAdc, countTdc, countPwb);
   }

   void BeginRun(bool start)
   {
      fMfe->Msg(MINFO, "BeginRun", "Begin run begin!");

      fEq->fOdbEqSettings->RB("TRG/PassThrough",    &fConfTrigPassThrough, true);
      fEq->fOdbEqSettings->RB("ADC/Trigger",        &fConfEnableAdcTrigger, true);
      fEq->fOdbEqSettings->RB("PWB/enable_trigger", &fConfEnablePwbTrigger, true);
      fEq->fOdbEqSettings->RB("TDC/Trigger",        &fConfEnableTdcTrigger, true);


      DWORD t0 = ss_millitime();

      LockAll();

      DWORD t1 = ss_millitime();

      fMfe->Msg(MINFO, "BeginRun", "Begin run locked!");

      gBeginRunStartThreadsTime = TMFE::GetTime();

      printf("Creating threads!\n");
      std::vector<std::thread*> t;

      if (fTrgCtrl) {
         t.push_back(new std::thread(&TrgCtrl::BeginRunTrgLocked, fTrgCtrl, start, fConfEnableAdcTrigger, fConfEnablePwbTrigger, fConfEnableTdcTrigger));
      }

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i]) {
            t.push_back(new std::thread(&AdcCtrl::BeginRunAdcLocked, fAdcCtrl[i], start, fConfEnableAdcTrigger && !fConfTrigPassThrough));
         }
      }

      for (unsigned i=0; i<fPwbCtrl.size(); i++) {
         if (fPwbCtrl[i]) {
            t.push_back(new std::thread(&PwbCtrl::BeginRunPwbLocked, fPwbCtrl[i], start, fConfEnablePwbTrigger && !fConfTrigPassThrough));
         }
      }

      DWORD t2 = ss_millitime();

      fMfe->Msg(MINFO, "BeginRun", "Begin run threads started!");

      printf("Joining threads!\n");
      for (unsigned i=0; i<t.size(); i++) {
         t[i]->join();
         delete t[i];
      }

      DWORD t3 = ss_millitime();

      fMfe->Msg(MINFO, "BeginRun", "Begin run threads joined!");

      WriteEvbConfigLocked();

      int num_banks = 0;

      if (fTrgCtrl) {
         num_banks += fTrgCtrl->fNumBanks;
      }

      for (unsigned i=0; i<fAdcCtrl.size(); i++) {
         if (fAdcCtrl[i] && fConfEnableAdcTrigger) {
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

      DWORD t4 = ss_millitime();

      if (fTrgCtrl && start) {
         fTrgCtrl->StartTrgLocked();
      }

      DWORD t5 = ss_millitime();

      fMfe->Msg(MINFO, "BeginRun", "Begin run unlocking!");

      UnlockAll();

      DWORD te = ss_millitime();

      fMfe->Msg(MINFO, "BeginRun", "Begin run unlocked!");

      fMfe->Msg(MINFO, "BeginRun", "Begin run done in %d ms: lock %d, threads start %d, join %d, evb %d, trg %d, unlock %d", te-t0, t1-t0, t2-t1, t3-t2, t4-t3, t5-t4, te-t5);
   }

   void HandleBeginRun()
   {
      fMfe->Msg(MINFO, "HandleBeginRun", "Begin run!");
      BeginRun(true);
   }

   void HandleEndRun()
   {
      fMfe->Msg(MINFO, "HandleEndRun", "End run begin!");
      LockAll();
      fMfe->Msg(MINFO, "HandleEndRun", "End run locked!");
      bool send_extra_trigger = true;
      StopLocked(send_extra_trigger);
      fMfe->Msg(MINFO, "HandleEndRun", "End run stopped!");
      UnlockAll();
      fMfe->Msg(MINFO, "HandleEndRun", "End run unlocked!");
      fMfe->Msg(MINFO, "HandleEndRun", "End run done!");
   }

   void HandlePauseRun()
   {
      fMfe->Msg(MINFO, "HandlePauseRun", "Pause run!");
      if (fTrgCtrl) {
         fTrgCtrl->fLock.lock();
         fTrgCtrl->XStopTrgLocked();
         fTrgCtrl->fLock.unlock();
      }
   }

   void HandleResumeRun()
   {
      fMfe->Msg(MINFO, "HandleResumeRun", "Resume run!");
      if (fTrgCtrl) {
         fTrgCtrl->XStartTrg();
      }
   }

   void StartThreads()
   {
      // ensure threads are only started once
      static bool gOnce = true;
      assert(gOnce == true);
      gOnce = false;

      if (fTrgCtrl) {
         fTrgCtrl->StartThreads();
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

      if (fTrgCtrl)
         fTrgCtrl->JoinThreads();

      printf("Ctrl::JoinThreads: TRG done!\n");

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

Ctrl* gCtrl = NULL; // kludge warning!

PwbCtrl* FindPwbMate(const PwbCtrl* pwb)
{
   int sata_mate = -1;
   gCtrl->fEq->fOdbEqSettings->RIAI("PWB/per_pwb_slot/sata_mate",  pwb->fOdbIndex, &sata_mate);
   for (unsigned i=0; i<gCtrl->fPwbCtrl.size(); i++) {
      if (gCtrl->fPwbCtrl[i]) {
         if (gCtrl->fPwbCtrl[i]->fModule == sata_mate) {
            printf("FindPwbMate: mate for [%s] %d is %d [%s]\n", pwb->fOdbName.c_str(), pwb->fOdbIndex, sata_mate, gCtrl->fPwbCtrl[i]->fOdbName.c_str());
            return gCtrl->fPwbCtrl[i];
         }
      }
   }
   return NULL;
}

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
   eqc->Buffer = "BUFTRG";
   
   TMFeEquipment* eq = new TMFeEquipment(mfe, "CTRL", eqc);
   eq->Init();
   eq->SetStatus("Starting...", "white");

   mfe->RegisterEquipment(eq);

   gEvbC = mfe->fOdbRoot->Chdir(("Equipment/" + eq->fName + "/EvbConfig").c_str(), true);

   eq->fOdbEq->Delete("PwbState");
   gPwbState = eq->fOdbEq->Chdir("PwbState", true);

   Ctrl* ctrl = new Ctrl;

   ctrl->fMfe = mfe;
   ctrl->fEq = eq;

   mfe->RegisterRpcHandler(ctrl);
   mfe->SetTransitionSequenceStart(910);
   mfe->SetTransitionSequenceStop(90);
   //mfe->DeregisterTransitionPause();
   //mfe->DeregisterTransitionResume();

   ctrl->LoadOdb();

   gCtrl = ctrl; // kludge warning!

   if (1) {
      int run_state = 0;
      mfe->fOdbRoot->RI("Runinfo/State", &run_state);
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

   while (!mfe->fShutdownRequested) {
      time_t now = time(NULL);

      if (now > next_periodic) {
         ctrl->ThreadPeriodic();
         next_periodic += 5;
      }

      {
         std::vector<TrgData*> atbuf;

         {
            std::lock_guard<std::mutex> lock(gTrgDataBufLock);
            //printf("Have events: %d\n", gTrgDataBuf.size());
            for (unsigned i=0; i<gTrgDataBuf.size(); i++) {
               atbuf.push_back(gTrgDataBuf[i]);
               gTrgDataBuf[i] = NULL;
            }
            gTrgDataBuf.clear();
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

      mfe->PollMidas(100);
      if (mfe->fShutdownRequested)
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
