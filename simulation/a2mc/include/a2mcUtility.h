#include <iostream>
#include <sstream>
#include <iomanip>
#include <time.h> 
#include "TSystemDirectory.h"
#include "TFile.h"

using namespace std;

class Utility {
public:
	Utility(int, char**);
	~Utility();
	bool checkDir(const char *, const char *);
	int GetNEvents() const {return m_numberOfEvents;}
	int GetRun() const {return m_run;}
    string GetRunTime() const {return m_run_time;}
    string GetIniFile() const {return m_ini_file;}
private:
	void synthaxError() {
 		cout << "Synthax error!" << endl;
  		exit(1);
	}
	int m_numberOfEvents;
    string m_run_time;
	int m_run;
    string m_ini_file; ///< initialization file (usually ./a2MC.ini)
};