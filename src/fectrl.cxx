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

static std::mutex gOdbLock;
#define LOCK_ODB() std::lock_guard<std::mutex> lock(gOdbLock)
//#define LOCK_ODB() TMFE_LOCK_MIDAS(mfe)

#define C(x) ((x).c_str())

static std::string toString(int value)
{
   char buf[256];
   sprintf(buf, "%d", value);
   return buf;
}

static int odbReadArraySize(TMFE* mfe, const char*name)
{
   int status;
   HNDLE hdir = 0;
   HNDLE hkey;
   KEY key;

   LOCK_ODB();

   status = db_find_key(mfe->fDB, hdir, (char*)name, &hkey);
   if (status != DB_SUCCESS)
      return 0;

   status = db_get_key(mfe->fDB, hkey, &key);
   if (status != DB_SUCCESS)
      return 0;

   return key.num_values;
}

#if 0
static int odbResizeArray(TMFE* mfe, const char*name, int tid, int size)
{
   int oldSize = odbReadArraySize(mfe, name);

   if (oldSize >= size)
      return oldSize;

   int status;
   HNDLE hkey;
   HNDLE hdir = 0;

   LOCK_ODB();

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

   LOCK_ODB();

   char bufi[256];
   sprintf(bufi,"[%d]", i);

   status = db_get_value(mfe->fDB, 0, C(path + bufi), &v, &size, TID_DOUBLE, TRUE);

   return v;
}
#endif

int OdbGetInt(TMFE* mfe, const char* path, int default_value, bool create)
{
   LOCK_ODB();

   int v = 0;
   int size = sizeof(v);

   int status = db_get_value(mfe->fDB, 0, path, &v, &size, TID_INT, FALSE);

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
   std::map<std::string,int> i; // integer variables
   std::map<std::string,double> d; // double variables
   std::map<std::string,bool> b; // boolean variables
   std::map<std::string,std::vector<int>> ia; // integer array variables
   std::map<std::string,std::vector<double>> da; // double array variables
   std::map<std::string,std::vector<bool>> ba; // boolean array variables
};

typedef std::map<std::string,EsperModuleData> EsperNodeData;

class Alpha16ctrl
{
public:
   TMFE* mfe = NULL;
   TMFeEquipment* eq = NULL;
   KOtcpConnection* s = NULL;

   std::string fOdbName;
   int fOdbIndex = -1;

   bool fVerbose = false;

   bool fFailed = false;

   bool fOk = true;

   int fPollSleep = 10;
   int fFailedSleep = 10;

   std::mutex fLock;

   int fNumBanks = 0;

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

      LOCK_ODB();

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

      LOCK_ODB();

