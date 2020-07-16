#include "ReadEventTree.hh"
#include "argparse.hh"
#include "TApplication.h"
#include <string>
using namespace std;

int main(int argc, char** argv)
{
  // make a new ArgumentParser
   ArgumentParser parser;
   parser.appName(argv[0]);
   parser.addArgument("-f","--rootfile",1,false);
   parser.addArgument("-p","--plot",1);
    
   // parse the command-line arguments - throws if invalid format
   parser.parse(argc, argv);

   string fname = parser.retrieve<string>("rootfile");

   bool plot = false;
   if( parser.count("plot") ) plot = true;

   TApplication* app;
   if( plot )
      app = new TApplication("ReadEventTree",&argc,argv);
   
   ReadEventTree(fname);

   if( plot )
      app->Run();

   return 0;
}


/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
