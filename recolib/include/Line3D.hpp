/*
 * Written by: Daniel Duque
 * Last modified on 05 May 2021
 *
 * Declaration of the class Line3D. This simply defines what our model is i.e.
 * a straight line in 3D, which needs 6 free parameters to be fully defined
 * */

#ifndef LINE3D_HPP
#define LINE3D_HPP

class Line3D{
private:
  //This defines a straight line that passes through the point (x0,y0,z0)
  //and has a direction parallel to the vector dir_x i + dir_y j + dir_z k
  double x0;
  double y0;
  double z0;
  double dir_x;
  double dir_y;
  double dir_z;
public:
  Line3D(double a_x, double a_y, double a_z,
         double a_dir_x, double a_dir_y, double a_dir_z);
  //This calculates the projection of our line into a plane that contains
  //the point (a_x, a_y, a_z) and has a normal vector with 
  //direction a_dir_x i + a_dir_y j + a_dir_z k
  //If the returned line has direction 0,0,0 it means it is just a point
  Line3D GetProjection(double a_x, double a_y, double a_z,
                       double a_dir_x, double a_dir_y, double a_dir_z) const;
  double GetAnX() const;
  double GetAY() const;
  double GetAZ() const;
  double GetADirX() const;
  double GetADirY() const;
  double GetADirZ() const;
  //Get one of the coordinates
  //0 is x, 1 is y, and 2 is z
  double GetCoord(int i) const;
  double GetDir(int i) const;
};
#endif
