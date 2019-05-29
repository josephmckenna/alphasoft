#ifndef __TAlphaGeoEnvironmentXML__
#define __TAlphaGeoEnvironmentXML__

#include "TGeoManager.h"

#include <TDOMParser.h>
#include <TXMLAttr.h>
#include <TXMLNode.h>

class TAlphaGeoEnvironmentXML
{
  private:

  public:
    TAlphaGeoEnvironmentXML() {}
    TAlphaGeoEnvironmentXML( const char * filename ) {
      ParseFile( filename );
    }

    virtual ~TAlphaGeoEnvironmentXML() {}

    void ParsePrimary( TXMLNode * node )
    {
      TString name;
      TString medium;
      TString shape;
      Double_t rmin = 0.;
      Double_t rmax = 0.;
      Double_t dz = 0.;
      //Double_t x = 0.;
      //Double_t y = 0.;
      //Double_t z = 0.;
      for(;node;node = node->GetNextNode()){
        if(node->GetNodeType() == TXMLNode::kXMLElementNode) {
         if(strcmp(node->GetNodeName(),"name")==0)
          name = node->GetText();
         if(strcmp(node->GetNodeName(),"medium")==0)
          medium = node->GetText();
         if(strcmp(node->GetNodeName(),"shape")==0)
          shape = node->GetText();
         if(strcmp(node->GetNodeName(),"rmin")==0)
          rmin = atof(node->GetText());
         if(strcmp(node->GetNodeName(),"rmax")==0)
          rmax = atof(node->GetText());
         if(strcmp(node->GetNodeName(),"dz")==0)
          dz = atof(node->GetText());
         //if(strcmp(node->GetNodeName(),"X")==0)
          //x = atof(node->GetText());
         //if(strcmp(node->GetNodeName(),"Y")==0)
          //y = atof(node->GetText());
         //if(strcmp(node->GetNodeName(),"Z")==0)
          //z = atof(node->GetText());
        }
      }
      Double_t params[] = { rmin, rmax, dz };
      TGeoVolume * top = gGeoManager->Volume( name, shape.Data(), gGeoManager->GetMedium(medium)->GetId(),params,3);

      gGeoManager->SetTopVolume(top);
      top->SetVisibility( kFALSE );
    }

    void ParseTube( TXMLNode * node )
    {
      TString name;
      TString medium;
      TString shape;
      Double_t rmin = 0.;
      Double_t rmax = 0.;
      Double_t dz = 0.;
      Double_t x = 0.;
      Double_t y = 0.;
      Double_t z = 0.;
      for(;node;node = node->GetNextNode()){
        if(node->GetNodeType() == TXMLNode::kXMLElementNode) {
	  if(strcmp(node->GetNodeName(),"name")==0)
	    name = node->GetText();
	  if(strcmp(node->GetNodeName(),"medium")==0)
	    medium = node->GetText();
	  if(strcmp(node->GetNodeName(),"shape")==0)
	    shape = node->GetText();
	  if(strcmp(node->GetNodeName(),"rmin")==0)
	    rmin = atof(node->GetText());
	  if(strcmp(node->GetNodeName(),"rmax")==0)
	    rmax = atof(node->GetText());
	  if(strcmp(node->GetNodeName(),"dz")==0)
	    dz = atof(node->GetText());
	  if(strcmp(node->GetNodeName(),"X")==0)
	    x = atof(node->GetText());
	  if(strcmp(node->GetNodeName(),"Y")==0)
	    y = atof(node->GetText());
	  if(strcmp(node->GetNodeName(),"Z")==0)
	    z = atof(node->GetText());
        }
      }
      Double_t params[] = { rmin, rmax, dz };
      Double_t *ubuf = 0;
      
      TGeoVolume * tube = gGeoManager->Volume( name, shape.Data(), gGeoManager->GetMedium(medium)->GetId(),params,3);
      gGeoManager->Node( name, 0, gGeoManager->GetTopVolume()->GetName(),
			 x, y, z, 0, kTRUE, ubuf);

       if(strncmp(name,"Mirror",6) == 0)         tube->SetLineColor(kGreen);
       else if (strncmp(name,"Sol",3) == 0)      tube->SetLineColor(kMagenta);
       else if (strncmp(name,"LHe",3) == 0)      tube->SetLineColor(kBlue);
       else if (strncmp(name,"OVC",3) == 0)      tube->SetLineColor(kYellow);
       else if (strncmp(name,"UHV",3) == 0)      tube->SetLineColor(kYellow-9);
       else if (strncmp(name,"Heat",4) == 0)     tube->SetLineColor(kOrange+2);
       else if (strncmp(name,"Supp",4) == 0)     tube->SetLineColor(kGray+3);
       else if (strncmp(name,"Elec",4) == 0)     tube->SetLineColor(kOrange);
       else if (strncmp(name,"EPO",3) == 0)      tube->SetLineColor(kWhite);
       //else 
      tube->SetVisibility( kFALSE );
      
    }

