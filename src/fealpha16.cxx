//
// fealpha16.cxx
//
// Frontend for configuration and monitoring of GRIF16/ALPHA16 boards.
//

#include <stdio.h>
#include <netdb.h> // getnameinfo()
//#include <stdlib.h>
#include <string.h> // memcpy()
#include <errno.h> // errno
//#include <unistd.h>
//#include <time.h>

#include <string>
#include <vector>

#include "midas.h"

const char *frontend_name = "fealpha16";                 /* fe MIDAS client name */
const char *frontend_file_name = __FILE__;               /* The frontend file name */

extern "C" {
   BOOL frontend_call_loop = TRUE;       /* frontend_loop called periodically TRUE */
   int display_period = 0;               /* status page displayed with this freq[ms] */
   int max_event_size = 1*1024*1024;     /* max event size produced by this frontend */
   int max_event_size_frag = 5 * 1024 * 1024;     /* max for fragmented events */
   int event_buffer_size = 8*1024*1024;           /* buffer size to hold events */
}

extern "C" {
  int interrupt_configure(INT cmd, INT source, PTYPE adr);
  INT poll_event(INT source, INT count, BOOL test);
  int frontend_init();
  int frontend_exit();
  int begin_of_run(int run, char *err);
  int end_of_run(int run, char *err);
  int pause_run(int run, char *err);
  int resume_run(int run, char *err);
  int frontend_loop();
  int read_event(char *pevent, INT off);
}

#ifndef EQ_NAME
#define EQ_NAME "ALPHA16"
#endif

#ifndef EQ_EVID
#define EQ_EVID 1
#endif

EQUIPMENT equipment[] = {
   { EQ_NAME,                         /* equipment name */
      {EQ_EVID, 0, "SYSTEM",          /* event ID, trigger mask, Evbuf */
       EQ_MULTITHREAD, 0, "MIDAS",    /* equipment type, EventSource, format */
       TRUE, RO_ALWAYS,               /* enabled?, WhenRead? */
       50, 0, 0, 0,                   /* poll[ms], Evt Lim, SubEvtLim, LogHist */
       "", "", "",}, read_event,      /* readout routine */
   },
   {""}
};
////////////////////////////////////////////////////////////////////////////

static HNDLE hDB;
static HNDLE hKeySet; // equipment settings

#include "mscbcxx.h"


void alpha16_info(MscbSubmaster* s)
{
   int size;
   char buf[256];

   printf("ALPHA16 MSCB submaster %s\n", s->GetName().c_str());

   s->Read(0, 0, buf, sizeof(buf), &size);
   printf("    MAC address: %s\n", buf);

   s->Read(1, 2, buf, sizeof(buf), &size);
   printf("    sp_frun: %d\n", buf[0]);

   int temperature;
   s->Read(1, 5, &temperature, 4, &size);
   printf("    cpu_temp: %d\n", temperature);

   int f_esata;
   s->Read(1, 47, &f_esata, 4, &size);
   printf("    f_esata:  %d\n", f_esata);

   int f_adc;
   s->Read(1, 49, &f_adc, 4, &size);
   printf("    f_adc:    %d\n", f_adc);

   s->Read(1, 109, buf, sizeof(buf), &size);
   printf("    UDP Dest IP: %s\n", buf);

   s->Read(1, 111, buf, sizeof(buf), &size);
   printf("    UDP Dest MAC: %s\n", buf);

   uint16_t dst_prt;
   s->Read(1, 113, &dst_prt, 2, &size);
   printf("    UDP Dest port:  %d\n", dst_prt);

   int udp_on;
   s->Read(1, 115, &udp_on, 4, &size);
   printf("    UDP on:  %d\n", udp_on);

   int udp_cnt;
   s->Read(1, 118, &udp_cnt, 4, &size);
   printf("    UDP count:  %d\n", udp_cnt);

   for (int i=0; i<16; i++) {
      printf("  ADC%02d: ", i);

      int a_fgain;
      int t_on;
      int t_tdly;
      int w_spnt;

      s->Read(2+i, 0, &a_fgain, 4, &size);
      s->Read(2+i, 3, &t_on, 4, &size);
      s->Read(2+i, 4, &t_tdly, 4, &size);
      s->Read(2+i, 6, &w_spnt, 4, &size);
      printf(" %d %d %d %d", a_fgain, t_on, t_tdly, w_spnt);

      printf("\n");
   }
}

