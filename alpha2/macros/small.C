TCanvas* BasicCanvas1;
TCanvas* BasicCanvas2;

void small()
{
    TA2Plot* basic1 = new TA2Plot();
    basic1->AddTimeGate(53704,0,300);
    basic1->LoadData();
    BasicCanvas1=basic1->DrawCanvas("Example plot of run 57181");
    basic1->PrintFull();


          TA2Plot* temp = new TA2Plot();
        temp->AddDumpGates(53704, {"Ramp up Sol A"}, {2});
        temp->SetLVChannel("DCTA",4);
        temp->LoadData();
        temp->DrawCanvas();

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


    TA2Plot* basic1 = new TA2Plot();
    basic1->AddTimeGate(45000,0,350);
    basic1->AddDUmp(37000,0,350);
    basic1->AddTimeGate(54000,0,350);
    basic1->AddTimeGate(56000,0,350);
    basic1->LoadData();
    BasicCanvas1=basic1->DrawCanvas("Example plot of run 45000");
    
    TA2Plot* basic2 = new TA2Plot();
    basic2->AddTimeGate(57181,0,350);
    basic2->LoadData();
    BasicCanvas2=basic2->DrawCanvas("Example plot of run 57181");

    TA2Plot basicsum = (*basic1 + *basic2);
    BasicCanvassum=basicsum.DrawCanvas("basicsum");




    basic1->PrintFull();
    basic2->PrintFull();
    basicsum->PrintFull();


    TA2Plot* TotalSumMixing = new TA2Plot();
    TA2Plot* TotalSumCosmic = new TA2Plot();
    int runNuM[] = {37994, 37997, 38881, 38883, 37994, 37997, 38881, 38883, 38891, 39993, 39997, 40932, 40987, 40996, 41987, 41990, 42486, 42488, 42493, 44974, 44983, 45462, 45645, 46981, 46989, 46992, 48773, 49871, 50926, 51965, 51974, 51982, 52987, 52989, 53954, 53991, 53993, 54982, 54989, 56875, 56937, 56974, 57117, 57179, 57207};
    int runNuC[] = {37495, 37846, 39841, 40764, 41678, 41914, 42503, 42796, 42798, 42880, 42907, 42910, 43851, 43978, 45443, 45511, 45678, 46572, 46684, 46724, 46757, 46776, 47643, 47756, 47847, 47851, 47917, 48532, 48824, 49923, 51896, 51899, 51902, 51905, 52943, 53905, 53967, 54935, 54966, 56999, 57003, 57262, 57267, 57273};
    for(int i : runNuM)
    {
        TA2Plot* basic1 = new TA2Plot();
        basic1->AddTimeGate(i,0,350);
        basic1->LoadData();
        TotalSumMixing += *basic1;
    }
    for(int i : runNuC)
    {
        TA2Plot* basic1 = new TA2Plot();
        basic1->AddTimeGate(i,0,350);
        basic1->LoadData();
        *TotalSumCosmic += *basic1;
    }
    TotalSumMixingCanv=TotalSumMixing->DrawCanvas("TotalSumMixing");
    TotalSumCosmicCanv=TotalSumCosmic->DrawCanvas("TotalSumCosmic");



    TA2Plot* basic1 = new TA2Plot();
    basic1->AddTimeGate(37994,0,350);
    basic1->LoadData();
    BasicCanvas1=basic1->DrawCanvas("Example plot of run 45000");

//========================================================================================================
    
    TA2Plot* basic1 = new TA2Plot();
    basic1->AddTimeGate(57181,0,350);
    basic1->LoadData();
    BasicCanvas1=basic1->DrawCanvas("Example plot of run 57181");

    TA2Plot* basic2 = new TA2Plot();
    basic2->AddTimeGate(45000,0,350);
    basic2->LoadData();
    BasicCanvas1=basic2->DrawCanvas("Example plot of run 45000");

    TA2Plot* basic3 = new TA2Plot();
    basic3->AddTimeGate(39993,0,350);
    basic3->LoadData();
    BasicCanvas1=basic3->DrawCanvas("Example plot of run 39993");
    basic3->WriteEventList("test39993take2");


    TA2Plot basicsum = (*basic1 + *basic2);
    basicsum+=*basic3;
    BasicCanvassum=basicsum.DrawCanvas("basicsum");
    basicsum.WriteEventList("1");

//========================================================================================================

    TA2Plot* basic1 = new TA2Plot();
    basic1->AddTimeGate(57181,0,3000);
    basic1->SetLVChannel("HPRO",4);
    basic1->LoadData();
    BasicCanvas1=basic1->DrawCanvas("Example plot of run 57181 with HPRO[4] = BMod inside and ID52[3] = Carlsberg DS ");

    TA2Plot* basic1 = new TA2Plot();
    basic1->AddTimeGate(57181,0,3000);
    basic1->SetLVChannel("HPRO",4);
    basic1->SetLVChannel("HPRO",0);
    basic1->LoadData();
    BasicCanvas1=basic1->DrawCanvas("Example plot of run 57086 with HPRO[4] = BMod inside and HPRO[0] = Carlsberg DS ");

    TA2Plot* basic1 = new TA2Plot();
    basic1->AddTimeGate(39993,0,350);
    basic1->SetLVChannel("DET1",1);
    basic1->LoadData();
    BasicCanvas1=basic1->DrawCanvas("Example plot of run 39993 with HPRO[4] = BMod inside and ID52[3] = Carlsberg DS ");


//========================================================================================================
    
    TA2Plot* basic1 = new TA2Plot();
    basic1->AddTimeGate(45000,0,350);
    basic1->LoadData();
    BasicCanvas1=basic1->DrawCanvas("Example plot of run 45000");
    basic1->WriteEventList("45000WrittentoList");
    basic1->SaveAs("basic1.root");

    TA2Plot* basic2 = new TA2Plot();
    basic2->AddTimeGate(39993,0,350);
    basic2->LoadData();
    BasicCanvas1=basic2->DrawCanvas("Example plot of run 39993");
    basic2->WriteEventList("39993WrittentoList");
    basic2->SaveAs("basic2.root");

    TA2Plot basicsum = (*basic1 + *basic2);
    BasicCanvassum=basicsum.DrawCanvas("basicsum");
    basicsum.WriteEventList("basicsumWrittentoList");
    basicsum.SaveAs("basicsum.root");

//========================================================================================================

    TA2Plot* basic3 = new TA2Plot();
    basic3->AddTimeGate(45000,0,350);
        basic3->AddTimeGate(39993,0,350);
            basic3->LoadData();



}