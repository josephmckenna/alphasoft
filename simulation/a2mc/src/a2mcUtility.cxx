///< ##############################################
///< Developed for the Alpha experiment [Nov. 2020]
///< germano.bonomi@cern.ch
///< ##############################################

#include "a2mcUtility.h"

Utility::Utility(int argc, char** argv) {
	m_numberOfEvents = 10;
	m_run = 0;

	if (argc < 2) return;
    ///< Setting the time of the run start
    time_t now = time(nullptr);
    struct tm * time;
    time = localtime ( &now );
    ostringstream run_start;
    run_start   << time->tm_year+1900 << "-" 
                << std::setw(2) << std::setfill('0') << time->tm_mon+1 << "-" 
                << std::setw(2) << std::setfill('0') << time->tm_mday << "-" 
                << std::setw(2) << std::setfill('0') << time->tm_hour << "-" 
                << std::setw(2) << std::setfill('0') << time->tm_min << "-" 
                << std::setw(2) << std::setfill('0') << time->tm_sec;
    m_run_time = run_start.str(); // Time in which the run started

    ///< Setting (getting) the number of run and the number of events
    for (int i = 1; i < argc; i = i + 2) {
	    if (strcmp(argv[i], "-events") == 0) { //Number of events                                                                             
	        if (argc > i + 1) m_numberOfEvents = atoi(argv[i+1]);
			else synthaxError();
	    } else if (strcmp(argv[i], "-run") == 0) { //Run number                                                        
	        if (argc > i + 1) m_run = atof(argv[i+1]);
			else synthaxError();
	    } else {
	    	cout << "Unknown option!" << endl;
        	exit(1);
	    }
	}
}


Utility::~Utility() {}


bool Utility::checkDir(const char *dirname="./", const char *ext="root") {
   bool dir_exist = false;
   TSystemDirectory dir(dirname, dirname);
   TList *files = dir.GetListOfFiles();
   if (files) {
      TSystemFile *file;
      TString fname;
      TIter next(files);
      while ((file=(TSystemFile*)next())) {
         fname = file->GetName();
         if (file->IsDirectory() && fname.EqualTo(ext)) {
           dir_exist = true;
         }
      }
   }
   return dir_exist;
}
