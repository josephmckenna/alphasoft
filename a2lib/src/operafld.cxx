//
// Name: operafld.cpp
// Description: TWIST OPERA field map
// Author: Amir Ali Ahmadi
// Date: March ,21,2002 
//
// GEANT integration:
// Author: Konstantin Olchanski
// Date: 2002-APR-11
//
// To compile the standalone test code:
//   g++ -O2 -g -Wall -o operafld operafld.cpp -DTEST_OPERAFLD
//
//
// $Id: operafld.C,v 1.1 2005/07/13 21:42:57 olchansk Exp $
//

//ClassInterPol.cpp
//Author: Amir Ali Ahmadi
// Date: March ,21,2002 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

struct field3
{
  float bx;
  float by;
  float bz; 	
};

const int kXmin  = -50;
const int kYmin  = -50;
#ifdef ALPHA1COMP
const int kZmin  = -444;
#else
const int kZmin  = -420;
#endif

const int kXmax  =  50;
const int kYmax  =  50;
#ifdef ALPHA1COMP
const int kZmax  =  444;
#else
const int kZmax  =  420;
#endif

const int kXsize = kXmax - kXmin + 1;
const int kYsize = kYmax - kYmin + 1;
const int kZsize = kZmax - kZmin + 1;

static int gMinX = 0;
static int gMaxX = 0;
static int gMinY = 0;
static int gMaxY = 0;
static int gMinZ = 0;
static int gMaxZ = 0;

static bool gRadialX = false;
static bool gRadialY = false;

static float gFieldZ0 = 0;

static struct field3 map[kXsize][kYsize][kZsize];

