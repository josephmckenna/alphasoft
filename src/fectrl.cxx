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

#if 0
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

std::string OdbGetString(TMFE* mfe, const char* path, int index)
{
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
         eq->SetStatus("http error", "red");
         mfe->Msg(MERROR, "Write", "HttpGet() error %s", e.message.c_str());
         return false;
      }

      if (reply_body.find("error") != std::string::npos) {
         eq->SetStatus("http error", "red");
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

   std::string Read(const char* mid, const char* vid)
   {
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
         eq->SetStatus("http error", "red");
         mfe->Msg(MERROR, "Read", "HttpGet() error %s", e.message.c_str());
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

   bool Identify()
   {
      std::string elf_buildtime = Read("board", "elf_buildtime");
      std::string sw_qsys_ts = Read("board", "sw_qsys_ts");
      std::string hw_qsys_ts = Read("board", "hw_qsys_ts");

      mfe->Msg(MINFO, "Identify", "ALPHA16 %s firmware 0x%08x-0x%08x-0x%08x", fOdbName.c_str(), xatoi(elf_buildtime.c_str()), xatoi(sw_qsys_ts.c_str()), xatoi(hw_qsys_ts.c_str()));

      bool ok = true;

      ok &= elf_buildtime.length() > 0;
      ok &= sw_qsys_ts.length() > 0;
      ok &= hw_qsys_ts.length() > 0;

      return ok;
   }

   bool Configure()
   {
      int udp_port = OdbGetInt(mfe, "/Equipment/UDP/Settings/udp_port");
      int adc16_samples = OdbGetInt(mfe, "/Equipment/CTRL/Settings/adc16_samples");

      printf("Configure %s: udp_port %d, adc16 samples %d\n", fOdbName.c_str(), udp_port, adc16_samples);

      bool ok = true;

      ok &= Stop();

      // make sure everything is stopped

      ok &= Write("board", "force_run", "false");
      ok &= Write("udp", "enable", "false");

      // switch clock to ESATA

      ok &= Write("board", "clk_lmk", "1");

      // configure the ADCs

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

      {
         std::string json;
         json += "[";
         for (int i=0; i<32; i++) {
            json += "false";
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
};

class Ctrl : public TMFeRpcHandlerInterface
{
public:
   TMFE* mfe = NULL;
   TMFeEquipment* eq = NULL;

   std::vector<Alpha16ctrl*> fA16ctrl;

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
   };

   void Init()
   {
      int num = odbReadArraySize(mfe, "/Equipment/Ctrl/Settings/ALPHA16_MODULES");

      if (num == 0) {
         mfe->Msg(MERROR, "Init", "Please create string array Settings/ALPHA16_MODULES");
         exit(1);
      }

      printf("Init: ALPHA16_MODULES: %d\n", num);

      for (int i=0; i<num; i++) {
         fA16ctrl.push_back(NULL);
      }

      for (int i=0; i<num; i++) {
         std::string name = OdbGetString(mfe, "/Equipment/Ctrl/Settings/ALPHA16_MODULES", i);

         //printf("index %d name [%s]\n", i, name.c_str());

         if (name.length() == 0)
            continue;
         if (name[0] == '#')
            continue;

         KOtcpConnection* s = new KOtcpConnection(name.c_str(), "http");

         s->fReadTimeout = 5*1000;
         s->fWriteTimeout = 5*1000;
         s->fHttpKeepOpen = false;

         class Alpha16ctrl* a16 = new Alpha16ctrl;

         a16->mfe = mfe;
         a16->eq = eq;
         a16->s = s;
         a16->fOdbName = name;
         
         bool ok = a16->Identify();
         if (!ok) {
            mfe->Msg(MERROR, "Init", "ALPHA16 %s cannot be used.", name.c_str());
            delete a16;
            continue;
         }
         
         fA16ctrl[i] = a16;
      }

      mfe->Msg(MINFO, "Init", "Will use %d ALPHA16 modules", (int)fA16ctrl.size());

      char buf[256];
      sprintf(buf, "Init: %d A16", (int)fA16ctrl.size());
      eq->SetStatus(buf, "#00FF00");
   }

   bool Identify()
   {
      bool ok = true;
      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i]) {
            ok &= fA16ctrl[i]->Identify();
         }
      }
      return ok;
   }

   void Configure()
   {
      int countOk = 0;
      int countBad = 0;
      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i]) {
            bool ok = fA16ctrl[i]->Configure();
            if (ok) {
               countOk += 1;
            }
            if (!ok)
               countBad += 1;
         }
      }
      
      char buf[256];
      if (countBad == 0) {
         sprintf(buf, "Configure: %d A16 Ok", countOk);
         eq->SetStatus(buf, "#00FF00");
      } else {
         sprintf(buf, "Configure: %d A16 Ok, %d bad", countOk, countBad);
         eq->SetStatus(buf, "yellow");
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
      mfe->Msg(MINFO, "Start", "Start ok %d", ok);
      return ok;
   }

   bool Stop()
   {
      bool ok = true;
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
      for (unsigned i=0; i<fA16ctrl.size(); i++) {
         if (fA16ctrl[i]) {
            ok &= fA16ctrl[i]->SoftTrigger();
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
            sprintf(buf, "%d A16 Ok", countOk);
            eq->SetStatus(buf, "#00FF00");
         } else {
            sprintf(buf, "%d A16 Ok, %d bad", countOk, countBad);
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

   std::string HandleRpc(const char* cmd, const char* args)
   {
      mfe->Msg(MINFO, "HandleRpc", "RPC cmd [%s], args [%s]", cmd, args);
      return "OK";
   }

   void HandleBeginRun()
   {
      printf("BeginRun!\n");
      Identify();
      Configure();
      ReadAndCheck();
      WriteVariables();
      Start();
      SoftTrigger();
   }

   void HandleEndRun()
   {
      printf("EndRun!\n");
      Stop();
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

   mfe->SetTransitionSequence(900, 100, 0, 0);

   ctrl->Init();
   // already done inside Init(), ctrl->Identify();
   ctrl->Configure();

   {
      int run_state = OdbGetInt(mfe, "/Runinfo/State");
      bool running = (run_state == 3);
      if (running)
         ctrl->Start();
   }

   ctrl->Periodic();

   ctrl->SoftTrigger();

   printf("init done!\n");

   while (!mfe->fShutdown) {

      ctrl->Periodic();

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
