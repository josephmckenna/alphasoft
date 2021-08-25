/*Written by: Daniel Duque
 * Last modified on 05 May 2021
 *
 * This is the definition of our FCN class, which derives from FCNBase.
 * Here we define the actual value to be minimized (whatever is returned from the operator()).
 * In our case, this is going to be the chi-squared
 * */

#include"Line3DFcn.hpp"
#include<cassert>
#include<iostream>

#include<limits> //infinity chi2 contribution

Line3DFcn::Line3DFcn(const
                     std::vector<
                     std::pair<
                     std::array<double,3>,std::array<double,3>>>& points) :
                     s_points(points), the_error_def(1.){
}
double Line3DFcn::Up() const{
  return the_error_def;
}
//This should return the value of chi-squared to be minimized
//These parameters are, in this order: x0,y0,z0,dir_x,dir_y,dir_z
double Line3DFcn::operator()(const std::vector<double>& pars) const{
  assert(pars.size() == 6);
  Line3D  the_line(pars[0],pars[1],pars[2],pars[3],pars[4],pars[5]);  
  //Now go through each spacepoint, and add their contribution to chi2
  //Each point contributes 2 times per projection (3 projections)
  //i.e. 6 contributions to chi2 per point
  double chi2{0};
  for(unsigned int i = 0; i < s_points.size(); ++i){
    //std::cout << "-----------\n---------\nNEW SPACEPOINT\n";
    //contribution for each projection
    for(int plane = 0; plane < 3; ++plane){
      //std::cout << "-----------\n---------\nNEW PROJECTION\n";
      std::array<double,3> nml {0,0,0};
      nml[plane] = 1;
      //Get the line projection to that plane
      Line3D proj = the_line.GetProjection(0,0,0,nml[0],nml[1],nml[2]);
      //Get ehe 2 contributions to chi2 from this projection
      for(int ax = 0; ax < 3; ++ax){
        if(ax != plane){
          //Do the maths to find the contributions.
          //std::cout << "-----------\n---------\nNEW CONTRIBUTION\n";
          double pax = s_points[i].GetCoord(ax);
          double uax = proj.GetDir(ax);
          double rax = proj.GetCoord(ax);
          double sgax = s_points[i].GetErr(ax);
          for(int o_ax = 0; o_ax < 3; ++o_ax){
            if(o_ax != ax && o_ax != plane){
              double po_ax = s_points[i].GetCoord(o_ax);
              double uo_ax = proj.GetDir(o_ax);
              double ro_ax = proj.GetCoord(o_ax); 
              if(uo_ax == 0){
                chi2 += std::numeric_limits<double>::infinity();
              }else{
                //Now just compute the contribution and add it.
                double contr = (pax - (uax/uo_ax)*(po_ax-ro_ax)+ rax) / sgax;
                chi2 += contr * contr;
              }
              /*
              std::cout << "----------------\n"
                        << "Currently testing line through ("
                        << pars[0] << ',' << pars[1] << ',' << pars[2]
                        << "), with direction vector <"
                        << pars[3] << ',' << pars[4] << ',' << pars[5]
                        << ">\nCurrent projection through ("
                        << proj.GetAnX() << ',' << proj.GetAY() << ',' << proj.GetAZ()
                        << "), with direction vector <"
                        << proj.GetADirX() << ',' << proj.GetADirY() << ',' << proj.GetADirZ()
                        << ">\nPi=" << pax 
                        << "\nPj=" << po_ax << "\nsigi=" << sgax
                        << "\nRi=" << rax << "\nRj=" << ro_ax
                        << "\nUi=" << uax << "\nUj=" << uo_ax
                        << "\nChi=" << chi2;
              std::cin.get();
              */
            }
          }  
        }
      }
    }
  }
  //std::cout << "\n------\n------\nchi2=" << chi2 << "\n------\n------";
  return chi2;
}