//
// Name: operafld_
// Description: calculate field map values
// Return value:
//     0 = success
//     1 = outside of the field map
//     2 = inside a poisoned field map region (iron yoke)
// Arguments:
//     vector[3] - space coordinates in [cm]
//     b[3]      - calculated field vector [kGauss]
//
extern "C" int gufl(const float vector[3],float b[3])
{
  static float lastb[3];

  float scale = 10.0; // from Tesla to kGauss
  float scalelength = 10.0; // from cm to mm

  float x = vector[0]*scalelength;
  float y = vector[1]*scalelength;
  float z = vector[2]*scalelength;
  //float z = -vector[2]*scalelength; // why minus? -- AC

  float phi = 0.;

  if (gRadialY)
    {
      float r = sqrt(x*x + y*y);
      phi = atan2(y,x);
      x = 0;
      y = r;
    }
  else if (gRadialX)
    {
      float r = sqrt(x*x + y*y);
      phi  = atan2(y,x);
      x = r;
      y = 0;
    }

  if ((x<gMinX)||(x>=gMaxX)||(y<gMinY)||(y>=gMaxY)||(z<gMinZ)||(z>=gMaxZ))
    {
      fprintf(stdout,"operafld: Point xyz %f %f %f is outside of the field map.\n",vector[0],vector[1],vector[2]);
      b[0] = lastb[0];
      b[1] = lastb[1];
      b[2] = lastb[2];
      printf("lastb %f %f %f\n",lastb[0],lastb[1],lastb[2]);
      return 1;
    }

  x -= kXmin;
  y -= kYmin;
  z -= kZmin;

  int i = int(x);
  int j = int(y);
  int k = int(z);

  // Round to an even number
  if((i%2)!=0)
  {
    if((x-(float)i)<0)
      i--;
    else
      i++;
  }
  
  if((j%2)!=0)
  {
    if((y-(float)j)<0)
      j--;
    else
      j++;
  }
  
  if((k%2)!=0)
  {
    if((z-(float)k)<0)
      k--;
    else
      k++;
  }


  // for ALPHA just stop here and read the map	
  b[0] = scale*map[i][j][k].bx;
  b[1] = scale*map[i][j][k].by;
  b[2] = scale*map[i][j][k].bz;
//printf("%lf %lf %lf %d %d %d %lf %lf %lf\n",x,y,z,i,j,k,b[0],b[1],b[2]);
//printf("%lf %lf %lf %d %d %d %lf %lf %lf\n",x+kXmin,y+kYmin,z+kZmin,i,j,k,b[0],b[1],b[2]);
  return 0; // I comply to the last comment and I exit here -- AC


  float t = x-i;
  float u = y-j;
  float v = z-k;

  //printf("xyz=%f %f %f, ijk=%d %d %d, tuv=%f %f %f\n",x,y,z,i,j,k,t,u,v);

  //Now I want to calculate the value of the By at the requested Xi, Yi point, but in the lower plane.

  const int mapstep = 2;

  float Bx1L = map[i+0][j+0][k].bx;
  float Bx2L = map[i+mapstep][j+0][k].bx;
  float Bx3L = map[i+mapstep][j+mapstep][k].bx;
  float Bx4L = map[i+0][j+mapstep][k].bx;
    
  float BxiL=(1-t)*(1-u)*Bx1L+t*(1-u)*Bx2L+t*u*Bx3L+(1-t)*u*Bx4L; 

  //Now I want to calculate the value of the By at the requested Xi, Yi point, but in the upper plane.

  float Bx1U = map[i+0][j+0][k+mapstep].bx;
  float Bx2U = map[i+mapstep][j+0][k+mapstep].bx;
  float Bx3U = map[i+mapstep][j+mapstep][k+mapstep].bx;
  float Bx4U = map[i+0][j+1][k+mapstep].bx;

  float BxiU=(1-t)*(1-u)*Bx1U+t*(1-u)*Bx2U+t*u*Bx3U+(1-t)*u*Bx4U; 
 
  // I have the value of the Bxi on the lower and upper XY planes. Now I want to use linear 
  //interpolation in 1D for figuring out the desiered Value of the Bxi at Xi, Yi, Zi.  
	
  b[0] = scale*((1-v)*BxiL+v*BxiU);

  //Now I want to calculate the value of the By at the requested Xi, Yi point, but in the lower plane.

  float By1L = map[i+0][j+0][k].by;
  float By2L = map[i+mapstep][j+0][k].by;
  float By3L = map[i+mapstep][j+mapstep][k].by;
  float By4L = map[i+0][j+mapstep][k].by;
    
  float ByiL=(1-t)*(1-u)*By1L+t*(1-u)*By2L+t*u*By3L+(1-t)*u*By4L; 

  //Now I want to calculate the value of the By at the requested Xi, Yi point, but in the upper plane.

  float By1U = map[i+0][j+0][k+mapstep].by;
  float By2U = map[i+mapstep][j+0][k+mapstep].by;
  float By3U = map[i+mapstep][j+mapstep][k+mapstep].by;
  float By4U = map[i+0][j+mapstep][k+mapstep].by;

  float ByiU=(1-t)*(1-u)*By1U+t*(1-u)*By2U+t*u*By3U+(1-t)*u*By4U; 
 
  // I have the value of the Byi on the lower and upper XY planes. Now I want to use linear 
  //interpolation in 1D for figuring out the desiered Value of the Byi at Xi, Yi, Zi.  
	
  b[1] = scale*((1-v)*ByiL+v*ByiU);

  //printf("by: %f %f %f %f    %f %f %f %f\n",By1L,By2L,By3L,By4L,By1U,By2U,By3U,By4U);

  //Now I want to calculate the value of the Bz at the requested Xi, Yi point, but in the lower plane.

  float Bz1L= map [i+0][j+0][k].bz;
  float Bz2L= map [i+mapstep][j+0][k].bz;
  float Bz3L= map [i+mapstep][j+mapstep][k].bz;
  float Bz4L= map [i+0][j+mapstep][k].bz;
    
  float BziL=(1-t)*(1-u)*Bz1L+t*(1-u)*Bz2L+t*u*Bz3L+(1-t)*u*Bz4L; 

  //Now I want to calculate the value of the Bz at the requested Xi, Yi point, but in the upper plane.

  float Bz1U = map[i+0][j+0][k+mapstep].bz;
  float Bz2U = map[i+mapstep][j+0][k+mapstep].bz;
  float Bz3U = map[i+mapstep][j+mapstep][k+mapstep].bz;
  float Bz4U = map[i+0][j+mapstep][k+mapstep].bz;

  float BziU=(1-t)*(1-u)*Bz1U+t*(1-u)*Bz2U+t*u*Bz3U+(1-t)*u*Bz4U; 

  //printf("bz: %f %f %f %f    %f %f %f %f\n",Bz1L,Bz2L,Bz3L,Bz4L,Bz1U,Bz2U,Bz3U,Bz4U);
 
  // I have the value of the Bzi on the lower and upper XY planes. Now I want to use linear 
  //interpolation in 1D for figuring out the desiered Value of the Bzi at Xi, Yi, Zi.  
	
  b[2] = scale*((1-v)*BziL+v*BziU);

  if (gRadialY)
    {
      float br = b[1];
      b[0] = br*cos(phi);
      b[1] = br*sin(phi);
    }
  else if (gRadialX)
    {
      float br = b[0];
      b[0] = br*cos(phi);
      b[1] = br*sin(phi);
    }

  //printf("operafld: vector %f %f %f, b: %f %f %f\n",vector[0],vector[1],vector[2],b[0],b[1],b[2]);

  //
  // Test for iron region poisoning.
  //
  // The iron region is poisoned with NANs, so b[2] should be NAN.
  // All comparisons with NANs return FALSE, so the above test
  // should return 2 for NAN.
  //
  if (b[2] < 1000000)
    {
      lastb[0] = b[0];
      lastb[1] = b[1];
      lastb[2] = b[2];
      //printf("FIELD: %lf %lf %lf %lf %lf %lf\n",vector[0],vector[1],vector[2],b[0],b[1],b[2]);
      return 0;
    }
  else
    {
      //fprintf(stdout,"operafld: Point xyz %f %f %f is inside the poisoned iron yoke region.\n",vector[0],vector[1],vector[2]);
      // do not return NaNs to caller.
      b[0] = lastb[0];
      b[1] = lastb[1];
      b[2] = lastb[2];
      return 2;
    }
}

