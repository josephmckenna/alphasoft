#ifndef __TAlphaGeoDetectorXML__
#define __TAlphaGeoDetectorXML__

#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include <TDOMParser.h>
#include <TXMLAttr.h>
#include <TXMLNode.h>
#include <TList.h>
#include <Riostream.h>

class TAlphaGeoDetectorXML {
 private:
  Double_t silx;
  Double_t sily;
  Double_t silz;
  TString silmedium;

 public:
  TAlphaGeoDetectorXML() {
    silx = 0.015*2.75;
    sily = 3;
    silz = 11.5;
    silmedium = "Silicon Medium";
  }
  virtual ~TAlphaGeoDetectorXML() {}
  
  TAlphaGeoDetectorXML( const char * filename ) {
    silx = 0.015*2.75;
    sily = 3;
    silz = 11.5;
    silmedium = "Silicon Medium";
    ParseFile( filename );
  }
  
  void ParseHybrid( TXMLNode * node )
  {
    TString name;
    Int_t number = 0;
    Double_t x = 0.;
    Double_t y = 0.;
    Double_t z = 0.;
    //Double_t cos = 0.;
    //Double_t sin = 0.;
    Double_t theta1 = 0.;
    Double_t phi1 = 0;
    Double_t theta2 = 0.;
    Double_t phi2 = 0;
    Double_t theta3 = 0.;
    Double_t phi3 = 0;
    for(;node;node = node->GetNextNode()){
      if(node->GetNodeType() == TXMLNode::kXMLElementNode) {
	if(strcmp(node->GetNodeName(),"name")==0)
	  name = node->GetText();
	if(strcmp(node->GetNodeName(),"number")==0)
	  number = atoi(node->GetText());
	if(strcmp(node->GetNodeName(),"X")==0)
	  x = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"Y")==0)
	  y = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"Z")==0)
	  z = atof(node->GetText());
	//if(strcmp(node->GetNodeName(),"cos")==0)
	  //cos = atof(node->GetText());
	//if(strcmp(node->GetNodeName(),"sin")==0)
	  //sin = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"theta1")==0)
	  theta1 = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"phi1")==0)
	  phi1 = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"theta2")==0)
	  theta2 = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"phi2")==0)
	  phi2 = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"theta3")==0)
	  theta3 = atof(node->GetText());
	if(strcmp(node->GetNodeName(),"phi3")==0)
	  phi3 = atof(node->GetText());
      }
    }
    
    TGeoCombiTrans * combitrans = new TGeoCombiTrans(x,y,z,new TGeoRotation(name,theta1,phi1,theta2,phi2,theta3,phi3));
    
    TGeoVolume * hybrid = gGeoManager->MakeBox(name, gGeoManager->GetMedium(silmedium.Data()),silx,sily,silz);
    hybrid->SetLineColor(kCyan);
    hybrid->SetVisibility( kFALSE );
        
    gGeoManager->GetTopVolume()->AddNode( hybrid,number,combitrans );
  }
  
  void ParseHybrids( TXMLNode * node )
  {
    for(;node;node = node->GetNextNode()){
      if(node->GetNodeType() == TXMLNode::kXMLElementNode) {
	if(strcmp(node->GetNodeName(),"silx")==0)
	  silx = atof( node->GetText() );
	if(strcmp(node->GetNodeName(),"sily")==0)
	  sily = atof( node->GetText() );
	if(strcmp(node->GetNodeName(),"silz")==0)
	  silz = atof( node->GetText() );
	if(strcmp(node->GetNodeName(),"silmedium")==0)
	  silmedium = node->GetText();
	if(strcmp(node->GetNodeName(),"module")==0){
	  ParseHybrid(node->GetChildren());
	}
      }
      ParseHybrids(node->GetChildren());
    }
  }
  
  Int_t ParseFile( TString filename )
  {
    printf("det: %s\n",filename.Data());
    TDOMParser *domParser = new TDOMParser();
    domParser->SetValidate(false);
    Int_t parsecode = domParser->ParseFile(filename);
    if(parsecode < 0) {
      std::cerr << domParser->GetParseCodeMessage(parsecode) << std::endl;
      return -1;
    }
    TXMLNode * node = domParser->GetXMLDocument()->GetRootNode();
    ParseHybrids(node);
    delete domParser;
    return 0;
  }
  
  ClassDef( TAlphaGeoDetectorXML, 1);
    
};

#endif
