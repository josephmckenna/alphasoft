
double max_occ=-1111.;
double min_occ=-1111.;
double max_amp=-1111.;
double min_amp=-1111.;
double max_ofl=-1111.;
double min_ofl=-1111.;


void SetHistoLimits( int run ) {
  if(0)  {
    // donothing 
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
}
