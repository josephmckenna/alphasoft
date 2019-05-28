#ifndef __TAlphaGeoMaterialXML__
#define __TAlphaGeoMaterialXML__

#include <TDOMParser.h>
#include <TXMLAttr.h>
#include <TXMLNode.h>
#include <TList.h>
#include <Riostream.h>
#include <TGeoManager.h>
#include <stdlib.h>

extern "C" int operainit_(char*filename);

class TAlphaGeoMaterialXML
{
 private:
  Int_t fiVol;
  Int_t fBt;
  Double_t fBz;
  Double_t fBmax;
  Double_t fMxang;
  Double_t fMxmul;
  Double_t fDedx;
  Double_t fEpsil;
  Double_t fMnstep;

 public:
  TAlphaGeoMaterialXML() {}
  
  virtual ~TAlphaGeoMaterialXML() {}
  
  void ParseParameters( TXMLNode * node )
  { 
    TString filename;

    for(;node;node = node->GetNextNode()){
      if(node->GetNodeType() == TXMLNode::kXMLElementNode) {
	if(strcmp(node->GetNodeName(),"iVol")==0)
	  fiVol = atoi(node->GetText());
	if(strcmp(node->GetNodeName(),"Bt")==0)
	  fBt = atoi(node->GetText());
	if(strcmp(node->GetNodeName(),"Bz")==0)
	  fBz = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"Bmax")==0)
	  fBmax = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"Mxang")==0)
	  fMxang = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"Mxmul")==0)
	  fMxang = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"Dedx")==0)
	  fDedx = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"Epsil")==0)
	  fEpsil = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"Mnstep")==0)
	  fMnstep = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"FieldMap")==0)
	  filename = node->GetText();
      }
    }
    if( fBt == 2 && operainit_((Char_t*)filename.Data()) == 0 )
      {}
    else
      {
	fBt = 3;
      }
  }

  void ParseElement( TXMLNode * node, Double_t &a, Double_t &z, Double_t &w )
  {
    for(;node;node = node->GetNextNode()){
      if(node->GetNodeType() == TXMLNode::kXMLElementNode) {
	if(strcmp(node->GetNodeName(),"A")==0)
          a = atof( node->GetText() );	  
	if(strcmp(node->GetNodeName(),"Z")==0)
          z = atof( node->GetText() );
	if(strcmp(node->GetNodeName(),"W")==0)
          w = atof( node->GetText() );
      }
    }
  }
  
  void ParseMixture( TXMLNode * node )
  {
    TGeoMixture * mixture = new TGeoMixture();
    Double_t a = 0.;
    Double_t z = 0.;
    Double_t w = 0.;
    
    for(;node;node = node->GetNextNode()){
      if(node->GetNodeType() == TXMLNode::kXMLElementNode) {
	if(strcmp(node->GetNodeName(),"name")==0)
          mixture->SetName( node->GetText() );
	if(strcmp(node->GetNodeName(),"element")==0) {
	  ParseElement( node->GetChildren(), a,z,w );
	  mixture->AddElement( a,z,w );
	}
	if(strcmp(node->GetNodeName(),"dens")==0)
          mixture->SetDensity( atof(node->GetText()) );
      }
    }
    gGeoManager->AddMaterial( mixture );
  }

  void ParseMaterial( TXMLNode * node )
  {
    TString name;
    Double_t a = 10.0e-16;
    Double_t z = 10.0e-16;
    Double_t rho= 10.0e-16;
    Double_t radlen= 1.0;
    for(;node;node = node->GetNextNode()){
      if(node->GetNodeType() == TXMLNode::kXMLElementNode) {
	if(strcmp(node->GetNodeName(),"name")==0)
	  name = node->GetText();
	if(strcmp(node->GetNodeName(),"A")==0)
	  a = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"Z")==0)
	  z = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"dens")==0)
	  rho = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"radl")==0)
	  radlen = atof(node->GetText());
      }
    }
    new TGeoMaterial(name.Data(),a,z,rho,radlen);
  }
  
  void ParseMedium( TXMLNode * node )
  {
    TString name;
    Int_t numed =0;
    TGeoMaterial * mat = NULL;
    
    for(;node;node = node->GetNextNode()){
      if(node->GetNodeType() == TXMLNode::kXMLElementNode) {
	if(strcmp(node->GetNodeName(),"N")==0)
	  numed = atoi(node->GetText());
	if(strcmp(node->GetNodeName(),"name")==0)
	  name = node->GetText();
	if(strcmp(node->GetNodeName(),"nmaterial")==0){
	  mat = gGeoManager->GetMaterial( node->GetText() );
	  if( !mat ) {
	    printf("Not a valid material (%s)!\n",node->GetText());
	    return;
	  }
	}
      }
    }
    
    Double_t params [] = {static_cast<Double_t>(fiVol),static_cast<Double_t>(fBt),fBz,fBmax,fMxang,fDedx,fEpsil,fMnstep};
    new TGeoMedium(name.Data(), numed, mat, params);
  }
  
  
  void ParseMaterialXML( TXMLNode * node )
  {
    for(;node;node = node->GetNextNode()){
      if(node->GetNodeType() == TXMLNode::kXMLElementNode) {
	if(strcmp(node->GetNodeName(),"material")==0)
	  ParseMaterial(node->GetChildren());
	if(strcmp(node->GetNodeName(),"parameters")==0)
	  ParseParameters(node->GetChildren());
	if(strcmp(node->GetNodeName(),"mixture")==0)
	  ParseMixture(node->GetChildren());
	if(strcmp(node->GetNodeName(),"medium")==0)
	  ParseMedium(node->GetChildren());
      }
      ParseMaterialXML(node->GetChildren());
    }
  }
  
  Int_t ParseFile( const char * filename )
  {
    TDOMParser *domParser = new TDOMParser();
    domParser->SetValidate(true);
    Int_t parsecode = domParser->ParseFile(filename);
    if(parsecode < 0) {
      std::cerr << domParser->GetParseCodeMessage(parsecode) << std::endl;
      return -1;
    }
    TXMLNode * node = domParser->GetXMLDocument()->GetRootNode();
    printf("Parsing: %s, RootNodeName: %s\n",filename, node->GetNodeName());
    ParseMaterialXML(node);
    printf("Finished parsing material XML\n");
    delete domParser;
    return 0;
  }
  
  ClassDef( TAlphaGeoMaterialXML, 1 );
};


#endif
