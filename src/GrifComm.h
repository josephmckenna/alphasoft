// GrifComm.h - communication with grif-c firmware

#include <stdio.h>
#include <vector>
#include "tmfe.h"

class GrifSocket
{
 public:
   bool fDebug = false;
   std::vector<char> fAddr;
   int fSocket = -1;

 public:
   static const int kMaxPacketSize = 1500;

 public:
   bool open_udp_socket(int server_port, const char *hostname, TMFE* mfe);
   bool waitmsg(int timeout, std::string *errstr);
   int readmsg(char* replybuf, int bufsize, int timeout, std::string *errstr);
   void flushmsg(std::string *errstr);
   bool sndmsg(const char *message, int msglen, std::string* errstr);
};

class GrifComm
{
public:
   TMFE* mfe = NULL;
   std::string fHostname;

public:
   GrifSocket fCmdSocket;
   GrifSocket fDataSocket;

   int fDebug = 0;
   int fTimeout_usec = 10000;

public:
   bool fFailed = true;

public:   

public:   
   bool try_write_param(int par, int chan, int val, std::string *errstr);
   bool try_read_param(int par, int chan, uint32_t* value, std::string *errstr);
   bool read_param(int par, int chan, uint32_t* value);
   bool write_param(int par, int chan, int value);
   bool write_drq();
   bool write_stop();
   bool OpenSockets();
   
private:
   void printreply(const char* replybuf, int bytes);
   const char *parname(int num);
   void param_encode(char *buf, int par, int write, int chan, int val);
};

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