      //printf("Write ODB %s : %s\n", C(path), v);
      int status = db_set_value(mfe->fDB, 0, C(path), &v[0], sizeof(int)*v.size(), v.size(), TID_INT);
      if (status != DB_SUCCESS) {
         printf("WR: db_set_value status %d\n", status);
      }
   }
         
   void WRD(const char* mid, const char* vid, const std::vector<double>& v)
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

      LOCK_ODB();
   
      //printf("Write ODB %s : %s\n", C(path), v);
      int status = db_set_value(mfe->fDB, 0, C(path), &v[0], sizeof(double)*v.size(), v.size(), TID_DOUBLE);
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

      LOCK_ODB();

      int status = db_set_value(mfe->fDB, 0, C(path), bb, sizeof(BOOL)*v.size(), v.size(), TID_BOOL);
      if (status != DB_SUCCESS) {
         printf("WR: db_set_value status %d\n", status);
      }

      delete bb;
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

   KOtcpError GetModules(std::vector<std::string>* mid)
   {
      std::vector<std::string> headers;
      std::vector<std::string> reply_headers;
      std::string reply_body;

      KOtcpError e = s->HttpGet(headers, "/read_node?includeMods=y", &reply_headers, &reply_body);

      if (e.error) {
         //LOCK_ODB();
         //eq->SetStatus("http error", "red");
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

   KOtcpError ReadVariables(const std::string& mid, EsperModuleData* vars)
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

      KOtcpError e = s->HttpGet(headers, url.c_str(), &reply_headers, &reply_body);

      if (e.error) {
         //LOCK_ODB();
         //eq->SetStatus("http error", "red");
         mfe->Msg(MERROR, "ReadVariables", "HttpGet() error %s", e.message.c_str());
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
                        WR(mid.c_str(), vid.c_str(), vaed->Stringify().c_str());
                     } else if (type == 1 || type == 2 || type == 3 || type == 4 || type == 5 || type == 6) {
                        std::vector<int> val = JsonToIntArray(vaed);
                        if (val.size() == 1)
                           vars->i[vid] = val[0];
                        else
                           vars->ia[vid] = val;
                        WRI(mid.c_str(), vid.c_str(), val);
                     } else if (type == 9) {
                        std::vector<double> val = JsonToDoubleArray(vaed);
                        if (val.size() == 1)
                           vars->d[vid] = val[0];
                        else
                           vars->da[vid] = val;
                        WRD(mid.c_str(), vid.c_str(), val);
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
                     } else if (type == 13) {
                        WR(mid.c_str(), vid.c_str(), vaed->Stringify().c_str());
                     } else {
                        printf("mid [%s] vid [%s] type %d json value %s\n", mid.c_str(), vid.c_str(), type, vaed->Stringify().c_str());
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

   bool ReadAll(EsperNodeData* data)
   {
      if (fVerbose)
         printf("Reading %s\n", fOdbName.c_str());

      if (fFailed)
         return false;

      std::vector<std::string> modules;

      KOtcpError e = GetModules(&modules);

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
         //LOCK_ODB();
         //eq->SetStatus("http error", "red");
         mfe->Msg(MERROR, "Read", "HttpGet() error %s", e.message.c_str());
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

   bool Write(const char* mid, const char* vid, const char* json)
   {
      if (fFailed)
         return false;

      std::string url;
      url += "/write_var?";
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

      KOtcpError e = s->HttpPost(headers, url.c_str(), json, &reply_headers, &reply_body);

      if (e.error) {
         //LOCK_ODB();
         //eq->SetStatus("http error", "red");
         mfe->Msg(MERROR, "Write", "HttpPost() error %s", e.message.c_str());
         fFailed = true;
         return false;
      }

      if (reply_body.find("error") != std::string::npos) {
         //LOCK_ODB();
         //eq->SetStatus("http error", "red");
         mfe->Msg(MERROR, "Write", "AJAX write %s.%s value \"%s\" error: %s", mid, vid, json, reply_body.c_str());
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

   std::string Read(const char* mid, const char* vid, std::string* last_errmsg = NULL)
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

      KOtcpError e = s->HttpGet(headers, url.c_str(), &reply_headers, &reply_body);

      if (e.error) {
         if (!last_errmsg || e.message != *last_errmsg) {
            mfe->Msg(MERROR, "Read", "HttpGet() error %s", e.message.c_str());
            *last_errmsg = e.message;
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

   int fLmkPll1lcnt = 0;
   int fLmkPll2lcnt = 0;

   double fFpgaTemp = 0;
   double fSensorTemp0 = 0;
   double fSensorTempMax = 0;
   double fSensorTempMin = 0;

   bool Check(EsperNodeData data)
   {
      if (fFailed) {
         printf("%s: failed\n", fOdbName.c_str());
         return false;
      }

      int run_state = OdbGetInt(mfe, "/Runinfo/State", 0, false);
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
      double fpga_temp = data["board"].d["fpga_temp"];
      int clk_lmk = data["board"].i["clk_lmk"];
      int lmk_pll1_lcnt = data["board"].i["lmk_pll1_lcnt"];
      int lmk_pll2_lcnt = data["board"].i["lmk_pll2_lcnt"];
      int lmk_pll1_lock = data["board"].b["lmk_pll1_lock"];
      int lmk_pll2_lock = data["board"].b["lmk_pll2_lock"];

      printf("%s: fpga temp: %.0f, freq_esata: %d, clk_lmk %d lock %d %d lcnt %d %d, run %d, nim %d %d, esata %d %d, trig %d %d, udp %d, tx_cnt %d\n",
             fOdbName.c_str(),
             fpga_temp,
             freq_esata,
             clk_lmk,
             lmk_pll1_lock, lmk_pll2_lock,
             lmk_pll1_lcnt, lmk_pll2_lcnt,
             force_run, nim_ena, nim_inv, esata_ena, esata_inv, trig_nim_cnt, trig_esata_cnt, udp_enable, udp_tx_cnt);

      if (freq_esata == 0) {
         if (LogOnce("board.freq_esata.missing"))
            mfe->Msg(MERROR, "Check", "ALPHA16 %s: no ESATA clock", fOdbName.c_str());
         ok = false;
      } else {
         LogOk("board.freq_esata.missing");
      }

      if (freq_esata != 62500000) {
         if (LogOnce("board.freq_esata.locked"))
            mfe->Msg(MERROR, "Check", "ALPHA16 %s: not locked to ESATA clock", fOdbName.c_str());
         ok = false;
      } else {
         LogOk("board.freq_esata.locked");
      }

      if (!lmk_pll1_lock || !lmk_pll2_lock) {
         if (LogOnce("board.lmk_lock"))
            mfe->Msg(MERROR, "Check", "ALPHA16 %s: LMK PLL not locked", fOdbName.c_str());
         ok = false;
      } else {
         LogOk("board.lmk_lock");
      }

      if (lmk_pll1_lcnt != fLmkPll1lcnt) {
         mfe->Msg(MERROR, "Check", "ALPHA16 %s: LMK PLL1 lock count changed %d to %d", fOdbName.c_str(), fLmkPll1lcnt, lmk_pll1_lcnt);
         fLmkPll1lcnt = lmk_pll1_lcnt;
      }

      if (lmk_pll2_lcnt != fLmkPll2lcnt) {
         mfe->Msg(MERROR, "Check", "ALPHA16 %s: LMK PLL2 lock count changed %d to %d", fOdbName.c_str(), fLmkPll2lcnt, lmk_pll2_lcnt);
         fLmkPll2lcnt = lmk_pll2_lcnt;
      }

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

   bool Identify()
   {
      if (fFailed) {
         fFailed = false;
      } else {
         fLastErrmsg = "";
      }

      std::string elf_buildtime = Read("board", "elf_buildtime", &fLastErrmsg);

      if (!elf_buildtime.length() > 0)
         return false;

      std::string sw_qsys_ts = Read("board", "sw_qsys_ts", &fLastErrmsg);

      if (!sw_qsys_ts.length() > 0)
         return false;

      std::string hw_qsys_ts = Read("board", "hw_qsys_ts", &fLastErrmsg);

      if (!hw_qsys_ts.length() > 0)
         return false;

      mfe->Msg(MINFO, "Identify", "ALPHA16 %s firmware 0x%08x-0x%08x-0x%08x", fOdbName.c_str(), xatoi(elf_buildtime.c_str()), xatoi(sw_qsys_ts.c_str()), xatoi(hw_qsys_ts.c_str()));

      if (xatoi(elf_buildtime.c_str()) != 0x59555815) {
         mfe->Msg(MINFO, "Identify", "ALPHA16 %s firmware is not compatible with the daq", fOdbName.c_str());
         return false;
      }

      return true;
   }

   bool Configure()
   {
      if (fFailed) {
         printf("Configure %s: failed flag\n", fOdbName.c_str());
         return false;
      }

      int udp_port = OdbGetInt(mfe, "/Equipment/UDP/Settings/udp_port", 0, false);

      int adc16_enable = 1;
      int adc16_samples = OdbGetInt(mfe, "/Equipment/CTRL/Settings/adc16_samples", 700, true);
      int adc16_trig_delay = OdbGetInt(mfe, "/Equipment/CTRL/Settings/adc16_trig_delay", 0, true);
      int adc16_trig_start = OdbGetInt(mfe, "/Equipment/CTRL/Settings/adc16_trig_start", 150, true);

      int adc32_enable = OdbGetInt(mfe, (std::string("/Equipment/CTRL/Settings/adc32_enable[" + toString(fOdbIndex) + "]").c_str()), 0, true);

      printf("Configure %s: udp_port %d, adc16 samples %d, trig_delay %d, trig_start %d, adc32 enable %d\n", fOdbName.c_str(), udp_port, adc16_samples, adc16_trig_delay, adc16_trig_start, adc32_enable);

      bool ok = true;

      ok &= Stop();

      // make sure everything is stopped

      ok &= Write("board", "force_run", "false");
      ok &= Write("udp", "enable", "false");

      // switch clock to ESATA

      ok &= Write("board", "clk_lmk", "1");

      // configure the ADCs

      fNumBanks = 0;

      if (adc16_enable) {
         fNumBanks += 16;
      }

      {
         std::string json;
         json += "[";
         for (int i=0; i<16; i++) {
            json += toString(adc16_trig_delay);
            json += ",";
         }
         json += "]";
         
         ok &= Write("adc16", "trig_delay", json.c_str());
      }

      {
         std::string json;
         json += "[";
         for (int i=0; i<16; i++) {
            json += toString(adc16_trig_start);
            json += ",";
         }
         json += "]";
         
         ok &= Write("adc16", "trig_start", json.c_str());
      }

      {
         std::string json;
         json += "[";
         for (int i=0; i<16; i++) {
            json += toString(adc16_samples);
            json += ",";
         }
         json += "]";
         
         ok &= Write("adc16", "trig_stop", json.c_str());
      }

      if (adc32_enable) {
         fNumBanks += 32;
      }

      {
         std::string json;
         json += "[";
         for (int i=0; i<32; i++) {
            if (adc32_enable) {
               json += "true";
            } else {
               json += "false";
            }
            json += ",";
         }
         json += "]";
         
         ok &= Write("fmc32", "enable", json.c_str());
      }

      // program the IP address and port number in the UDP transmitter

      int udp_ip = 0;
      udp_ip |= (192<<24);
      udp_ip |= (168<<16);
      udp_ip |= (1<<8);
      udp_ip |= (1<<0);

      ok &= Write("udp", "dst_ip", toString(udp_ip).c_str());
      ok &= Write("udp", "dst_port", toString(udp_port).c_str());
      ok &= Write("udp", "enable", "true");

      return ok;
   }

   bool Start()
   {
      bool ok = true;
      ok &= Write("board", "nim_ena", "true");
      ok &= Write("board", "esata_ena", "true");
      ok &= Write("board", "force_run", "true");
      return ok;
   }

   bool Stop()
   {
      bool ok = true;
      ok &= Write("board", "force_run", "false");
      ok &= Write("board", "nim_ena", "false");
      ok &= Write("board", "esata_ena", "false");
      return ok;
   }

   bool SoftTrigger()
   {
      //printf("SoftTrigger!\n");
      bool ok = true;
      ok &= Write("board", "nim_inv", "true");
      ok &= Write("board", "nim_inv", "false");
      //printf("SoftTrigger done!\n");
      return ok;
   }

   void Thread()
   {
      printf("thread for %s started\n", fOdbName.c_str());
      while (!mfe->fShutdown) {
         if (fFailed) {
            std::lock_guard<std::mutex> lock(fLock);
            bool ok = Identify();
            if (!ok) {
               fOk = false;
               sleep(fFailedSleep);
               continue;
            }
         }

         {
            std::lock_guard<std::mutex> lock(fLock);

            EsperNodeData e;
            bool ok = ReadAll(&e);
            if (ok) {
               ok = Check(e);
               if (ok)
                  fOk = true;
            }
            if (!ok)
               fOk = false;
         }

         sleep(fPollSleep);
      }
      printf("thread for %s shutdown\n", fOdbName.c_str());
   }
};

void start_a16_thread(Alpha16ctrl* a16)
{
   a16->Thread();
}

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

class AlphaTctrl
{
public:
   TMFE* mfe = NULL;
   TMFeEquipment* eq = NULL;

   std::string fHostname;
   std::string fOdbName;

   bool fVerbose = false;

   bool fFailed = false;

   bool fOk = true;

   int fPollSleep = 10;
   int fFailedSleep = 10;

   std::mutex fLock;

   int fUpdateCount = 0;

   struct sockaddr fCmdAddr;
   struct sockaddr fDataAddr;
   int fCmdAddrLen = 0;
   int fDataAddrLen = 0;
   int fDataSocket = 0;
   int fCmdSocket = 0;

   ///////////////////////////////////////////////////////////////////////////
   //////////////////////      network data link      ////////////////////////

   int sndmsg(int sock_fd, const struct sockaddr *addr, int addr_len, const char *message, int msglen)
   {
      int flags=0;
      int bytes = sendto(sock_fd, message, msglen, flags, addr, addr_len);
      if (bytes < 0) {
         fprintf(stderr,"sndmsg: sendto failed, errno %d (%s)\n", errno, strerror(errno));
         return -1;
      }
      if (bytes != msglen) {
         fprintf(stderr,"sndmsg: sendto failed, short return %d\n", bytes);
         return -1;
      }
      return bytes;
   }
   
   // select: -ve=>error, zero=>no-data +ve=>data-avail
   int testmsg(int socket, int timeout)
   {
      //printf("testmsg: timeout %d\n", timeout);
      
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = timeout;

      fd_set read_fds;
      FD_ZERO(&read_fds);
      FD_SET(socket, &read_fds);

      int num_fd = socket+1;
      int ret = select(num_fd, &read_fds, NULL, NULL, &tv);
      //printf("ret %d, errno %d (%s)\n", ret, errno, strerror(errno));
      return ret;
   }
   
   static const int MAX_PKT_SIZE = 1500;

   unsigned char replybuf[MAX_PKT_SIZE];

   // reply pkts have 16bit dword count, then #count dwords
   int readmsg(int socket)
   {
      //printf("readmsg enter!\n");
      
      int ret = testmsg(socket, 10000); // wait 10ms for msg
      if (ret <= 0) {
         return -2;
      }

      int flags=0;
      struct sockaddr client_addr;
      socklen_t client_addr_len;
      int bytes = recvfrom(socket, replybuf, MAX_PKT_SIZE, flags, &client_addr, &client_addr_len);

      if (bytes < 0) {
         fprintf(stderr,"readmsg: recvfrom failed, errrno %d (%s)\n", errno, strerror(errno));
         return -1;
      }

      //printf("readmsg return %d\n", bytes);
      return bytes;
   }
   
   int open_udp_socket(int server_port, const char *hostname, struct sockaddr *addr, int *addr_len)
   {
      struct sockaddr_in local_addr;
      struct hostent *hp;
      int sock_fd;
      
      int mxbufsiz = 0x00800000; /* 8 Mbytes ? */
      int sockopt=1; // Re-use the socket
      if( (sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
         fprintf(stderr,"udptest: ERROR opening socket\n");
         return(-1);
      }
      setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int));
      if(setsockopt(sock_fd, SOL_SOCKET,SO_RCVBUF, &mxbufsiz, sizeof(int)) == -1){
         fprintf(stderr,"udptest: setsockopt for buff size\n");
         return(-1);
      }
      memset(&local_addr, 0, sizeof(local_addr));
      local_addr.sin_family = AF_INET;
      local_addr.sin_port = htons(0);          // 0 => use any available port
      local_addr.sin_addr.s_addr = INADDR_ANY; // listen to all local addresses
      if( bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr) )<0) {
         fprintf(stderr,"udptest: ERROR on binding\n");
         return(-1);
      }
      // now fill in structure of server addr/port to send to ...
      *addr_len = sizeof(struct sockaddr);
      bzero((char *) addr, *addr_len);
      if( (hp=gethostbyname(hostname)) == NULL ){
         fprintf(stderr,"udptest: can't get host address for %s\n", hostname);
         return(-1);
      }
      struct sockaddr_in* addrin = (struct sockaddr_in*)addr;
      memcpy(&addrin->sin_addr, hp->h_addr_list[0], hp->h_length);
      addrin->sin_family = AF_INET;
      addrin->sin_port = htons(server_port);
      
      return sock_fd;
   }
   
   void printreply(int bytes)
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
   
   int param_decode(const unsigned char *buf, int *par, int *chan, int *val)
   {
      if( strncmp((const char*)buf, "RDBK", 4) != 0 ){ return(-1); } // wrong header
      if( ! (buf[4] & 0x40) ){ return(-1); } // readbit not set
      *par  = ((buf[4] & 0x3f ) << 8) | buf[5];
      *chan = ((buf[6] & 0xff ) << 8) | buf[7];
      *val  = (buf[8]<<24) | (buf[9]<<16) | (buf[10]<<8) | buf[11];
      return(0);
   }
   
   bool write_param(int par, int chan, int val)
   {
      int bytes;
      char msgbuf[256];
      //printf("Writing %d [0x%08x] into %s[par%d] on chan%d[%2d,%2d,%3d]\n", val, val, parname(par), par, chan, chan>>12, (chan>>8)&0xF, chan&0xFF);
      param_encode(msgbuf, par, WRITE, chan, val);
      bytes = sndmsg(fCmdSocket, &fCmdAddr, fCmdAddrLen, msgbuf, 12);
      if (bytes < 0)
         return false;
      bytes = readmsg(fCmdSocket);
      //printf("   reply      :%d bytes ...", bytes);
      //printreply(bytes);
      return true;
   }
   
   bool read_param(int par, int chan, uint32_t* value)
   {
      int bytes;
      int val = 0;
      //printf("Reading %s[par%d] on chan%d[%2d,%2d,%3d]\n", parname(par), par, chan, chan>>12, (chan>>8)&0xF, chan&0xFF);
      char msgbuf[256];
      param_encode(msgbuf, par, READ, chan, 0);
      int ret = sndmsg(fCmdSocket, &fCmdAddr, fCmdAddrLen, msgbuf, 12);
      if (ret != 12) {
         return false;
      }
      bytes = readmsg(fCmdSocket);
      if (bytes == -2) {
         fprintf(stderr, "readmsg: timeout!\n");
         return false;
      }
      //printf("   reply      :%d bytes ... ", bytes);
      //printreply(bytes);
      bytes = readmsg(fCmdSocket);
      //printf("   read-return:%d bytes ... ", bytes);
      //printreply(bytes);
      if( param_decode(replybuf, &par, &chan, &val) == 0 ){
         //printf("%10s[par%2d] on chan%5d[%2d,%2d,%3d] is %10u [0x%08x]\n",
         //       parname(par), par, chan, chan>>12, (chan>>8)&0xF, chan&0xFF, val, val);
      }
      *value = val;
      return true;
   }
   
   bool ReadCsr(uint32_t* valuep)
   {
      return read_param(63, 0xFFFF, valuep);
   }

   bool WriteCsr(uint32_t value)
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

   uint32_t fCsr = 0;

   bool WriteCsrBits(uint32_t set_bits, uint32_t clr_bits)
   {
      fCsr |= set_bits;
      fCsr &= ~clr_bits;
      return WriteCsr(fCsr);
   }

   bool Init()
   {
      const int DATA_PORT = 8800;
      const int CMD_PORT  = 8808;

      bool ok = true;
      fDataSocket = open_udp_socket(DATA_PORT,fHostname.c_str(),&fDataAddr,&fDataAddrLen);
      fCmdSocket  = open_udp_socket(CMD_PORT, fHostname.c_str(),&fCmdAddr,&fCmdAddrLen);
      return ok;
   }

   bool Identify()
   {
      fFailed = false;

      bool ok = true;

      uint32_t timestamp = 0;

      ok &= read_param(31, 0xFFFF, &timestamp);

      if (!ok)
         return ok;

      time_t ts = (time_t)timestamp;
      const struct tm* tptr = localtime(&ts);
      char tstampbuf[256];
      strftime(tstampbuf, sizeof(tstampbuf), "%d%b%g_%H:%M", tptr);

      mfe->Msg(MINFO, "Identify", "ALPHAT %s firmware timestamp 0x%08x (%s)", fOdbName.c_str(), timestamp, tstampbuf);
      return true;
   }

   bool Configure()
   {
      if (fFailed) {
         printf("Configure %s: failed flag\n", fOdbName.c_str());
         return false;
      }

      bool ok = true;

      fCsr = 0;
      ok &= WriteCsr(fCsr);
      ok &= Stop();

      return ok;
   }

   bool fRunning = false;

   bool Start()
   {
      bool ok = true;
      ok &= WriteCsrBits(0x100, 0);
      fRunning = true;
      return ok;
   }

   bool Stop()
   {
      bool ok = true;
      ok &= WriteCsrBits(0, 0x100);
      fRunning = false;
      return ok;
   }

   bool SoftTrigger()
   {
      printf("AlphaTctrl::SoftTrigger!\n");
      bool ok = true;
      ok &= WriteCsrBits(0x200, 0);
      ok &= WriteCsrBits(0, 0x200);
      return ok;
   }

   void Thread()
   {
      printf("thread for %s started\n", fOdbName.c_str());
      while (!mfe->fShutdown) {
         if (fFailed) {
            std::lock_guard<std::mutex> lock(fLock);
            fFailed = false;
            bool ok = Identify();
            if (!ok) {
               fOk = false;
               sleep(fFailedSleep);
               continue;
            }
         }

         if (fRunning) {
            SoftTrigger();
         }

         sleep(1);
         //sleep(fPollSleep);
      }
      printf("thread for %s shutdown\n", fOdbName.c_str());
   }
};

void start_at_thread(AlphaTctrl* at)
{
   at->Thread();
}

class Ctrl : public TMFeRpcHandlerInterface
{
public:
   TMFE* mfe = NULL;
   TMFeEquipment* eq = NULL;

   AlphaTctrl* fATctrl = NULL;
   std::vector<Alpha16ctrl*> fA16ctrl;

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

   void Init()
   {
      // check that Init() is not called twice
      assert(fATctrl == NULL);

      int countAT = 0;

      for (int i=0; i<1; i++) { // THIS IS NOT A LOOP
         std::string name = OdbGetString(mfe, "/Equipment/Ctrl/Settings/ALPHAT_MODULES", 0);

         if (name[0] == '#') {
            continue;
         }

         AlphaTctrl* at = new AlphaTctrl();
         at->fHostname = name;
         at->fOdbName = name;
         at->mfe = mfe;
         at->eq = eq;

         bool ok = at->Init();
         if (!ok) {
            mfe->Msg(MERROR, "Init", "ALPHAT %s init failed.", name.c_str());
            delete at;
            continue;
         }

         ok = at->Identify();
         if (!ok) {
            mfe->Msg(MERROR, "Init", "ALPHAT %s cannot be used.", name.c_str());
            delete at;
            continue;
         }

         at->fOk = true;
         
         fATctrl = at;
         countAT++;
      }

      printf("Init: ALPHAT_MODULES: %d\n", countAT);

      // check that Init() is not called twice
      assert(fA16ctrl.size() == 0);

      int num = odbReadArraySize(mfe, "/Equipment/Ctrl/Settings/ALPHA16_MODULES");

      if (num == 0) {
         mfe->Msg(MERROR, "Init", "Please create string array Settings/ALPHA16_MODULES");
         exit(1);
      }

      printf("Init: ALPHA16_MODULES: %d\n", num);

      for (int i=0; i<num; i++) {
         fA16ctrl.push_back(NULL);
      }

      int countA16 = 0;

      for (int i=0; i<num; i++) {
         std::string name = OdbGetString(mfe, "/Equipment/Ctrl/Settings/ALPHA16_MODULES", i);

         //printf("index %d name [%s]\n", i, name.c_str());

         if (name.length() == 0)
            continue;
         if (name[0] == '#')
            continue;

         KOtcpConnection* s = new KOtcpConnection(name.c_str(), "http");

         s->fConnectTimeoutMilliSec = 2*1000;
         s->fReadTimeoutMilliSec = 2*1000;
         s->fWriteTimeoutMilliSec = 2*1000;
         s->fHttpKeepOpen = false;

         class Alpha16ctrl* a16 = new Alpha16ctrl;

         a16->mfe = mfe;
         a16->eq = eq;
         a16->s = s;
         a16->fOdbName = name;
         a16->fOdbIndex = i;
         
         bool ok = a16->Identify();
         if (!ok) {
            mfe->Msg(MERROR, "Init", "ALPHA16 %s cannot be used.", name.c_str());
            delete a16;
            continue;
         }

         a16->fOk = true;
         
         fA16ctrl[i] = a16;
         countA16++;
      }

      int countFeam = 0;

      mfe->Msg(MINFO, "Init", "Will use %d ALPHAT, %d ALPHA16, %d FEAM modules", countAT, countA16, countFeam);

      char buf[256];
      sprintf(buf, "Init: %d AT, %d A16, %d FEAM", countAT, countA16, countFeam);
      eq->SetStatus(buf, "#00FF00");
   }

   bool Identify()
   {
      bool ok = true;

      if (fATctrl) {
         ok &= fATctrl->Identify();
      }

      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i]) {
            ok &= fA16ctrl[i]->Identify();
         }
      }

      return ok;
   }

   void Configure()
   {
      bool at_ok = false;
      int countOk = 0;
      int countBad = 0;

      fNumBanks = 0;

      if (fATctrl) {
         at_ok = fATctrl->Configure();
      }

      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i]) {
            bool ok = fA16ctrl[i]->Configure();
            if (ok) {
               fNumBanks += fA16ctrl[i]->fNumBanks;
               countOk += 1;
            }
            if (!ok)
               countBad += 1;
         }
      }
      
      char buf[256];
      if (countBad == 0) {
         sprintf(buf, "Configure: %d AT, %d A16 Ok, %d banks", at_ok, countOk, fNumBanks);
         eq->SetStatus(buf, "#00FF00");
      } else {
         sprintf(buf, "Configure: %d AT, %d A16 Ok, %d bad, %d banks", at_ok, countOk, countBad, fNumBanks);
         eq->SetStatus(buf, "yellow");
      }

      {
         std::vector<int> num_banks;
         num_banks.push_back(fNumBanks);
         WVI("num_banks", num_banks);
      }
   }

   bool Start()
   {
      bool ok = true;

      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i]) {
            ok &= fA16ctrl[i]->Start();
         }
      }

      if (fATctrl) {
         ok &= fATctrl->Start();
      }

      mfe->Msg(MINFO, "Start", "Start ok %d", ok);
      return ok;
   }

   bool Stop()
   {
      bool ok = true;

      if (fATctrl) {
         ok &= fATctrl->Stop();
      }

      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i]) {
            ok &= fA16ctrl[i]->Stop();
         }
      }

      mfe->Msg(MINFO, "Stop", "Stop ok %d", ok);
      return ok;
   }

   bool SoftTrigger()
   {
      bool ok = true;

      if (fATctrl) {
         ok &= fATctrl->SoftTrigger();
      } else {
         for (unsigned i=0; i<fA16ctrl.size(); i++) {
            if (fA16ctrl[i]) {
               ok &= fA16ctrl[i]->SoftTrigger();
            }
         }
      }

      return ok;
   }

   void ReadAndCheck()
   {
      int countOk = 0;
      int countBad = 0;
      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i]) {
            if (fA16ctrl[i]->fFailed) {
               fA16ctrl[i]->Identify();
               countBad += 1;
               continue;
            }
            EsperNodeData e;
            bool ok = fA16ctrl[i]->ReadAll(&e);
            if (ok) {
               ok = fA16ctrl[i]->Check(e);
               if (ok)
                  countOk += 1;
            }
            if (!ok)
               countBad += 1;
         }
      }

      {
         char buf[256];
         if (countBad == 0) {
            sprintf(buf, "%d A16 Ok, %d banks", countOk, fNumBanks);
            eq->SetStatus(buf, "#00FF00");
         } else {
            sprintf(buf, "%d A16 Ok, %d bad, %d banks", countOk, countBad, fNumBanks);
            eq->SetStatus(buf, "yellow");
         }
      }
   }

   void ThreadReadAndCheck()
   {
      int count_at = 0;
      int countOk = 0;
      int countBad = 0;

      if (fATctrl) {
         if (fATctrl->fOk) {
            count_at += 1;
         }
      }

      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i]) {
            bool ok = fA16ctrl[i]->fOk;
            if (ok)
               countOk += 1;
            else
               countBad += 1;
         }
      }

      {
         LOCK_ODB();
         char buf[256];
         if (countBad == 0) {
            sprintf(buf, "%d AT, %d A16 Ok, %d banks", count_at, countOk, fNumBanks);
            eq->SetStatus(buf, "#00FF00");
         } else {
            sprintf(buf, "%d AT, %d A16 Ok, %d bad, %d banks", count_at, countOk, countBad, fNumBanks);
            eq->SetStatus(buf, "yellow");
         }
      }
   }

   void WriteVariables()
   {
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

      WVD("fpga_temp", fpga_temp);
      WVD("sensor_temp0", sensor_temp0);
      WVD("sensor_temp_max", sensor_temp_max);
      WVD("sensor_temp_min", sensor_temp_min);
   }

   void Periodic()
   {
      ReadAndCheck();
      WriteVariables();
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
      printf("LockAll...done\n");
   }

   void UnlockAll()
   {
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

   void HandleBeginRun()
   {
      printf("BeginRun!\n");
      LockAll();
      Identify();
      Configure();
      ReadAndCheck();
      WriteVariables();
      Start();
      SoftTrigger();
      UnlockAll();
   }

   void HandleEndRun()
   {
      printf("EndRun!\n");
      LockAll();
      Stop();
      UnlockAll();
   }

   void StartThreads()
   {
      // ensure threads are only started once
      static bool gOnce = true;
      assert(gOnce == true);
      gOnce = false;

      if (fATctrl) {
         std::thread * t = new std::thread(start_at_thread, fATctrl);
         t->detach();
      }

      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i]) {
            std::thread * t = new std::thread(start_a16_thread, fA16ctrl[i]);
            t->detach();
         }
      }
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

   Ctrl* ctrl = new Ctrl;

   ctrl->mfe = mfe;
   ctrl->eq = eq;

   mfe->RegisterRpcHandler(ctrl);
   mfe->SetTransitionSequence(890, 100, -1, -1);

   ctrl->Init();
   // already done inside Init(), ctrl->Identify();
   ctrl->Configure();

   {
      int run_state = OdbGetInt(mfe, "/Runinfo/State", 0, false);
      bool running = (run_state == 3);
      if (running)
         ctrl->Start();
   }

   ctrl->Periodic();

   ctrl->SoftTrigger();

   ctrl->StartThreads();

   printf("init done!\n");

   while (!mfe->fShutdown) {

      //ctrl->Periodic();
      ctrl->ThreadPeriodic();

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