typedef std::vector<MscbSubmaster*> MscbDevices;

MscbDriver *gMscb = NULL;
MscbDevices gAlpha16list;

int interrupt_configure(INT cmd, INT source, PTYPE adr)
{
   return SUCCESS;
}

int frontend_init()
{
   int status;

   status = cm_get_experiment_database(&hDB, NULL);
   if (status != CM_SUCCESS) {
      cm_msg(MERROR, "frontend_init", "Cannot connect to ODB, cm_get_experiment_database() returned %d", status);
      return FE_ERR_ODB;
   }

   std::string path;
   path += "/Equipment";
   path += "/";
   path += EQ_NAME;
   path += "/Settings";

   gMscb = new MscbDriver();
   status = gMscb->Init();
   printf("MSCB::Init: status %d\n", status);

   MscbSubmaster* adc03 = gMscb->GetEthernetSubmaster("192.168.1.103");
   MscbSubmaster* adc04 = gMscb->GetEthernetSubmaster("192.168.1.104");

   status = adc03->Init();
   printf("MSCB::adc03::Init: status %d\n", status);

   status = adc03->ScanPrint(0, 100);
   printf("MSCB::adc03::ScanPrint: status %d\n", status);

   status = adc04->Init();
   printf("MSCB::adc04::Init: status %d\n", status);

   status = adc04->ScanPrint(0, 100);
   printf("MSCB::adc04::ScanPrint: status %d\n", status);

   gAlpha16list.push_back(adc03);
   gAlpha16list.push_back(adc04);

   for (unsigned i=0; i<gAlpha16list.size(); i++)
      alpha16_info(gAlpha16list[i]);

#if 0
   std::string path1 = path + "/udp_port";

   int udp_port = 50005;
   int size = sizeof(udp_port);
   status = db_get_value(hDB, 0, path1.c_str(), &udp_port, &size, TID_INT, TRUE);
   
   if (status != DB_SUCCESS) {
      cm_msg(MERROR, "frontend_init", "Cannot find \"%s\", db_get_value() returned %d", path1.c_str(), status);
      return FE_ERR_ODB;
   }
   
   status = db_find_key(hDB, 0, path.c_str(), &hKeySet);
   
   if (status != DB_SUCCESS) {
      cm_msg(MERROR, "frontend_init", "Cannot find \"%s\", db_find_key() returned %d", path.c_str(), status);
      return FE_ERR_ODB;
   }
   
   gDataSocket = open_udp_socket(udp_port);
   
   if (gDataSocket < 0) {
      printf("frontend_init: cannot open udp socket\n");
      cm_msg(MERROR, "frontend_init", "Cannot open UDP socket for port %d", udp_port);
      return FE_ERR_HW;
   }

   cm_msg(MINFO, "frontend_init", "Frontend equipment \"%s\" is ready, listening on UDP port %d", EQ_NAME, udp_port);
#endif
   return SUCCESS;
}

int frontend_loop()
{
   ss_sleep(10);
   return SUCCESS;
}

int begin_of_run(int run_number, char *error)
{
   return SUCCESS;
}

int end_of_run(int run_number, char *error)
{
   return SUCCESS;
}

int pause_run(INT run_number, char *error)
{
   return SUCCESS;
}

int resume_run(INT run_number, char *error)
{
   return SUCCESS;
}

int frontend_exit()
{
   return SUCCESS;
}

INT poll_event(INT source, INT count, BOOL test)
{
   //printf("poll_event: source %d, count %d, test %d\n", source, count, test);

   if (test) {
      for (int i=0; i<count; i++)
         ss_sleep(10);
      return 1;
   }

   return 1;
}

int read_event(char *pevent, int off)
{
   return 0;
#if 0
   bk_init32(pevent);
   char* pdata;
   bk_create(pevent, bankname, TID_BYTE, (void**)&pdata);
   memcpy(pdata, buf, length);
   bk_close(pevent, pdata + length);
   return bk_size(pevent); 
#endif
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
