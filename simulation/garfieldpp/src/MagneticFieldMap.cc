#include "MagneticFieldMap.hh"

#include <fstream>
#include <sstream>

typedef vector<double> record_t;
//-----------------------------------------------------------------------------
// Let's overload the stream input operator to read a list of CSV fields (which a CSV record).
// Remember, a record is a list of doubles separated by commas ','.
std::istream& operator >> ( std::istream& ins, record_t& record )
{
  // make sure that the returned record contains only the stuff we read now
  record.clear();

  // read the entire line into a string (a CSV record is terminated by a newline)
  string line;
  getline( ins, line );

  // now we'll use a stringstream to separate the fields out of the line
  std::stringstream ss( line );
  string field;
  while (getline( ss, field, ',' ))
    {
      // for each field we wish to convert it to a double
      // (since we require that the CSV contains nothing but floating-point values)
      std::stringstream fs( field );
      double f = 0.0;  // (default value is 0.0)
      fs >> f;

      // add the newly-converted field to the end of the record
      record.push_back( f );
    }

  // Now we have read a single line, converted into a list of fields, converted the fields
  // from strings to doubles, and stored the results in the argument record, so
  // we just return the argument stream as required for this kind of input overload function.
  return ins;
}
//-----------------------------------------------------------------------------


MagneticFieldMap::MagneticFieldMap():bmin(0.), bmax(0.), 
				     dx(0.), dy(0.), dz(0.),
				     xmin(0.), xmax(0.), 
				     ymin(0.), ymax(0.), 
				     zmin(0.), zmax(0.),
				     polar(1) // default polar, bc I say so
{
  symmetries.clear();
  BfieldMap.clear();
}

MagneticFieldMap::MagneticFieldMap(bool pol):bmin(0.), bmax(0.), 
					     dx(0.), dy(0.), dz(0.),
					     xmin(0.), xmax(0.), 
					     ymin(0.), ymax(0.), 
					     zmin(0.), zmax(0.)
{
  if(pol) polar=1;
  symmetries.clear();
  BfieldMap.clear();
}

void MagneticFieldMap::MagneticField(double x, double y, double z,
				     double& bx, double& by, double& bz, int& status)
{
  if(BfieldMap.size())
    {
      status = 0;

      if( polar ) 
	{
	  Cartesian2Polar(x,y,x,y);
	  //            std::cout << "Coordinates: " << x << ", " << y << std::endl;
	}

      double phi = y;
      bool rot_sym(false);
      for(set<char>::iterator it = symmetries.begin(); it != symmetries.end(); it++)
	{
	  switch(*it)
	    {
	    case 'x': 
	      if(x < xmin) x = -x; 
	      break;
	    case 'y': 
	      if(y < ymin) y = -y; 
	      break;
	    case 'z': 
	      if(z < zmin) z = -z; 
	      break;
	    case 'p': 
	      rot_sym = true; 
	      break;
	    }
	}

      // a little confusingly, in symmetric polar coordinates, z is mapped to y and z is unused
    if( rot_sym ) 
      { 
	y = z;
	z = 0.;
      }

    if( x < xmin || x > xmax || y < ymin || y > ymax || z < zmin || z > zmax )
      {
	std::cerr << "MagneticFieldMap::MagneticField Requested field outside of magnetic field map." << std::endl;
	status = -47;
	return;
      }

    double xshifted = x - xmin;
    double yshifted = y - ymin;
    double zshifted = z - zmin;

    double ix_d = xshifted/dx;
    double iy_d = yshifted/dy;
    double iz_d = zshifted/dz;

    // a little confusingly, in symmetric polar coordinates, z is mapped to y and z is unused
    if( rot_sym ) 
      iz_d = 0;

    int ix0 = ix_d;
    int iy0 = iy_d;
    int iz0 = iz_d;

    int ix1(ix0), iy1(iy0), iz1(iz0);

    if( ix0 != ix_d ) ix1++;
    if( iy0 != iy_d ) iy1++;
    if( iz0 != iz_d ) iz1++;

    double ddx = ix_d - ix0;
    double ddy = iy_d - iy0;
    double ddz = iz_d - iz0;

    vector<double> field;

    for(unsigned int i = 0; i < BfieldMap[0][0][0].size(); i++)
      {
	double field00 = (1.-ddx)*BfieldMap[ix0][iy0][iz0][i] + ddx*BfieldMap[ix1][iy0][iz0][i];
	double field01 = (1.-ddx)*BfieldMap[ix0][iy0][iz1][i] + ddx*BfieldMap[ix1][iy0][iz1][i];
	double field10 = (1.-ddx)*BfieldMap[ix0][iy1][iz0][i] + ddx*BfieldMap[ix1][iy1][iz0][i];
	double field11 = (1.-ddx)*BfieldMap[ix0][iy1][iz1][i] + ddx*BfieldMap[ix1][iy1][iz1][i];
	
	double field0 = (1.-ddy)*field00 + ddy*field10;
	double field1 = (1.-ddy)*field01 + ddy*field11;
	
	field.push_back((1.-ddz)*field0 + ddz*field1);
      }

    if( field.size() == 3 )
      {
	bx = field[0];
	by = field[1];
	bz = field[2];
      } 
    else if(field.size() == 2) // shuffling components around, 
                               // because (r,phi,z) is more common than (r,z,phi)
      { 
	bx = field[0];
	by = 0.;
	bz = field[1];
      } 
    else 
      std::cerr << "MagneticFieldMap::MagneticField Unexpected number of field components (" << field.size() << ")" << std::endl;


    if(polar) 
      {
	double br(bx), bphi(by);
	bx = br*cos(M_PI*phi/180.) - bphi*sin(M_PI*phi/180.);
	by = br*sin(M_PI*phi/180.) + bphi*cos(M_PI*phi/180.);
      }
    } 
  else 
    { 
      status = -2;
    }
}

