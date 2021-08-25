/*
 * Written by: Daniel Duque
 * Last modified on 05 May 2021
 *
 * This is the declaration of the class derived from FCNBase. This calculates
 * the actual value we want to minimize. In our case this is a Chi-squared
 * between a set of data points and a Line3D object.
 * */

#ifndef LINE3DFCN_HPP
#define LINE3DFCN_HPP

#include"Line3D.hpp"
#include"Minuit2/FCNBase.h"
#include"TSpacePoint.hh"

#include<utility> //std::pair. Just remove this when you replace the ugly
                  // SpacePoint I use
#include<array>//Same as utility, just remove after SpacePoint class is in
#include<vector>

class Line3DFcn : public ROOT::Minuit2::FCNBase{
private:
  //Just a vector of SpacePoints
  //Here I have represented a spacepoint in an ugly way
  //Just replace this for the SpacePoint class after it works
  //first element of the pair are x,y,z
  //second elementof the pair are errors
  std::vector<TSpacePoint> s_points;
  double the_error_def;
public:
  Line3DFcn(const std::vector<
            std::pair<std::array<double,3>,std::array<double,3>>>&);
  //This returns the definition of error you want
  double Up() const;
  //This is what is actually minimized, the value returned by this operator
  //This should, in our case, return the chi-aquared given line parameters
  double operator()(const std::vector<double>&) const; 
};


#endif