static int readFile(FILE*fp)
{
  // read the header
  while (1)
    {
      char line[256];
      if (fgets(line,sizeof(line)-1,fp) == NULL)
	break;
      if (line[strlen(line)-1] == '\n')
	line[strlen(line)-1] = 0;

      if (strncmp(line,"LIMIT X",7) == 0)
	{
	  char* s;
	  gMinX = strtol(line + 8,&s,0);
	  gMaxX = strtol(s,&s,0);
	  continue;
	}

      if (strncmp(line,"LIMIT Y",7) == 0)
	{
	  char* s;
	  gMinY = strtol(line + 8,&s,0);
	  gMaxY = strtol(s,&s,0);
	  continue;
	}

      if (strncmp(line,"LIMIT Z",7) == 0)
	{
	  char* s;
	  gMinZ = strtol(line + 8,&s,0);
	  gMaxZ = strtol(s,&s,0);
	  continue;
	}

      if (strncmp(line,"X Y Z BX BY BZ",14) == 0)
	break;

      fprintf(stderr,"operafld: %s\n",line);
    }

  fprintf(stderr,"operafld: Field map size: x: %d..%d, y: %d..%d, z: %d..%d\n",gMinX,gMaxX,gMinY,gMaxY,gMinZ,gMaxZ);

  // read the field data
  while (1)
    {
      char line[256];

      if (fgets(line,sizeof(line)-1,fp) == NULL)
	break;

      char*s = line;

      int x = strtol(s,&s,0);
      int y = strtol(s,&s,0);
      int z = strtol(s,&s,0);
      float bx = strtod(s,&s);
      float by = strtod(s,&s);
      float bz = strtod(s,&s);

      if ((x<kXmin)||
	  (x>kXmax)||
	  (y<kYmin)||
	  (y>kYmax)||
	  (z<kZmin)||
	  (z>kZmax))
	{
	  fprintf(stderr,"operafld: Invalid field map node: x=%d, y=%d, z=%d\n",x,y,z);
	  return -1;
	}

      //if ((x==0)&&(y==0)&&(z==1))
      //fprintf(stderr,"xyz=%d %d %d, b=%f %f %f, line=[%s]\n",x,y,z,bx,by,bz,line);
      
      map[x-kXmin][y-kYmin][z-kZmin].bx = bx;
      map[x-kXmin][y-kYmin][z-kZmin].by = by;
      map[x-kXmin][y-kYmin][z-kZmin].bz = bz;
    }

  gRadialX = false;
  gRadialY = false;

  if (gMinX == 0 && gMaxX == 0)
    {
      fprintf(stderr,"operafld: Radial map along the Y axis\n");
      gRadialY = true;
      gMaxX = 1;
    }
  else if (gMinY == 0 && gMaxY == 0)
    {
      fprintf(stderr,"operafld: Radial map along the X axis\n");
      gRadialX = true;
      gMaxY = 1;
    }

  if (!gRadialY && gMinX == 0)
    {
      fprintf(stderr,"operafld: Mirroring map: X to -X\n");
      
      for (int x=1; x<=gMaxX; x++)
        for (int y=gMinY; y<=gMaxY; y++)
          for (int z=gMinZ; z<=gMaxZ; z++)
            {
              map[-x-kXmin][y-kYmin][z-kZmin].bx = - map[x-kXmin][y-kYmin][z-kZmin].bx;
              map[-x-kXmin][y-kYmin][z-kZmin].by =   map[x-kXmin][y-kYmin][z-kZmin].by;
              map[-x-kXmin][y-kYmin][z-kZmin].bz =   map[x-kXmin][y-kYmin][z-kZmin].bz;
            }
      gMinX = -gMaxX;
    }
      
  if (!gRadialY && gMaxX == 0)
    {
      fprintf(stderr,"operafld: Mirroring map: -X to X\n");
      
      for (int x=gMinX; x<0; x++)
        for (int y=gMinY; y<=gMaxY; y++)
          for (int z=gMinZ; z<=gMaxZ; z++)
            {
              map[-x-kXmin][y-kYmin][z-kZmin].bx = - map[x-kXmin][y-kYmin][z-kZmin].bx;
              map[-x-kXmin][y-kYmin][z-kZmin].by =   map[x-kXmin][y-kYmin][z-kZmin].by;
              map[-x-kXmin][y-kYmin][z-kZmin].bz =   map[x-kXmin][y-kYmin][z-kZmin].bz;
            }
      gMaxX = -gMinX;
    }
      
  if (!gRadialX && gMinY == 0)
    {
      fprintf(stderr,"operafld: Mirroring map: Y to -Y\n");
      
      for (int y=1; y<=gMaxY; y++)
        for (int x=gMinX; x<=gMaxX; x++)
          for (int z=gMinZ; z<=gMaxZ; z++)
            {
              map[x-kXmin][-y-kYmin][z-kZmin].bx =   map[x-kXmin][y-kYmin][z-kZmin].bx;
              map[x-kXmin][-y-kYmin][z-kZmin].by = - map[x-kXmin][y-kYmin][z-kZmin].by;
              map[x-kXmin][-y-kYmin][z-kZmin].bz =   map[x-kXmin][y-kYmin][z-kZmin].bz;
            }
      gMinY = -gMaxY;
    }

  if (!gRadialX && gMaxY == 0)
    {
      fprintf(stderr,"operafld: Mirroring map: -Y to Y\n");
      
      for (int y=gMinY; y<0; y++)
        for (int x=gMinX; x<=gMaxX; x++)
          for (int z=gMinZ; z<=gMaxZ; z++)
            {
              map[x-kXmin][-y-kYmin][z-kZmin].bx =   map[x-kXmin][y-kYmin][z-kZmin].bx;
              map[x-kXmin][-y-kYmin][z-kZmin].by = - map[x-kXmin][y-kYmin][z-kZmin].by;
              map[x-kXmin][-y-kYmin][z-kZmin].bz =   map[x-kXmin][y-kYmin][z-kZmin].bz;
            }
      gMaxY = -gMinY;
    }

  if (gMinZ == 0)
    {
      fprintf(stderr,"operafld: Mirroring map: Z to -Z\n");
      
      for (int y=gMinY; y<gMaxY; y++)
        for (int x=gMinX; x<=gMaxX; x++)
          for (int z=gMinZ; z<=gMaxZ; z++)
            {
              map[x-kXmin][y-kYmin][-z-kZmin].bx =   map[x-kXmin][y-kYmin][z-kZmin].bx;
              map[x-kXmin][y-kYmin][-z-kZmin].by =   map[x-kXmin][y-kYmin][z-kZmin].by;
              map[x-kXmin][y-kYmin][-z-kZmin].bz =   map[x-kXmin][y-kYmin][z-kZmin].bz;
            }
      gMinZ = -gMaxZ;
    }

  fprintf(stderr,"operafld: Field map size: x: %d..%d, y: %d..%d, z: %d..%d\n",gMinX,gMaxX,gMinY,gMaxY,gMinZ,gMaxZ);

  // remember the field value at x,y,z = 0
  gFieldZ0 = map[0-kXmin][0-kYmin][0-kZmin].bz;
	
  fprintf(stderr,"operafld: Bz at origin is %.3f Tesla.\n",gFieldZ0);

  return 0;
}

