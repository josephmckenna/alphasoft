void small()
{
    TA2Plot* basic1 = new TA2Plot();
    basic1->AddTimeGate(45000,0.,350.);
    basic1->LoadData();
    TCanvas* BasicCanvas1=basic1->DrawCanvas("Example plot of run 45000");
    BasicCanvas1->Draw();
    basic1->PrintFull();

    TA2Plot basic2(basic1);
    basic2.PrintFull();
    TCanvas* BasicCanvas2=basic2.DrawCanvas("Example plot of run 45000");
    BasicCanvas2->Draw();
    basic2.PrintFull();
}