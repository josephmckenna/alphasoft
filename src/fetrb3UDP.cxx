// 
// Modified version of feudp.cxx from Konstantin; repurposed to setup and readout UDP socket from TRB3 FGPA-TDC
// 
// Also periodically reads TRB3 temperatures.



#include <stdio.h>
#include <netdb.h> // getnameinfo()
#include <string.h> // memcpy()
#include <errno.h> // errno
#include <string>
#include <vector>
#include <math.h> // fabs()

#include "midas.h"
#include "mfe.h"

// Header for TRB3 control
#include <trbnet.h>
#include <trberror.h>
// buffer for TRB3 commands
static size_t BUFFER_SIZE = 4194304;  /* 4MByte */
static uint32_t buffer[4194304];    
 

const char *frontend_name = "fetdc";                     /* fe MIDAS client name */
const char *frontend_file_name = __FILE__;               /* The frontend file name */

   BOOL frontend_call_loop = TRUE;       /* frontend_loop called periodically TRUE */
   int display_period = 0;               /* status page displayed with this freq[ms] */
   int max_event_size = 1*1024*1024;     /* max event size produced by this frontend */
   int max_event_size_frag = 5 * 1024 * 1024;     /* max for fragmented events */
   int event_buffer_size = 8*1024*1024;           /* buffer size to hold events */

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
  INT read_trb3_temperature(char *pevent, INT off);

#ifndef EQ_NAME
#define EQ_NAME "TDC"
#endif

#ifndef EQ_EVID
#define EQ_EVID 1
#endif

#define EQ_NAME_TEMP "TDC_TEMP"

#define NUM_FPGA 4

EQUIPMENT equipment[] = {
   { EQ_NAME,                         /* equipment name */
      {EQ_EVID, 0, "SYSTEM",          /* event ID, trigger mask, Evbuf */
       EQ_MULTITHREAD, 0, "MIDAS",    /* equipment type, EventSource, format */
       TRUE, RO_ALWAYS,               /* enabled?, WhenRead? */
       50, 0, 0, 0,                   /* poll[ms], Evt Lim, SubEvtLim, LogHist */
       "", "", "",}, read_event,      /* readout routine */
   },
   {EQ_NAME_TEMP,            /* equipment name */
    {2, 0,                   /* event ID, trigger mask */
     "SYSTEM",               /* event buffer */
     EQ_PERIODIC,              /* equipment type */
     LAM_SOURCE(0, 0xFFFFFF),        /* event source crate 0, all stations */
     "MIDAS",                /* format */
     TRUE,                   /* enabled */
     RO_ODB|RO_ALWAYS,           /* read only when running */
     2000,                    /* poll for 1000ms */
     0,                      /* stop run after this event limit */
     0,                      /* number of sub events */
     1,                      
     "", "", "",},
    read_trb3_temperature,      /* readout routine */
    },
   {""}
};
////////////////////////////////////////////////////////////////////////////

#include <sys/time.h>

static double GetTimeSec()
{
   struct timeval tv;
   gettimeofday(&tv,NULL);
   return tv.tv_sec + 0.000001*tv.tv_usec;
}

struct Source
{
  struct sockaddr addr;
  char bank_name[5];
  std::string host_name;
};

static std::vector<Source> gSrc;

//static HNDLE hDB;
static HNDLE hKeySet; // equipment settings

static int gDataSocket;

static int gUnknownPacketCount = 0;
static bool gSkipUnknownPackets = false;

static int gLastEventNumber = -1;
static int gNumberNonseqEventNumbers = 0;
static bool gTemperatureReadFailed = false;

