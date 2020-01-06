#ifndef _TSISQOD_
#define _TSISQOD_
class TSISQOD: public TObject
{
  public:
int clock;
double t;
int RunNumber;
int DumpStarts[10];
int DumpStop[10];
//Error
//Cat start dumps
//Cat stop dumps
//Atm start dumps
//Atm stop dumps
// RCT start dumps
// RCT stop dumps
//Microwave Sweep Starts
//Microwave Sweep Stops

//SIS QOD?
//IO32_NOBUSY
//Error
//IO32_NOBUSY rate
//Error
//IO32
//Error
//IO32 rate
//Error
//ATOM_OR PMT
//Error
//ATOM_OR PMT rate
//Error
//ATOM_AND PMT
//Error
//ATOM_AND PMT rate
//Error
//Si Triggers/ATOM_OR
//Error
//Si Triggers/ATOM_AND
//Error
ClassDef(TSISQOD,1);
};
#endif
