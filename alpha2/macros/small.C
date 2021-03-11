TCanvas* BasicCanvas1;
TCanvas* BasicCanvas2;

void small()
{
    TA2Plot* basic1 = new TA2Plot();
    basic1->AddTimeGate(45000,0,350);
    basic1->LoadData();
    BasicCanvas1=basic1->DrawCanvas("Example plot of run 45000");
    basic1->DrawCanvas("Example plot of run 45000");
    BasicCanvas1->Draw();
    basic1->PrintFull();

    TA2Plot* basic2 = new TA2Plot();
    basic2->AddTimeGate(57181,0,350);
    basic2->LoadData();
    BasicCanvas2=basic2->DrawCanvas("Example plot of run 57181");
    basic2->DrawCanvas("Example plot of run 57181");
    BasicCanvas2->Draw();
    basic2->PrintFull();

    TA2Plot* basic3 = new TA2Plot();
    basic3->AddTimeGate(51965,0,350);
    basic3->LoadData();
    BasicCanvas3=basic3->DrawCanvas("Example plot of run 51965");
    basic3->DrawCanvas("Example plot of run 51965");
    BasicCanvas3->Draw();
    basic3->PrintFull();

    TA2Plot* basic4 = new TA2Plot();
    basic4->AddTimeGate(39993,0,350);
    basic4->LoadData();
    BasicCanvas4=basic4->DrawCanvas("Example plot of run 39993");
    basic4->DrawCanvas("Example plot of run 5139993965");
    BasicCanvas4->Draw();
    basic4->PrintFull();

    TA2Plot basic2(*basic1);
    basic2.PrintFull();
    //basic2.LoadData();
    BasicCanvas2=basic2.DrawCanvas("Example plot of copy of 45000");
    BasicCanvas2->Draw();
    basic2.PrintFull();

    TA2Plot basicsum = &(*basic1 + *basic2);


    TA2Plot* basic1 = new TA2Plot();
    basic1->AddTimeGate(51965,0,350);
    basic1->LoadData();
    BasicCanvas1=basic1->DrawCanvas("Example plot of run 51965");
    TA2Plot* basic2 = new TA2Plot();
    basic2->AddTimeGate(57181,0,350);
    basic2->LoadData();
    BasicCanvas2=basic2->DrawCanvas("Example plot of run 57181");
    TA2Plot basicsum = (*basic1 + *basic2);
    basicsum.DrawCanvas("addition");

    basic1->PrintFull();
    basic2->PrintFull();
    basicsum->PrintFull();

}