int open_udp_socket(int server_port)
{
   int status;
   
   int fd = socket(AF_INET, SOCK_DGRAM, 0);
   
   if (fd < 0) {
      cm_msg(MERROR, "open_udp_socket", "socket(AF_INET,SOCK_DGRAM) returned %d, errno %d (%s)", fd, errno, strerror(errno));
      return -1;
   }

   int opt = 1;
   status = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

   if (status == -1) {
      cm_msg(MERROR, "open_udp_socket", "setsockopt(SOL_SOCKET,SO_REUSEADDR) returned %d, errno %d (%s)", status, errno, strerror(errno));
      return -1;
   }

   int bufsize = 8*1024*1024;
   //int bufsize = 20*1024;

   status = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));

   if (status == -1) {
      cm_msg(MERROR, "open_udp_socket", "setsockopt(SOL_SOCKET,SO_RCVBUF) returned %d, errno %d (%s)", status, errno, strerror(errno));
      return -1;
   }

   struct sockaddr_in local_addr;
   memset(&local_addr, 0, sizeof(local_addr));

   local_addr.sin_family = AF_INET;
   local_addr.sin_port = htons(server_port);
   local_addr.sin_addr.s_addr = INADDR_ANY;

   status = bind(fd, (struct sockaddr *)&local_addr, sizeof(local_addr));

   if (status == -1) {
      cm_msg(MERROR, "open_udp_socket", "bind(port=%d) returned %d, errno %d (%s)", server_port, status, errno, strerror(errno));
      return -1;
   }

   int xbufsize = 0;
   unsigned size = sizeof(xbufsize);

   status = getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &xbufsize, &size);

   //printf("status %d, xbufsize %d, size %d\n", status, xbufsize, size);

   if (status == -1) {
      cm_msg(MERROR, "open_udp_socket", "getsockopt(SOL_SOCKET,SO_RCVBUF) returned %d, errno %d (%s)", status, errno, strerror(errno));
      return -1;
   }

   cm_msg(MINFO, "open_udp_socket", "UDP port %d socket receive buffer size is %d", server_port, xbufsize);

   return fd;
}

bool addr_match(const Source* s, void *addr, int addr_len)
{
  int v = memcmp(&s->addr, addr, addr_len);
#if 0
  for (int i=0; i<addr_len; i++)
    printf("%3d - 0x%02x 0x%02x\n", i, ((char*)&s->addr)[i], ((char*)addr)[i]);
  printf("match %d, hostname [%s] bank [%s], status %d\n", addr_len, s->host_name.c_str(), s->bank_name, v);
#endif
  return v==0;
}

int wait_udp(int socket, int msec)
{
   int status;
   fd_set fdset;
   struct timeval timeout;

   FD_ZERO(&fdset);
   FD_SET(socket, &fdset);

   timeout.tv_sec = msec/1000;
   timeout.tv_usec = (msec%1000)*1000;

   status = select(socket+1, &fdset, NULL, NULL, &timeout);

#ifdef EINTR
   if (status < 0 && errno == EINTR) {
      return 0; // watchdog interrupt, try again
   }
#endif

   if (status < 0) {
      cm_msg(MERROR, "wait_udp", "select() returned %d, errno %d (%s)", status, errno, strerror(errno));
      return -1;
   }

   if (status == 0) {
      return 0; // timeout
   }

   if (FD_ISSET(socket, &fdset)) {
      return 1; // have data
   }

   // timeout
   return 0;
}

