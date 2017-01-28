// feyair.cxx

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "KOsocket.h"

bool Wait(KOsocket*s, int wait_sec, const char* explain)
{
   while (1) {
      int a = s->available();
      //printf("Wait %d sec, available %d\n", wait_sec, a);
      if (a > 0)
         return true;
      if (wait_sec <= 0) {
         printf("Timeout waiting for %s\n", explain);
         return false;
      }
      wait_sec--;
      sleep(1);
   }
   // not reached
}

int main(int argc, char* argv[])
{
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   const char* name = argv[1];

   if (strcmp(name, "tpc01")==0) {
      // good
   } else if (strcmp(name, "tpc02")==0) {
      // good
   } else {
      printf("Only tpc01 and tpc02 permitted. Bye.\n");
      return 1;
   }

   while (1) {
      KOsocket* s = new KOsocket(name, 40);

      char out[256];
      
      sprintf(out, "%s.%d.txt", name, (int)time(NULL));
      
      FILE *fout = fopen(out, "w");
      assert(fout);
      
      printf("Writing to %s\n", out);
      
      setbuf(fout, NULL);
      
      int count = 0;
      
      while (1) {
         time_t t1 = time(NULL);
         
         const char *cmd1 = "uart_regfile_status_read 7 8 3\n";
         s->write(cmd1, strlen(cmd1));

         if (!Wait(s, 10, "reply to uart_regfile_status_read"))
            break;
         
         char reply[102400];

         int rd = s->read(reply, sizeof(reply));
         int v = atoi(reply);
         int have = v & (1<<19);
         if (rd > 0)
            reply[rd] = 0;

         printf("rd %d, value %d, 0x%x, have %d, reply [%s]\n", rd, v, v, have, reply);
         
         time_t t2 = time(NULL);
         
         if (!have) {
            sleep(1);
            continue;
         }
         
         const char *cmd2 = "simult_capture_of_hw_trig_uart_nios_dacs 7 3 0 0\n";
         s->write(cmd2, strlen(cmd2));
         
         int total = 0;
         
         if (!Wait(s, 40, "reply to simult_capture_of_hw_trig_uart_nios_dacs"))
            break;
         
         rd = s->read(reply, sizeof(reply));
         printf("rd %d, reply [%s]\n", rd, reply);
         
         if (rd <= 0)
            break;
         
         total += rd;
         
         reply[rd] = 0;
         
         time_t t3 = time(NULL);
         
         fprintf(fout, "%d %d %d %d %s", count, (int)t1, (int)t2, (int)t3, reply);
         
         while (rd > 0) {
            if (!Wait(s, 3, "data read"))
               break;

            rd = s->read(reply, sizeof(reply));
            printf("%d.", rd);
            if (rd <= 0)
               break;
            total += rd;
            reply[rd] = 0;
            fprintf(fout, "%s", reply);
            //if (rd < 750)
            //break;
         }
         printf("done\n");
         
         time_t te = time(NULL);
         
         printf("event %d, got %d bytes, times %d %d %d\n", count, total, (int)(t2-t1), (int)(t3-t2), (int)(te-t1));
         
         fprintf(fout, " total %d elapsed %d %d %d\n", total, (int)(t2-t1), (int)(t3-t2), (int)(te-t1));
         count++;
      }
      
      fclose(fout);
      fout = NULL;
      
      s->shutdown();
      delete s;
      s = NULL;
   }

   return 0;
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
