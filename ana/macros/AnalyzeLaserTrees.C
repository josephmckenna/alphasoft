{
    TTree *fPadTree = (TTree*)_file0->FindObjectAny("fPadTree");
    TH2D hp1("hp1","",576,0,576,32,0,32);
    hp1.SetMarkerColor(kRed);
    fPadTree->Draw("col:row>>hp1","time<4000");
    TH2D hp2("hp2","",576,0,576,32,0,32);
    fPadTree->Draw("col:row>>hp2","time>4000","same");

    TTree *fAnodeTree = (TTree*)_file0->FindObjectAny("fAnodeTree");
    fAnodeTree->Draw("time");
    TH2D ha1("ha1","",256,256,512,700,0,70000);
    ha1.SetMarkerColor(kRed);
    fAnodeTree->Draw("amp:wire>>ha1","time<4000");
    TH2D ha2("ha2","",256,256,512,700,0,70000);
    ha2.SetMarkerColor(kBlack);
    fAnodeTree->Draw("amp:wire>>ha2","time>4000","same");
}
