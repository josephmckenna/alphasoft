#include "LyAlpha.C"

int main()
{
    TA2Plot* basic1 = new TA2Plot();
    basic1->AddTimeGate(57181,30000,35000);
    basic1->SetLVChannel("HPRO",4);
    basic1->LoadData();
    TCanvas* BasicCanvas1 = basic1->DrawCanvas("Example plot of run 57181 with HPRO[4] = BMod inside and ID52[3] = Carlsberg DS ");

    PlotLyAlphaScan(57019, 0);

    unsigned int runStart = Get_A2Analysis_Report(57181).GetRunStartTime();

    return EXIT_SUCCESS;
}