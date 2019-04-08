#include "ComponentBmap.hh"

using namespace std;

namespace Garfield {
  ComponentBmap::ComponentBmap(bool pol):Bmap(pol){
        m_className = "ComponentBmap";
        
        cout << m_className << " created." << endl;
    }

    bool ComponentBmap::SetSymmetries(string sym){
      return Bmap.SetSymmetries(sym);
    }

    bool ComponentBmap::CheckSymmetries(){
      return Bmap.CheckSymmetries();
    }

    void ComponentBmap::GetMapBoundaries(double &x0, double &x1, double &y0, double &y1, double &z0, double &z1){
      Bmap.GetMapBoundaries(x0,x1,y0, y1,z0,z1);
    }

    void ComponentBmap::MagneticField(double x, double y, double z,
                                      double& bx, double& by, double& bz, int& status){
      Bmap.MagneticField(x,y,z,
			 bx, by, bz, 
			 status);
      if( status >= 0 ) return;
      else 
	{  // if no field map is present, revert to the standard class function
	  ComponentAnalyticField::MagneticField(x, y, z, bx, by, bz, status);
	}
    }

    bool ComponentBmap::ReadMagneticFieldMap(const std::string filename, float scale){
      return Bmap.ReadMap(filename, scale);
    }
}

/* emacs
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 3
 * indent-tabs-mode: nil
 * End:
 */
