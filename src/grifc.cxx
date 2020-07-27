// grifc.cxx
//
// ALPHA-T command line communication tool
//
// K.Olchanski TRIUMF 2018
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

#include "GrifComm.h"

int main(int argc, char* argv[])
{
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   signal(SIGPIPE, SIG_IGN);

   //thread_test();

   TMFE* mfe = TMFE::Instance();

   TMFeError err = mfe->Connect("grifc");
   if (err.error) {
      printf("Cannot connect, bye.\n");
      return 1;
   }

   std::string hostname = "trg01";

   GrifComm* grifc = new GrifComm();
   grifc->mfe = mfe;
   grifc->fHostname = hostname;
   grifc->OpenSockets();
   grifc->fFailed = false;

   if (1) {
      // communication test
      int addr = 0x23; // pulser period register
      uint32_t v = 0;
      while (!mfe->fShutdownRequested) {
         std::string errstr;
         bool ok = grifc->try_write_param(addr, 0xFFFF, v, &errstr);
         if (!ok) printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d), error: %s\n", addr, ok, v, v, errstr.c_str());
         uint32_t r = 0;
         ok = grifc->read_param(addr, 0xFFFF, &r);
         if (r != v) {
            printf("mismatch: write 0x%08x, read 0x%08x\n", v, r);
         }
         v++;
      }
   } else if (0) {
      int addr = 0x37;
      uint32_t v = 0x80000000;
      bool ok = grifc->write_param(addr, 0xFFFF, v);
      printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);
      v = 0;
      ok = grifc->write_param(addr, 0xFFFF, v);
      printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);

      v = 0x40000000;
      ok = grifc->write_param(addr, 0xFFFF, v);
      printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);

      FILE *fp = fopen("test.mlu", "r");

      while (1) {
         char buf[256];
         char*s = fgets(buf, sizeof(buf), fp);
         if (!s)
            break;

         int vaddr = strtoul(s, &s, 0);
         int value = strtoul(s, &s, 0);

         printf("addr 0x%08x, value %d, read: %s", addr, value, buf);

         //v = 0x40010101;
         v = 0x40000000;
         v |= (value<<16);
         v |= vaddr;
         ok = grifc->write_param(addr, 0xFFFF, v);
         printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);
      }

      fclose(fp);

      v = 0;
      ok = grifc->write_param(addr, 0xFFFF, v);
      printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);

   } else if (0) {
      int addr = 0x37;
      uint32_t v = 0x80000000;
      bool ok = grifc->write_param(addr, 0xFFFF, v);
      printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);
      v = 0;
      ok = grifc->write_param(addr, 0xFFFF, v);
      printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);
      v = 0x40000000;
      ok = grifc->write_param(addr, 0xFFFF, v);
      printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);
      v = 0;
      ok = grifc->write_param(addr, 0xFFFF, v);
      printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);
      v = 0x40010101;
      ok = grifc->write_param(addr, 0xFFFF, v);
      printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);
      v = 0;
      ok = grifc->write_param(addr, 0xFFFF, v);
      printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);
   } else if (0) {
      uint32_t v = 0;
      int addr = 0x1F;
      bool ok = grifc->read_param(addr, 0xFFFF, &v);

      printf("grifc: read addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);
   } else {
      int addr = 0x37;
      uint32_t v = 0x1234;
      bool ok = grifc->write_param(addr, 0xFFFF, v);
      printf("grifc: write addr 0x%08x, ok %d, value 0x%08x (%d)\n", addr, ok, v, v);
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
