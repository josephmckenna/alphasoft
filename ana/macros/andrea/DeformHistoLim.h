
double max_occ=-1111.;
double min_occ=-1111.;
double max_amp=-1111.;
double min_amp=-1111.;
double max_ofl=-1111.;
double min_ofl=-1111.;


void SetHistoLimits( int run ) {
  if(0)  {  /*donothing*/  }
  else if( run == 904512 )
    {
      cout<<"Setting histo for combined run "<<run<<endl;
      //max_occ=13000.;
      min_occ=5000.;
      max_amp=1300.;
      min_amp=600.;
      max_ofl=900.;
      min_ofl=1.;
    }
  else if( run == 904502 )
    {
      cout<<"Setting histo for combined run "<<run<<endl;
      //max_occ=13000.;
      min_occ=9000.;
      max_amp=1600.;
      min_amp=900.;
      //max_ofl=500.;
      min_ofl=1.;
    }
  else if( run == 904503 )
    {
      cout<<"Setting histo limits for run "<<run<<endl;
      max_occ=13000.;
      min_occ=3000.;
      max_amp=1600.;
      min_amp=900.;
      //max_ofl=500.;
      min_ofl=1.;
    }
  else if( run == 904473 )
    {
      cout<<"Setting histo limits for combined run "<<run<<endl;
      max_amp=2000.;
      min_amp=1000.;
      min_ofl=100.;
      min_occ=6000.;
    }
  else if( run == 904353 )
    {
      cout<<"Setting histo limits for combined run "<<run<<endl;
      max_amp=1730.;
      min_amp=200.;
      //max_ofl=600.;
      min_ofl=1.;
    }
  else if( run == 904275 )
    {
      cout<<"Setting histo limits for combined run "<<run<<endl;
      max_occ=9000.;
      max_amp=1500.;
      min_amp=10.;
      max_ofl=600.;
      min_ofl=1.;
    }
  else if( run == 904214 )
    {
      cout<<"Setting histo limits for run "<<run<<endl;
      max_occ=3000.;
      min_amp=10.;
      max_ofl=500.;
      min_ofl=1.;
    }
 else if( run == 904148 )
    {
      cout<<"Setting histo limits for run "<<run<<endl;
      max_occ=4000.;
      min_occ=-1111.;
      max_amp=1000.;
      min_amp=100.;
      max_ofl=100.;
      min_ofl=1.;
    }
 else if( run == 904147 )
    {
      cout<<"Setting histo limits for run "<<run<<endl;
      max_occ=4000.;
      min_occ=-1111.;
      max_amp=1500.;
      min_amp=100.;
      max_ofl=100.;
      min_ofl=1.;
    }
  else if( run == 904139 )
    {
      cout<<"Setting histo limits for run "<<run<<endl;
      max_occ=4000.;
      min_occ=-1111.;
      max_amp=2200.;
      min_amp=100.;
      max_ofl=600.;
      min_ofl=1.;
    }
  else if( run == 904136 )
    {
      cout<<"Setting histo limits for run "<<run<<endl;
      max_occ=3000.;
      min_occ=-1111.;
      max_amp=1500.;
      min_amp=100.;
      max_ofl=150.;
      min_ofl=1.;
    }
  else if( run == 904135 )
    {
      cout<<"Setting histo limits for run "<<run<<endl;
      max_occ=3000.;
      min_occ=-1111.;
      max_amp=1500.;
      min_amp=100.;
      max_ofl=150.;
      min_ofl=1.;
    }
  else if( run == 904134 )
    {
      cout<<"Setting histo limits for run "<<run<<endl;
      max_occ=8000.;
      min_occ=-1111.;
      max_amp=1800.;
      min_amp=100.;
      max_ofl=150.;
      min_ofl=1.;
    }
  else if( run == 904133 )
    {
      cout<<"Setting histo limits for run "<<run<<endl;
      max_occ=4000.;
      min_occ=-1111.;
      max_amp=2200.;
      min_amp=100.;
      max_ofl=500.;
      min_ofl=1.;
    }
  else if( run == 904132 )
    {
      cout<<"Setting histo limits for run "<<run<<endl;
      max_occ=4000.;
      min_occ=-1111.;
      max_amp=2200.;
      min_amp=100.;
      max_ofl=500.;
      min_ofl=1.;
    }
else if( run == 904014 )
    {
      cout<<"Setting histo limits for run "<<run<<endl;
      max_amp=1800.;
      min_amp=1100.;
      max_ofl=400.;
      min_ofl=1.;
    }
  else if( run == 903916 )
    {
      cout<<"Setting histo limits for combined run "<<run<<endl;
      max_occ=15000.;
      min_occ=8000.;
      max_amp=1800;
      min_amp=1100.;
      max_ofl=3200.;
      min_ofl=1.;
    }
  else if( run == 3873 )
    {
      cout<<"Setting histo limits for run "<<run<<endl;
      min_amp=100.;
      min_ofl=10.;
    }
  else if( run == 3879 )
    {
      cout<<"Setting histo limits for run "<<run<<endl;
      max_occ=4000.;      
      min_amp=100.;
      min_ofl=10.;
    }
}