int find_source(Source* src, const sockaddr* paddr, int addr_len)
{
   char host[NI_MAXHOST], service[NI_MAXSERV];
      
   int status = getnameinfo(paddr, addr_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
      
   if (status != 0) {
      cm_msg(MERROR, "read_udp", "getnameinfo() returned %d (%s), errno %d (%s)", status, gai_strerror(status), errno, strerror(errno));
      return -1;
   }

   char bankname[NAME_LENGTH];
   int size = sizeof(bankname);
      
   status = db_get_value(hDB, hKeySet, host, bankname, &size, TID_STRING, FALSE);
   
   if (status == DB_NO_KEY) {
      cm_msg(MERROR, "read_udp", "UDP packet from unknown host \"%s\"", host);
      cm_msg(MINFO, "read_udp", "Register this host by running following commands:");
      cm_msg(MINFO, "read_udp", "odbedit -c \"create STRING /Equipment/%s/Settings/%s\"", EQ_NAME, host);
      cm_msg(MINFO, "read_udp", "odbedit -c \"set /Equipment/%s/Settings/%s AAAA\", where AAAA is the MIDAS bank name for this host", EQ_NAME, host);
      return -1;
   } else if (status != DB_SUCCESS) {
      cm_msg(MERROR, "read_udp", "db_get_value(\"/Equipment/%s/Settings/%s\") status %d", EQ_NAME, host, status);
      return -1;
   }

   if (strlen(bankname) != 4) {
      cm_msg(MERROR, "read_udp", "ODB \"/Equipment/%s/Settings/%s\" should be set to a 4 character MIDAS bank name", EQ_NAME, host);
      cm_msg(MINFO, "read_udp", "Use this command:");
      cm_msg(MINFO, "read_udp", "odbedit -c \"set /Equipment/%s/Settings/%s AAAA\", where AAAA is the MIDAS bank name for this host", EQ_NAME, host);
      return -1;
   }
      
   cm_msg(MINFO, "read_udp", "UDP packets from host \"%s\" will be stored in bank \"%s\"", host, bankname);
      
   src->host_name = host;
   strlcpy(src->bank_name, bankname, 5);
   memcpy(&src->addr, paddr, sizeof(src->addr));
   
   return 0;
}

int read_udp(int socket, char* buf, int bufsize, char* pbankname)
{
   if (wait_udp(socket, 100) < 1)
      return 0;

#if 0
   static int count = 0;
   static double tt = 0;
   double t = GetTimeSec();

   double dt = (t-tt)*1e6;
   count++;
   if (dt > 1000) {
      printf("read_udp: %5d %6.0f usec\n", count, dt);
      count = 0;
   }
   tt = t;
#endif

   struct sockaddr addr;
   socklen_t addr_len = sizeof(addr);
   int rd = recvfrom(socket, buf, bufsize, 0, &addr, &addr_len);
   
   if (rd < 0) {
      cm_msg(MERROR, "read_udp", "recvfrom() returned %d, errno %d (%s)", rd, errno, strerror(errno));
      return -1;
   }

   for (unsigned i=0; i<gSrc.size(); i++) {
      if (addr_match(&gSrc[i], &addr, addr_len)) {
         strlcpy(pbankname, gSrc[i].bank_name, 5);
         //printf("rd %d, bank [%s]\n", rd, pbankname);
         return rd;
      }
   }

   if (gSkipUnknownPackets)
      return -1;

   Source sss;

   int status = find_source(&sss, &addr, addr_len);

   if (status < 0) {

      gUnknownPacketCount++;

      if (gUnknownPacketCount > 10) {
         gSkipUnknownPackets = true;
         cm_msg(MERROR, "read_udp", "further messages are now suppressed...");
         return -1;
      }

      return -1;
   }

   gSrc.push_back(sss);
         
   strlcpy(pbankname, sss.bank_name, 5);
         
   return rd;
}

int interrupt_configure(INT cmd, INT source, PTYPE adr)
{
   return SUCCESS;
}


// This is the list of FPGA IDs for our TRB3
int fpga_ids[NUM_FPGA] = {0x100,0x101,0x102,0x103};



// enable/disable block of channels. setclr = true means enable the channel. 
int setEnableBlock(int board, int channel, bool setclr){
  
  int registr = (int)(channel/2) + 0xc802;
  int regvalue;
  if(channel%2)
    regvalue = 0xffff0000;
  else
    regvalue = 0xffff;

  int status;
  if(setclr)
    status = trb_register_setbit(board,registr,regvalue);
  else
    status = trb_register_clearbit(board,registr,regvalue);

  
  if(status == -1)   cm_msg(MERROR,"setup_trb","Problem with setting enable bit ");
  if (trb_errno == TRB_STATUS_WARNING) cm_msg(MERROR,"setup_trb","Status-Bit(s) have been set:\n%s\n", trb_termstr(trb_term));

  return 0;
}


INT setup_trb(){

   std::string path;
   path += "/Equipment";
   path += "/";
   path += EQ_NAME;
   path += "/Settings";

   std::string path1 = path + "/UseWindowing";

   BOOL use_windowing = true;
   int size = sizeof(use_windowing);
   int status = db_get_value(hDB, 0, path1.c_str(), &use_windowing, &size, TID_BOOL, TRUE);
  
  // enable/disable the event window (get TDC for some/all hits)
  if(use_windowing){
     cm_msg(MINFO,"setup_trb","Using TDC windowing ");
     for(int i = 0; i < NUM_FPGA; i++){

       // If we are using windowing, then set the window size before and after
       int window_before, window_after;
       size = sizeof(window_before);
       path1 = path + "/WindowBefore";
       status = db_get_value(hDB, 0, path1.c_str(), &window_before, &size, TID_INT, TRUE);
       path1 = path + "/WindowAfter";
       status = db_get_value(hDB, 0, path1.c_str(), &window_after, &size, TID_INT, TRUE);

       if(0){ // Check status before...
	 status = trb_register_read(fpga_ids[i],0xc801,buffer,BUFFER_SIZE);	 
	 uint32_t before = (buffer[1] & 0x7ff);
	 uint32_t after = ((buffer[1] >> 16) & 0x7ff);
	 printf("Current status of reg c801: 0x%x %x %x  (%i)\n",buffer[1], before, after,status);
       }

       uint32_t reg_value = 0x80000000;
       if(window_before >= 0 && window_before < 0x7ff)
	 reg_value += window_before;
       if(window_after > 0xf && window_after < 0x7ff)
	 reg_value += (window_after << 16);
       
       printf("Values to set: 0x%x 0x%x 0x%x\n",window_before,window_after, reg_value);


       //status = trb_register_setbit(fpga_ids[i],0xc801,0x80000000);
       status = trb_register_write(fpga_ids[i],0xc801,reg_value);
       if(status == -1){
	 cm_msg(MERROR,"setup_trb","Failed to set TRB3 TDC registers; lost communication; TRB3 crashed (clock loss?)");
	 return -1;
       }
       
      
     }
  }else{
     cm_msg(MINFO,"setup_trb","Not using TDC windowing ");
     for(int i = 0; i < NUM_FPGA; i++){
       status = trb_register_clearbit(fpga_ids[i],0xc801,0x80000000);
       if(status == -1){
	 cm_msg(MERROR,"setup_trb","Failed to set TRB3 TDC registers; lost communication; TRB3 crashed (clock loss?)");
	 return -1;
       }
     }
  }


  // enable/disable particular FPGA channels.
  for(int i = 0; i < NUM_FPGA; i++){// loop over FPGA
     
     std::string path2;
     if(i == 0) path2 = path + "/FPGA1Enable";     
     if(i == 1) path2 = path + "/FPGA2Enable";     
     if(i == 2) path2 = path + "/FPGA3Enable";     
     if(i == 3) path2 = path + "/FPGA4Enable";     
     BOOL fpga_enable[NUM_FPGA] = {true,false,false,false};
     size = sizeof(fpga_enable);
     status = db_get_value(hDB, 0, path2.c_str(), &fpga_enable, &size, TID_BOOL, TRUE);

     printf("Enable for FPGA %i: ",i);
     for(int j = 0; j < NUM_FPGA; j++){
        printf("%i ",fpga_enable[j]);
        setEnableBlock(fpga_ids[i], j, fpga_enable[j]);
     }
     printf("\n");
  }

  cm_msg(MINFO,"frontend_init","TRB3 Board Setup Finished");

  return 0;
}


int frontend_init()
{
   int status;

   set_equipment_status(EQ_NAME, "Starting...", "white");
   set_equipment_status(EQ_NAME_TEMP, "Starting...", "white");

   cm_msg(MINFO, "frontend_init", "Running start_trb.sh...");
   system("ssh agtdc@localhost ./start_trb.sh");
   cm_msg(MINFO, "frontend_init", "Running start_trb.sh... done.");

   setenv("DAQOPSERVER", "localhost:199", 0);

   status = cm_get_experiment_database(&hDB, NULL);
   if (status != CM_SUCCESS) {
      cm_msg(MERROR, "frontend_init", "Cannot connect to ODB, cm_get_experiment_database() returned %d", status);
      return FE_ERR_ODB;
   }

   //   cm_set_transition_sequence(TR_STOP,   601);
   std::string path;
   path += "/Equipment";
   path += "/";
   path += EQ_NAME;
   path += "/Settings";

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

   // Setup trbnet connection.  
   init_ports(); 
   
   // Check the FPGA addresses
   uint16_t trb_address;
   trb_address = 0xffff;
   cm_msg(MINFO,"frontend_init","Reading addresses of TRB3s (central FPGA + 4 TDC FPGAs:");
   
   status = trb_read_uid(trb_address, buffer, BUFFER_SIZE);     
   if (status == -1) {    
     cm_msg(MERROR,"frontend_init","TRB3 TDC: read_uid failed.  TRB3 not communicating.\n");
     return -1;     
   } else {      
      for (int i = 0; i < status; i += 4) {     
         cm_msg(MINFO,"frontend_init","0x%04x  0x%08x%08x  0x%02x",           
                buffer[i + 3],
                buffer[i], buffer[i + 1], buffer[i + 2]);
      }   
   }     

   // Configure TRB3
   status = setup_trb();
   if(status != 0) return FE_ERR_HW;

   // Reset sanity check variables;
   gLastEventNumber = -1;
   gNumberNonseqEventNumbers = 0;

   set_equipment_status(EQ_NAME, "Ok", "#00FF00");

   cm_msg(MINFO, "frontend_init", "Finished Initializating TRB3 Frontend \"%s\", listening on UDP port %d", EQ_NAME, udp_port);
   return SUCCESS;
}

int frontend_loop()
{
   ss_sleep(10);
   return SUCCESS;
}

int begin_of_run(int run_number, char *error)
{
   gUnknownPacketCount = 0;
   gSkipUnknownPackets = false;
   gTemperatureReadFailed = false;
   int status = setup_trb();
   if(status != 0) return FE_ERR_HW;

   // Reset sanity check variables;
   gLastEventNumber = -1;
   gNumberNonseqEventNumbers = 0;

   return SUCCESS;
}
 #include <unistd.h>
#define MAX_UDP_SIZE (0x10000)
int end_of_run(int run_number, char *error)
{

  if( gNumberNonseqEventNumbers != 0){
    cm_msg(MINFO, "end_of_run", "Total events with non-sequential event numbers in this run = %i\n", gNumberNonseqEventNumbers);  
  }


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
//#define MAX_UDP_SIZE (0x10000)

/* Swap bytes in 32 bit value.  */
#define R__bswap_constant_32(x) \
     ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |               \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

int read_event(char *pevent, int off)
{
   char buf[MAX_UDP_SIZE];
   char bankname[5];
   
   int length = read_udp(gDataSocket, buf, MAX_UDP_SIZE, bankname);
   if (length <= 0)
     return 0;   

   bk_init32(pevent);
   char* pdata;
   bk_create(pevent, bankname, TID_BYTE, (void**)&pdata);
   memcpy(pdata, buf, length);
   bk_close(pevent, pdata + length);

   // Data integrity check; confirm that event numbers are sequential (while handling roll-over)
   if(length > 5){
     uint32_t* fData = reinterpret_cast<uint32_t*>(pdata);
     uint32_t triggerWord = R__bswap_constant_32(fData[5]);
     uint32_t triggerNumber = ((triggerWord & 0xffffff00) >> 8);
     
     if(gLastEventNumber > 0){
       if(gLastEventNumber + 1 != triggerNumber && (gLastEventNumber != 0xffffff)){
	 printf("%i %i \n",triggerNumber,fData[5]);

	 if(gNumberNonseqEventNumbers <= 20){
	   cm_msg(MERROR, "read_event", "Non-sequential TRB3 event numbers: last=%i current=%i\n",gLastEventNumber,triggerNumber);  	   
	   if ( gNumberNonseqEventNumbers == 20){
	     cm_msg(MERROR, "read_event", "More than 20 non-sequential event numbers: suppressing further messages\n");
	   }	   
	 }
	 gNumberNonseqEventNumbers++;
       }
       
     }
     gLastEventNumber = triggerNumber;
   }

   return bk_size(pevent); 
}



// Read the TRB3 temperatures from trbnet interface
INT read_trb3_temperature(char *pevent, INT off){
  bool overtemp = false;
  static bool gOverTemp = false;
  double temp_limit = 40.0;
  
  // Don't keep reading is the a previous temperature read failed.
  if(gTemperatureReadFailed) return 0;

  /* init bank structure */
  bk_init32(pevent);
  
  float *pdata;
  bk_create(pevent, "TRTM", TID_FLOAT, (void **)&pdata);
  float* ptemp = pdata;

  int status = trb_register_read(0xc001,0,buffer,BUFFER_SIZE);
  if(status == -1){
    cm_msg(MERROR, "read_trb3_temperature", "Failed to get TRB3 temperature; lost communication; TRB3 crashed (clock loss?)");    
    set_equipment_status(EQ_NAME_TEMP, "Communication error", "#FF0000");
    gTemperatureReadFailed = true;
    return 0;    
  }
  float temp = (float)((buffer[1] & 0xfff00000) >> 20)/16.0;
  *pdata++ = temp;
  //printf("central FPGA temperature: %f\n",temp);
  if (temp > temp_limit) {
    if (!gOverTemp) {
      cm_msg(MERROR, "read_trb3_temperature", "TRB3 central FPGA is too hot: %f degC", temp);
    }
    overtemp = true;
  }
  for(int i = 0; i < NUM_FPGA; i++){
    status = trb_register_read(fpga_ids[i],0,buffer,BUFFER_SIZE);
    if(status == -1){
      cm_msg(MERROR, "read_trb3_temperature", "Failed to get TRB3 temperature; lost communication; TRB3 crashed (clock loss?)");
      set_equipment_status(EQ_NAME_TEMP, "Communication error", "#FF0000");
      gTemperatureReadFailed = true;
      return 0;    
    }
    temp = (float)((buffer[1] & 0xfff00000) >> 20)/16.0;
    *pdata++ = temp;    

    if (temp > temp_limit) {
      if (!gOverTemp) {
	cm_msg(MERROR, "read_trb3_temperature", "TRB3 FPGA %d is too hot: %f degC", i, temp);
      }
      overtemp = true;
    }
  }

  bk_close(pevent, pdata );

  if (0) {
  printf("FPGA temperatures: %.1f %.1f %.1f %.1f %.1f\n",
	 ptemp[0],
	 ptemp[1],
	 ptemp[2],
	 ptemp[3],
	 ptemp[4]);
  }

  // check for strange failure mode: all tempteratures read the same value
  bool all_same = true;
  for (int i=1; i<NUM_FPGA; i++) {
    if (fabs(ptemp[i] - ptemp[0]) > 0.001)
      all_same = false;
  }

  static bool gAllSame = false;

  if (all_same && !gAllSame) {
    cm_msg(MERROR, "read_trb3_temperature", "TRB3 FPGA temperatures all read the same value");
    gAllSame = true;
  } else if (gAllSame && !all_same) {
    cm_msg(MINFO, "read_trb3_temperature", "TRB3 FPGA temperatures all read different values");
    gAllSame = false;
  }

  if (gOverTemp && !overtemp) {
    cm_msg(MINFO, "read_trb3_temperature", "TRB3 FPGA temperature is okey now");
    gOverTemp = false;
  }
  
  if (overtemp) {
    set_equipment_status(EQ_NAME_TEMP, "FPGA is too hot", "yellow");
    gOverTemp = true;
  } else if (all_same) {
    set_equipment_status(EQ_NAME_TEMP, "FPGA temperature problem", "yellow");
  } else {
    set_equipment_status(EQ_NAME_TEMP, "Ok", "#00FF00");
  }

  return bk_size(pevent);
}

