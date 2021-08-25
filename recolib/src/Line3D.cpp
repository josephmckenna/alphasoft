/*
 * Written by: Daniel Duque
 * Last modified on 05 May 2021
 *
 * These are the definitions for our Line3D class
 * */

#include"Line3D.hpp"

Line3D::Line3D(double a_x, double a_y, double a_z,
               double a_dir_x, double a_dir_y, double a_dir_z) :
               x0(a_x), y0(a_y), z0(a_z),
               dir_x(a_dir_x), dir_y(a_dir_y), dir_z(a_dir_z) {
}
//Find the projection of our line into a plane that contains
//the point (a_x, a_y, a_z) and has a normal vector with direction
//a_dir_x i + a_dir_y j + a_dir_z k
Line3D Line3D::GetProjection(double a_x, double a_y, double a_z,
                             double a_dir_x, double a_dir_y, double a_dir_z)
                             const{
  //To find this line, we need a point on the line, and a direction
  //We can use the interception between our line and the plane as the point 
  //on the projection we want to find

  //First, check for the dot product between the direction of our line
  //and the direction normal to the plane
  double u_dot_n = dir_x * a_dir_x + dir_y * a_dir_y + dir_z * a_dir_z;
  //If these are perpendicular
  if(u_dot_n == 0){
    //Two possibilities. One intercept, or infinite intercepts
    //If a vector from the line to the plane is always perpendicular
    //to the normal of the plane, then the line is in the plane (infinite)
    double l_to_p = (a_x-x0)*a_dir_x+(a_y-y0)*a_dir_y+(a_z-z0)*a_dir_z;
    //Line is its own projection. Inifinite intercepts
    if(l_to_p == 0){
      return *this;
    }
    //There is no intercept
    else{
      //We need to find a point on the plane that projects to the line
      //This is the intercept of the plane and a line that
      // goes through (x0,y0,z0) with direction of the normal to the plane
      double param = l_to_p/(a_dir_x*a_dir_x+a_dir_y*a_dir_y+a_dir_z*a_dir_z);
      return Line3D(x0 + param * a_dir_x,
                    y0 + param * a_dir_y,
                    z0 + param * a_dir_z,
                    dir_x,
                    dir_y,
                    dir_z);
    }
  }
  //If they are not perpendicular i.e. only one intercept
  else{
    //Find the intercept, and later on we will find the direction
    double l_to_p = (a_x-x0)*a_dir_x+(a_y-y0)*a_dir_y+(a_z-z0)*a_dir_z;
    double param = l_to_p / u_dot_n;
    //With this param we now have the intercept
    //Now we only need to find the direction of this projection.
    //This projection is in the plane we have
    //This projection is also in a plane perpendicular to our plane
    //that goes through our line (call it plane 2)
    //We can find the normal to plane 2 as the cross product between
    //our line and the plane we have.
    //The direction of the projection will then be the cross product between
    //normal to our plane and normal to plane 2
    //Lets call the normal to plane 2 dir2_x, dir2_y, dir2_z
    double dir2_x = dir_y*a_dir_z - dir_z*a_dir_y;
    double dir2_y = dir_z*a_dir_x - dir_x*a_dir_z;
    double dir2_z = dir_x*a_dir_y - dir_y*a_dir_x;
    
    return Line3D(x0 + param * dir_x,
                  y0 + param * dir_y,
                  z0 + param * dir_z,
                  a_dir_y*dir2_z - a_dir_z*dir2_y,
                  a_dir_z*dir2_x - a_dir_x*dir2_z,
                  a_dir_x*dir2_y - a_dir_y*dir2_x);
  }
}
double Line3D::GetAnX() const{
  return x0;
}
double Line3D::GetAY() const{
  return y0;
}
double Line3D::GetAZ() const{
  return z0;
}
double Line3D::GetADirX() const{
  return dir_x;
}
double Line3D::GetADirY() const{
  return dir_y;
}
double Line3D::GetADirZ() const{
  return dir_z;
}
double Line3D::GetCoord(int i) const{
  if(i == 0){
    return x0;
  }
  else if(i == 1){
    return y0;
  }
  else if(i == 2){
    return z0;
  }
  else{
    return 9999;
  }
}
double Line3D::GetDir(int i) const{
  if(i == 0){
    return dir_x;
  }
  else if(i == 1){
    return dir_y;
  }
  else if(i == 2){
    return dir_z;
  }
  else{
    return 9999;
  }
}