//
// Name: operainit_
// Description:  initialize the opera field map
// Return value: returns on success, exit(1) on failure
//
extern "C" int operainit_(char*filename)
{
  for (int i=strlen(filename); i>0; i--)
    if (filename[i-1]!=' ')
      {
	if (filename[i]!=0) filename[i] = 0;
	break;
      }
  char fname[120];
  sprintf(fname,"%s%s%s",getenv("AGRELEASE"),"/aux/",filename);
  FILE* fp = fopen(fname,"r");
  if (fp == 0)
    {
      fprintf(stderr,"operafld: Cannot open field map file [%s], errno %d (%s). Exiting.\n",
	      fname,errno,strerror(errno));
      return 1;
    }
  
  fprintf(stderr,"operafld: Reading field map from %s\n",fname);
  
  if (readFile(fp) != 0)
    {
      fprintf(stderr,"operafld: Cannot read field map from %s. Exiting.\n",fname);
      return 1;
    }
  
  fclose(fp);
  return 0;
}

#if TEST_OPERAFLD

static int testSmoothness()
{
  int worstxx = 0;
  int worstyx = 0;
  int worstzx = 0;
  double worstbx = 0;

  int worstxy = 0;
  int worstyy = 0;
  int worstzy = 0;
  double worstby = 0;

  int worstxz = 0;
  int worstyz = 0;
  int worstzz = 0;
  double worstbz = 0;

  for (int x=0; x<kXsize; x+=2)
    for (int y=0; y<kYsize; y+=2)
      for (int z=1; z<kZsize-1; z+=2)
	{
	  if (0)
	    {
	      double b1x = map[x][y][z-1].bx;
	      double b2x = map[x][y][z+0].bx;
	      double b3x = map[x][y][z+1].bx;
	      
	      //double tolerance = 2*fabs(b3x-b1x);
	      double tolerance = 10.0;
	      double b22x = (b1x + b3x)/2.0;
	      double diff = b2x - b22x;
	      if ((diff>1.0) && (fabs(diff) > tolerance))
		{
		  printf("Bad point %4d %4d %4d: BX is %10.3f %10.3f %10.3f expected %10.3f, diff %10.3f, tolerance %10.3f\n",x+kXmin,y+kYmin,z+kZmin,b1x,b2x,b3x,b22x,diff,tolerance);
		}

	      if (fabs(diff) > fabs(worstbx))
		{
		  worstxx = x;
		  worstyx = y;
		  worstzx = z;
		  worstbx = diff;
		}
	    }

	  if (0)
	    {
	      double b1y = map[x][y][z-1].by;
	      double b2y = map[x][y][z+0].by;
	      double b3y = map[x][y][z+1].by;
	      
	      //double tolerance = 2*fabs(b3y-b1y);
	      double tolerance = 10.0;
	      double b22y = (b1y + b3y)/2.0;
	      double diff = b2y - b22y;
	      if ((diff>1.0) && (fabs(diff) > tolerance))
		{
		  printf("Bad point %4d %4d %4d: BY is %10.3f %10.3f %10.3f expected %10.3f, diff %10.3f, tolerance %10.3f\n",x+kXmin,y+kYmin,z+kZmin,b1y,b2y,b3y,b22y,diff,tolerance);
		}

	      if (fabs(diff) > fabs(worstby))
		{
		  worstxy = x;
		  worstyy = y;
		  worstzy = z;
		  worstby = diff;
		}
	    }

	  if (1)
	    {
	      double b1z = map[x][y][z-2].bz;
	      double b2z = map[x][y][z+0].bz;
	      double b3z = map[x][y][z+2].bz;
	      
	      //double tolerance = 2*fabs(b3z-b1z);
	      ////if (tolerance < 20.0) tolerance = 20;
	      //double b22z = (b1z + b3z)/2.0;
	      //double diff = b2z - b22z;
	      //if ((diff>1.0) && (fabs(diff) > tolerance))
	      //{
	      //  printf("Bad point %4d %4d %4d: BZ is %10.3f %10.3f %10.3f expected %10.3f, diff %10.3f, tolerance %10.3f\n",x+kXmin,y+kYmin,z+kZmin,b1z,b2z,b3z,b22z,diff,tolerance);
	      //}

	      double diff = 2.0*(b2z-b1z)/(b3z-b1z) - 1.0;

	      if (b1z == b3z) diff = 0;

	      if (fabs(diff) > 1.0)
		{
		  printf("Bad point %4d %4d %4d: BZ is %10.3f %10.3f %10.3f diff %10.3f\n",x+kXmin,y+kYmin,z+kZmin,b1z,b2z,b3z,diff);
		}

	      if (fabs(diff) > fabs(worstbz))
		{
		  worstxz = x;
		  worstyz = y;
		  worstzz = z;
		  worstbz = diff;
		}
	    }
	}

  printf("Worst BX spike is %10.3f at %4d %4d %4d\n",worstbx,worstxx+kXmin,worstyx+kYmin,worstzx+kZmin);
  printf("Worst BY spike is %10.3f at %4d %4d %4d\n",worstby,worstxy+kXmin,worstyy+kYmin,worstzy+kZmin);
  printf("Worst BZ spike is %10.3f at %4d %4d %4d\n",worstbz,worstxz+kXmin,worstyz+kYmin,worstzz+kZmin);

  return 0;
}

