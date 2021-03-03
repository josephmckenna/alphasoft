//#include "TA2Plot.h"
//#include<THF
//Plot something basic... lets see vertices between 0 and 350 seconds in run 45000
/*TA2Plot* basic = new TA2Plot();
//Set the time window we want
basic->AddTimeGate(
    45000, //runNumber
    0.,    //Start time of our plot
    350.); //End time of our plot

//With everything set... load data from disk
basic->LoadData();
//Render TA2Plot
TCanvas* BasicCanvas=basic->DrawCanvas("Example plot of run 45000");
//Draw canvas
BasicCanvas->Draw();*/

void lukas() {

    //Pulling two basic plots.
    TA2Plot* basic1 = new TA2Plot();
    basic1->AddTimeGate(45000,0.,350.);
    //basic1->SetLVChannel("DETT",1);
    basic1->LoadData();
    TFile * treefile = TFile::Open("basic1and2.root", "RECREATE");
    TTree * tree = new TTree("test", "test");
    tree -> Branch("TA2Plot","TA2Plot",&basic1,32000,0);
    tree -> Fill();
    tree -> Write();
    treefile -> Write();
    treefile -> Close();
    //TBrowser b("output39993.root");

    TA2Plot* basic2 = new TA2Plot();
    basic2->AddTimeGate(57181,0.,350.); 
    //basic2->SetLVChannel("DETT",1);
    basic2->LoadData();
    TFile * treefile0 = TFile::Open("basic2.root", "RECREATE");
    TTree * tree0 = new TTree("test0", "test0");
    tree0 -> Branch("TA2Plot","TA2Plot",&basic2,32000,0);
    tree0 -> Fill();
    tree0 -> Write();
    treefile0 -> Write();
    treefile0 -> Close();

    
    //Copy constructor
    TA2Plot basiccopy(basic1);
    //Tree print
    TFile * treefile1 = TFile::Open("copyconstruct.root", "RECREATE");
    TTree * tree1 = new TTree("test1", "test1");
    tree1 -> Branch("TA2Plot","TA2Plot",&basiccopy,32000,0);
    tree1 -> Fill();
    tree1 -> Write();
    treefile1 -> Write();
    treefile1 -> Close();

    //Assignment operator
    TA2Plot basicassign;
    basicassign = basic1;
    //Tree print
    TFile * treefile2 = TFile::Open("assignment.root", "RECREATE");
    TTree * tree2 = new TTree("test2", "test2");
    tree2 -> Branch("TA2Plot","TA2Plot",&basicassign,32000,0);
    tree2 -> Fill();
    tree2 -> Write();
    treefile2 -> Write();
    treefile2 -> Close();

    //Addition copy operator
    TA2Plot basiccopyaddit = *basic1 + *basic2;
    //Tree print
    TFile * treefile3 = TFile::Open("copyaddition.root", "RECREATE");
    TTree * tree3 = new TTree("test3", "test3");
    tree3 -> Branch("TA2Plot","TA2Plot",&basiccopyaddit,32000,0);
    tree3 -> Fill();
    tree3 -> Write();
    treefile3 -> Write();
    treefile3 -> Close();

    //Assignment addition operator
    TA2Plot basicassignaddit;
    basicassignaddit = *basic1 + *basic2;
    //Tree print
    TFile * treefile4 = TFile::Open("assignaddit.root", "RECREATE");
    TTree * tree4 = new TTree("test4", "test4");
    tree4 -> Branch("TA2Plot","TA2Plot",&basicassignaddit,32000,0);
    tree4 -> Fill();
    tree4 -> Write();
    treefile4 -> Write();
    treefile4 -> Close();


    //Plot something.
    TCanvas* BasicCanvas=basiccopy.DrawCanvas("Example plot of run 45000");
    BasicCanvas->Draw();
}

