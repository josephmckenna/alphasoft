#include "LyAlpha.C"

int main()
{
    TA2Plot* basic1 = new TA2Plot();
    basic1->AddTimeGate(42503,0,-1);
    basic1->AddTimeGate(37495,0,-1);
    basic1->LoadData();
    TCanvas* BasicCanvas1 = basic1->DrawCanvas("Example plot of run 57181 with HPRO[4] = BMod inside and ID52[3] = Carlsberg DS ");
    basic1->WriteEventList("testyfinal");

    //PlotLyAlphaScan(57019, 0);

    //unsigned int runStart = Get_A2Analysis_Report(57181).GetRunStartTime();

    return EXIT_SUCCESS;
}