static void printpoint(const float loc[3])
{
  float b[3];
  float ref = gFieldZ0; //20000.0;
  int iret = operafld_(loc,b);
  printf("Field at %10.4f %10.4f %10.4f is %10.6f %10.6f %10.6f, status %d\n",loc[0],loc[1],loc[2],b[0],b[1],b[2],iret);
}

static void testmap()
{
  float loc[3];

  loc[0] = 0;
  loc[1] = 0;
  loc[2] = 0;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = 0;
  loc[2] = 1.0;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = 0;
  loc[2] = 1.5;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = 0;
  loc[2] = 2.0;
  printpoint(loc);

  loc[0] =   0.516509354;
  loc[1] =  -0.72902399;
  loc[2] = -48.7460480;
  printpoint(loc);

  loc[0] = 0.000394722709;
  loc[1] = -0.0072238734;
  loc[2] = 129.751205;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = 0;
  loc[2] = 1000;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = 0;
  loc[2] = 140.0;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = 10;
  loc[2] = 140.0;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = 18;
  loc[2] = 140.0;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = 19;
  loc[2] = 140.0;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = 20;
  loc[2] = 140.0;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = 25;
  loc[2] = 140.0;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = 2.0;
  loc[2] = 0;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = 1.5;
  loc[2] = 0;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = 1.0;
  loc[2] = 0;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = 0;
  loc[2] = 0;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = -1.0;
  loc[2] = 0;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = -1.5;
  loc[2] = 0;
  printpoint(loc);

  loc[0] = 0;
  loc[1] = -2;
  loc[2] = 0;
  printpoint(loc);

  loc[0] = 1;
  loc[1] = 1;
  loc[2] = 0;
  printpoint(loc);

  loc[0] = -1;
  loc[1] = -1;
  loc[2] = 0;
  printpoint(loc);

  loc[0] = 1;
  loc[1] = -1;
  loc[2] = 0;
  printpoint(loc);
}

int main(int argc,char* argv[])
{
  operainit_(argv[1]);
  //linearZ(-150,-135);
  //linearZ(135,150);
  testSmoothness();
  //testmap();
  return 0;
}
#endif

//end file