bool MagneticFieldMap::ReadMap(const string filename, float scale)
{
  std::ifstream f(filename.c_str());
  if(f.is_open())
    {
      set<double> magnitudes;
      xmin = ymin = zmin = 1.e9;
      xmax = ymax = zmax =-1.e9;
      dx = dy = dz = 0.;
      int lnr(0);
      double xlast(1.e99), ylast(1.e99), zlast(1.e99);
      vector<vector<double> > zvals;
      vector<vector<vector<double> > > yvals;
      bool rot_sym(false);
      bool first(true);
      unsigned int nentries(0);
      
      while(f.good())
	{
	  lnr++;
	  if(f.peek() == '#' || f.peek() == 'r')
	    {
	      string line;
	      getline(f, line);
	      if(line.find("#Symmetries:") == 0)
		{
		  line.erase(0,12);
		  std::istringstream iss(line);
		  do 
		    {
		      char sym;
		      iss >> sym;
		      switch(sym){
		      case 'r': 
			sym = 'p';
		      case 'x':
		      case 'y':
		      case 'z':
		      case 'p': 
			symmetries.insert(sym); break;
		      default: 
			std::cerr << "MagneticFieldMap::ReadMap I don't understand symmetry parameter '" << sym << "'" << std::endl;
		      }
		    } while(iss.good());
		}
	      continue;
	    }// f.peek
	  
	  record_t entries;
	  f >> entries;
	  if( first )
	    {
	      nentries = entries.size();
	      if(polar)
		{
		  if(nentries == 4 || nentries == 5)// rotational symmetry can 
		    // still have a B_phi component
		    { 
		      rot_sym = true;
		      polar = 2;
		    } 
		  else if(nentries == 6) 
		    rot_sym = false;
		  else break;
		} 
	      else if(nentries < 6) 
		break;
	    }// first entry 
	  else if(entries.size() == 0) 
	    {
	      break;
	    } 
	  else if(entries.size() != nentries) 
	    {
	      std::cerr << "MagneticFieldMap::ReadMap item number mismatch in line " << lnr 
		   << ": " << entries.size() 
		   << " instead of " << nentries << " items" << std::endl;
	    }
	  first = false;
	
	  double ddx = entries[0] - xlast;
	  if( dx )
	    {
	      if(ddx != dx && ddx * dx > 0.)
		{
		  std::cerr << "MagneticFieldMap::ReadMap  x Step size must not vary within file, but varied between " << dx 
		       << " and " << ddx << std::endl;
		  std::cerr << "                           @ Line " << lnr 
		       <<": " << entries[0] << " - " << xlast << std::endl;
		  return false;
		}
	    } 
	  else if( xlast < 1.e90 )
	    {
	      dx = ddx;
	    }
	
	  double ddy = entries[1] - ylast;
	  if( dy )
	    {
	      if(ddy != dy && ddy * dy > 0.)
		{
		  std::cerr << "MagneticFieldMap::ReadMap  y Step size must not vary within file, but varied between " << dy 
		       << " and " << ddy << std::endl;
		  std::cerr << "                           @ Line " << lnr 
		       <<": " << entries[1] << " - " << ylast << std::endl;
		  return false;
		}
	    } 
	  else if( ylast < 1.e90 )
	    {
	      dy = ddy;
	    }

	  if( !rot_sym )
	    {
	      double ddz = entries[2] - zlast;
	      if( dz )
		{
		  if(ddz != dz && ddz * dz > 0.)
		    {
		      std::cerr << "MagneticFieldMap::ReadMap   z Step size must not vary within file, but varied between " << dz 
			   << " and " << ddz << std::endl;
		      std::cerr << "                            @ Line " << lnr <<": " 
			   << entries[2] << " - " << zlast << std::endl;
		      return false;
		    }
		} 
	      else if(zlast < 1.e90)
		{
		  dz = ddz;
		}

	    }
	  if(entries[0] < xmin) xmin = entries[0];
	  if(entries[1] < ymin) ymin = entries[1];
	  if(entries[0] > xmax) xmax = entries[0];
	  if(entries[1] > ymax) ymax = entries[1];
	  if( !rot_sym ) // in the case of rotational symmetry, 
	                 // z variables are not used, 
                         // actual z numbers are in y variables
	    {
	      if(entries[2] < zmin) zmin = entries[2];
	      if(entries[2] > zmax) zmax = entries[2];
	    }
      
	  vector<double> field;
	  int dim = 3;
	  if( rot_sym ) dim = 2;

	  double mag(0.);
	  for(unsigned int i = dim; i < entries.size(); i++)
	    {
	      field.push_back(entries[i]*scale);
	      mag += entries[i]*scale * entries[i]*scale;
	    }
	  magnitudes.insert(sqrt(mag));

	  if( ylast < 1.e90 && ddy ) // collect field values for y,z plane (or phi,z)
	    { 
	      yvals.push_back(zvals);
	      zvals.clear();
	    }
	  if( xlast < 1.e90 && ddx )  // collect planes into full volume
	    {
	      BfieldMap.push_back(yvals);
	      yvals.clear();
	    }

	  zvals.push_back(field);  // collect field values along z

	  xlast = entries[0];
	  ylast = entries[1];
      
	  if( !rot_sym ) zlast = entries[2];
      
	}// f.good()
    
      if( zvals.size() )  // flush remaining collected fields into map
	{ 
	  yvals.push_back(zvals);
	  BfieldMap.push_back(yvals);
	}
    
      if( rot_sym ) // as said above, this z doesn't mean z for rotational symmetry
	{   
	  dz = zmin = zmax = 0.;
	  symmetries.insert('p');
	}
      std::cout << "MagneticFieldMap::ReadMap FILE " << filename << " read." << std::endl;

      if( dx < 0. || dy < 0. || dz < 0. )
	{
	  std::cerr << "MagneticFieldMap::ReadMap Fieldmap not sorted ascending, not supported." << std::endl;
	  BfieldMap.clear();
	  return false;
	}

      if( polar )
	{
	  std::cout << "MagneticFieldMap::ReadMap  r goes from " << xmin << " to " << xmax 
	       << " in steps of " << dx << std::endl;
	  if( rot_sym )
	    {
	      std::cout << "MagneticFieldMap::ReadMap z goes from " << ymin << " to " << ymax 
		   << " in steps of " << dy << std::endl;
	      std::cout << "                          rotationally symmetric" << std::endl;
	    } 
	  else 
	    {
	      std::cout << "MagneticFieldMap::ReadMap  phi goes from " << ymin << " to " << ymax 
		   << " in steps of " << dy << std::endl;
	      std::cout << "                           z goes from " << zmin << " to " << zmax 
		   << " in steps of " << dz << std::endl;
	    }
	} 
      else 
	{
	  std::cout << "MagneticFieldMap::ReadMap" << std::endl;
	  std::cout << "\tx goes from " << xmin << " to " << xmax << " in steps of " << dx << std::endl;
	  std::cout << "\ty goes from " << ymin << " to " << ymax << " in steps of " << dy << std::endl;
	  std::cout << "\tz goes from " << zmin << " to " << zmax << " in steps of " << dz << std::endl;
	}

      bmin = *magnitudes.begin();
      bmax = *magnitudes.rbegin();

      f.close();

      if( symmetries.size() && BfieldMap.size() ) 
	return CheckSymmetries();
      else 
	return true;

    }// f.is_open() 
  else 
    {
      std::cerr << "MagneticFieldMap::ReadMap File " << filename << " not found." << std::endl;
      return false;
    }
  f.close();
  return false;
}