    void ParseTubs( TXMLNode * node )
    {
      TString name;
      TString medium;
      TString shape;
      Double_t rmin = 0.;
      Double_t rmax = 0.;
      Double_t dz = 0.;
      Double_t phi1 = 0.;
      Double_t phi2 = 0.;
      Double_t x = 0.;
      Double_t y = 0.;
      Double_t z = 0.;
      for(;node;node = node->GetNextNode()){
        if(node->GetNodeType() == TXMLNode::kXMLElementNode) {
	  if(strcmp(node->GetNodeName(),"name")==0)
	    name = node->GetText();
	  if(strcmp(node->GetNodeName(),"medium")==0)
	    medium = node->GetText();
	  if(strcmp(node->GetNodeName(),"shape")==0)
	    shape = node->GetText();
	  if(strcmp(node->GetNodeName(),"rmin")==0)
	    rmin = atof(node->GetText());
	  if(strcmp(node->GetNodeName(),"rmax")==0)
	    rmax = atof(node->GetText());
	  if(strcmp(node->GetNodeName(),"dz")==0)
	    dz = atof(node->GetText());
	  if(strcmp(node->GetNodeName(),"phi1")==0)
	    phi1 = atof(node->GetText());
	  if(strcmp(node->GetNodeName(),"phi2")==0)
	    phi2 = atof(node->GetText());
	  if(strcmp(node->GetNodeName(),"X")==0)
	    x = atof(node->GetText());
	  if(strcmp(node->GetNodeName(),"Y")==0)
	    y = atof(node->GetText());
	  if(strcmp(node->GetNodeName(),"Z")==0)
	    z = atof(node->GetText());
        }
      }
      Double_t params[] = { rmin, rmax, dz, phi1, phi2 };
      Double_t *ubuf = 0;
      
      TGeoVolume * tubs = gGeoManager->Volume( name, shape.Data(), gGeoManager->GetMedium(medium)->GetId(),params,5);
      gGeoManager->Node( name, 0, gGeoManager->GetTopVolume()->GetName(), x, y, z, 0, kTRUE, ubuf);
      
      if (strncmp(name,"OCT",3) == 0)      tubs->SetLineColor(kRed);
      else if (strncmp(name,"EPO",3) == 0) tubs->SetLineColor(kWhite);
      //else 
      tubs->SetVisibility( kFALSE );
    }

    void ParseEnvironment( TXMLNode * node )
    {
      for(;node;node = node->GetNextNode()){
        if(node->GetNodeType() == TXMLNode::kXMLElementNode) {
	  if(strcmp(node->GetNodeName(),"primaryVolume")==0)
	    ParsePrimary(node->GetChildren());
	  if(strcmp(node->GetNodeName(),"tubeVolume")==0)
	    ParseTube(node->GetChildren());
	  if(strcmp(node->GetNodeName(),"tubsVolume")==0)
	    ParseTubs(node->GetChildren());
        }
       ParseEnvironment(node->GetChildren());
      }
    }

    Int_t ParseFile( const char * filename )
    {
      TDOMParser *domParser = new TDOMParser();
      domParser->SetValidate(false);
      Int_t parsecode = domParser->ParseFile(filename);
      if(parsecode < 0) {
        std::cerr << domParser->GetParseCodeMessage(parsecode) << std::endl;
        return -1;
      }
      TXMLNode * node = domParser->GetXMLDocument()->GetRootNode();
      printf("Parsing Environmental XML file: %s, root node: %s\n",filename,node->GetNodeName());
      ParseEnvironment(node);
      printf("Finished parsing xml\n");
      delete domParser;
      return 0;
    }

    ClassDef( TAlphaGeoEnvironmentXML, 1 );
};


#endif
