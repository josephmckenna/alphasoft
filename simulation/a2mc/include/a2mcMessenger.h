#ifndef a2mc_MESSENGER_H
#define a2mc_MESSENGER_H

#include <stdio.h>
#include <string.h>

#include <iostream>
#include <iomanip>

#include <TObject.h>

class a2mcMessenger : public TObject
{
    public:
        a2mcMessenger();
        virtual ~a2mcMessenger();
        // static access method
        static a2mcMessenger* Instance(); 
        void toRight(UInt_t);
        void printLeft(UInt_t, const std::string &);
        void TitleFrame(const std::string &s);

    private:
        static  a2mcMessenger* fgInstance; ///< Singleton instance

    ClassDef(a2mcMessenger,1) //a2mcMessenger
};

#endif //a2mc_MESSENGER_H
