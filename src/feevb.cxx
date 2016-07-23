//
// feevb.cxx
//
// Frontend/event builder for GRIF16 data
//

#include <stdio.h>
#include <netdb.h> // getnameinfo()
#include <stdlib.h> // malloc()
#include <string.h> // memcpy()
#include <errno.h> // errno
//#include <unistd.h>
//#include <time.h>

#include <string>
#include <vector>
#include <deque>
#include <mutex>

#include "midas.h"

const char *frontend_name = "feevb";                     /* fe MIDAS client name */
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
#define EQ_NAME "EVB"
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

static int verbose = 0;
static HNDLE hDB;
static HNDLE hKeySet; // equipment settings

static int gEventReadCount = 0;

struct BankBuf
{
   std::string name;
   int tid;
   char* ptr;
   int psize;

   BankBuf(const char* bankname, int xtid, const char* s, int size) // ctor
   {
      name = bankname;
      tid = xtid;
      ptr = (char*)malloc(size);
      psize = size;
      memcpy(ptr, s, size);
   }

   ~BankBuf() // dtor
   {
      if (ptr)
         free(ptr);
      ptr = NULL;
      psize = 0;
   }
};

typedef std::vector<BankBuf*> FragmentBuf;

std::deque<FragmentBuf*> gBuf;
std::mutex       gBufLock;

// NOTE: event hander runs from the main thread!

void event_handler(HNDLE hBuf, HNDLE id, EVENT_HEADER *pheader, void *pevent)
{
   if (verbose)
      printf("event_handler: Evid: 0x%x, Mask: 0x%x, Serial: %d, Size: %d\n", pheader->event_id, pheader->trigger_mask, pheader->serial_number, pheader->data_size);

   gEventReadCount++;

   int nbytes = pheader->data_size;

   int nbanks;
   char banklist[STRING_BANKLIST_MAX];

   // Display # of banks and list of banks in the event
   nbanks = bk_list(pevent, banklist);
   printf("#banks:%d List:%s\n", nbanks, banklist);

   if (nbanks < 1)
      return;
   
   FragmentBuf* buf = new FragmentBuf();

   for (int i=0; i<nbanks; i++) {
      int status;
      DWORD bklen, bktype;
      void* pbank;
      
      status = bk_find((BANK_HEADER*)pevent, &banklist[i*4], &bklen, &bktype, &pbank);

      printf("bk_find status %d, name [%s], bklen %d, bktype %d\n", status, &banklist[i*4], bklen, bktype);

      BankBuf *bank = new BankBuf(&banklist[i*4], bktype, (char*)pbank, bklen);
      buf->push_back(bank);
   }

   std::lock_guard<std::mutex> lock(gBufLock);
   gBuf.push_back(buf);
}

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

   int evid = -1;
   int trigmask = 0xFFFF;

   int bh = 0;
   const char *bufname = "BUFUDP";

   status = bm_open_buffer(bufname, 0, &bh);
   if (status != BM_SUCCESS && status != BM_CREATED)
      {
         cm_msg(MERROR, "frontend_init", "Error: bm_open_buffer(\"%s\") status %d", bufname, status);
         return FE_ERR_HW;
      }

   int reqid = 0;
   status = bm_request_event(bh, evid, trigmask, GET_ALL, &reqid, event_handler);
   if (status != BM_SUCCESS)
      {
         cm_msg(MERROR, "frontend_init", "Error: bm_request_event() status %d", status);
         return FE_ERR_HW;
      }

   cm_msg(MINFO, "frontend_init", "Event builder started, buffer \"%s\", evid %d, trigmask 0x%x, verbose %d", bufname, evid, trigmask, verbose);

   return SUCCESS;
}

int frontend_loop()
{
   ss_sleep(10);
   return SUCCESS;
}

int begin_of_run(int run_number, char *error)
{
   printf("begin_of_run!\n");
   return SUCCESS;
}

int end_of_run(int run_number, char *error)
{
   printf("end_of_run!\n");
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
   if (gBuf.size() < 1)
      return 0;

   printf("in queue: %d\n", (int)gBuf.size());

   FragmentBuf* f = NULL;

   {
      std::lock_guard<std::mutex> lock(gBufLock);

      if (gBuf.size() < 1)
         return 0;
      
      f = gBuf.front();
      gBuf.pop_front();

      // implicit unlock of gBufLock
   }

   if (!f)
      return 0;
   
   bk_init32(pevent);

   for (unsigned i=0; i<f->size(); i++) {
      BankBuf* b = (*f)[i];

      char* pdata;
      bk_create(pevent, b->name.c_str(), b->tid, (void**)&pdata);
      memcpy(pdata, b->ptr, b->psize);
      bk_close(pevent, pdata + b->psize);

      delete b;
   }

   delete f;

   return bk_size(pevent); 
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
