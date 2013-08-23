#include "MapViewer.h"
#include "MapPoint.h"
#include "KeyFrame.h"
#include "LevelHelpers.h"
#include <iomanip>

#include <cvd/gl_helpers.h>

using namespace CVD;
using namespace std;


MapViewer::MapViewer(Map &map, GLWindow2 &glw):
  mMap(map), mGLWindow(glw)
{

  ScaleFactor = 1.0;
  //ScaleFactor = 1.0; //default scale factor was 0.1 (10 cm)
  //  mse3ViewerFromWorld = 
  //SE3<>::exp(makeVector(0,0,2,0,0,0)) * SE3<>::exp(makeVector(0,0,0,0.8 * M_PI,0,0));
  mse3ViewerFromWorld = 
    SE3<>::exp(makeVector(0,0,2,0,0,0)) * SE3<>::exp(makeVector(0,0,0,0.8 * M_PI,0,0));
  mFirstKFFound = false;
}

void MapViewer::DrawMapDots(SE3<> se3CamFromWorld)
{
  SetupFrustum();
  SetupModelView();
  
  int nForMass = 0;
  glColor3f(0,1,1);
   glPointSize(3);
  glBegin(GL_POINTS);
  mv3MassCenter = Zeros;
  for(size_t i=0; i<mMap.vpPoints.size(); i++)
    {
      Vector<3> v3Pos = mMap.vpPoints[i]->v3WorldPos;
      Vector<3> v3PosK = se3CamFromWorld * v3Pos;
      float dist = (float)v3PosK[2];
      float h = ceil(exp(-1*v3PosK[2]/50)*360);
      //float h = -3.6*dist + 360;
      if (h>360)
	h=360.0f;
      
      //h = i%360;

      float r,g,b;
      HtoRGB(&r,&g,&b,h);

      r/=360;
      g/=360;
      b/=360;

      //std::cout << "dist: " << dist << " hue: " << h << endl;

      //      Rgb<float> pcolor(r,g,b);
      //TooN::Vector<3> pcolor(r,g,b);
      glColor3f(r,g,b);
      // glColor(gavLevelColors[mMap.vpPoints[i]->nSourceLevel]);
      // glPointSize(size);

      if(v3Pos * v3Pos < 10000)
	{
	  nForMass++;
	  mv3MassCenter += v3Pos;
	}
      glVertex(v3Pos);
    }
  glEnd();
  mv3MassCenter = mv3MassCenter / (0.1 + nForMass);
}


void MapViewer::DrawGrid()
{
  SetupFrustum();
  SetupModelView();
  glLineWidth(1);
  
  glBegin(GL_LINES);
  
  // Draw a larger grid around the outside..
   double dGridInterval = 0.1;
  // double dGridInterval = 1.0*ScaleFactor;
  
  double dMin = -100.0 * dGridInterval;
  double dMax =  100.0 * dGridInterval;
  
  for(int x=-10;x<=10;x+=1)
    {
      if(x==0)
	glColor3f(1,1,1);
      else
	glColor3f(0.3,0.3,0.3);
      glVertex3d((double)x * 10 * dGridInterval, dMin, 0.0);
      glVertex3d((double)x * 10 * dGridInterval, dMax, 0.0);
    }
  for(int y=-10;y<=10;y+=1)
    {
      if(y==0)
	glColor3f(1,1,1);
      else
	glColor3f(0.3,0.3,0.3);
      glVertex3d(dMin, (double)y * 10 *  dGridInterval, 0.0);
      glVertex3d(dMax, (double)y * 10 * dGridInterval, 0.0);
    }
  
  glEnd();

  glBegin(GL_LINES);
  dMin = -10.0 * dGridInterval;
  dMax =  10.0 * dGridInterval;
  
  for(int x=-10;x<=10;x++)
    {
      if(x==0)
	glColor3f(1,1,1);
      else
	glColor3f(0.5,0.5,0.5);
      
      glVertex3d((double)x * dGridInterval, dMin, 0.0);
      glVertex3d((double)x * dGridInterval, dMax, 0.0);
    }
  for(int y=-10;y<=10;y++)
    {
      if(y==0)
	glColor3f(1,1,1);
      else
	glColor3f(0.5,0.5,0.5);
      glVertex3d(dMin, (double)y * dGridInterval, 0.0);
      glVertex3d(dMax, (double)y * dGridInterval, 0.0);
    }
  
  glColor3f(1,0,0);
  glVertex3d(0,0,0);
  glVertex3d(1,0,0);
  glColor3f(0,1,0);
  glVertex3d(0,0,0);
  glVertex3d(0,1,0);
  glColor3f(1,1,1);
  glVertex3d(0,0,0);
  glVertex3d(0,0,1);
  glEnd();
  
  // glColor3f(0.8,0.8,0.8);
  // glRasterPos3f(1.1,0,0);
  // mGLWindow.PrintString("x");
  // glRasterPos3f(0,1.1,0);
  // mGLWindow.PrintString("y");
  // glRasterPos3f(0,0,1.1);
  // mGLWindow.PrintString("z");
}

void MapViewer::DrawMap(SE3<> se3CamFromWorld, bool follow)
{
  mMessageForUser.str(""); // Wipe the user message clean

  if (!mFirstKFFound)
    {
      //      mse3ViewerFromWorld = SE3<>(makeVector(0,1.5,0,M_PI/16,0,0)) * se3CamFromWorld;// * SE3<>::exp(makeVector(10,0,0,0,0,0));
      mse3ViewerFromWorld = se3CamFromWorld * SE3<>(makeVector(0,-5,0.5,M_PI/16,0,0));
      mFirstKFFound = true;
    }

  if (follow)
    mse3ViewerFromWorld = se3CamFromWorld;
  
  // Update viewer position according to mouse input:
  {
    pair<Vector<6>, Vector<6> > pv6 = mGLWindow.GetMousePoseUpdate();
    SE3<> se3CamFromMC;
    se3CamFromMC.get_translation() = mse3ViewerFromWorld * mv3MassCenter;
    mse3ViewerFromWorld = SE3<>::exp(pv6.first) * 
      se3CamFromMC * SE3<>::exp(pv6.second) * se3CamFromMC.inverse() * mse3ViewerFromWorld;
  }

  mGLWindow.SetupViewport();
  glClearColor(0,0,0,0);
  glClearDepth(1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(1,1,1,1);

  glEnable(GL_DEPTH_TEST);
  DrawGrid();
  DrawMapDots(se3CamFromWorld);
  DrawCamera(se3CamFromWorld);
  for(size_t i=0; i<mMap.vpKeyFrames.size(); i++)
    DrawCamera(mMap.vpKeyFrames[i]->se3CfromW, true);
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  mMessageForUser << " Map: " << mMap.vpPoints.size() << "P, " << mMap.vpKeyFrames.size() << "KF";
  mMessageForUser << setprecision(4);
  mMessageForUser << "   Camera Pos: " << se3CamFromWorld.inverse().get_translation();
}

string MapViewer::GetMessageForUser()
{
  return mMessageForUser.str();
}

void MapViewer::SetupFrustum()
{
  glMatrixMode(GL_PROJECTION);  
  glLoadIdentity();
  double zNear = 0.3*ScaleFactor;
  //glFrustum(-zNear, zNear, 0.75*zNear,-0.75*zNear,zNear,50);
  glFrustum(-zNear, zNear, 0.75*zNear,-0.75*zNear,zNear,200);
  glScalef(1,1,-1);
  return;
};

void MapViewer::SetupModelView(SE3<> se3WorldFromCurrent)
{
  glMatrixMode(GL_MODELVIEW);  
  glLoadIdentity();
  glMultMatrix(mse3ViewerFromWorld * se3WorldFromCurrent);
  return;
};

void MapViewer::DrawCamera(SE3<> se3CfromW, bool bSmall)
{
  
  SetupModelView(se3CfromW.inverse());
  SetupFrustum();
  
  if(bSmall)
    glLineWidth(1);
  else
    glLineWidth(3);
  
  glBegin(GL_LINES);
  glColor3f(1,0,0);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(1.0f*ScaleFactor, 0.0f, 0.0f);
  glColor3f(0,1,0);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 1.0f*ScaleFactor, 0.0f);
  glColor3f(1,1,1);
  glVertex3f(0.0f, 0.0f, 0.0f);
  glVertex3f(0.0f, 0.0f, 1.0f*ScaleFactor);
  glEnd();

  
  if(!bSmall)
  {
	  glLineWidth(1);
	  glColor3f(0.5,0.5,0.5);
	  SetupModelView();
	  Vector<2> v2CamPosXY = se3CfromW.inverse().get_translation().slice<0,2>();
	  glBegin(GL_LINES);
	  glColor3f(1,1,1);
	  glVertex2d(v2CamPosXY[0] - 0.04, v2CamPosXY[1] + 0.04);
	  glVertex2d(v2CamPosXY[0] + 0.04, v2CamPosXY[1] - 0.04);
  	  glVertex2d(v2CamPosXY[0] - 0.04, v2CamPosXY[1] - 0.04);
	  glVertex2d(v2CamPosXY[0] + 0.04, v2CamPosXY[1] + 0.04);
	  glEnd();
  }
  
}

void MapViewer::HtoRGB(float *r, float *g, float *b, float h)
{

  if (h < 36)
    {*r=255; *g=0; *b=0;}
  else if (h < 72)
    {*r=255; *g=154; *b=0;}
  else if (h < 108)
    {*r=205; *g=255; *b=0;}
  else if (h < 144)
    {*r=51; *g=255; *b=0;}
  else if (h < 180)
    { *r=0; *g=255; *b=102;}
  else if (h < 216)
    {*r=0; *g=255; *b=255;}
  else if (h < 252)
    {*r=0; *g=102; *b=255;}
  else if (h < 288)
    {*r=51; *g=0; *b=255;}
  else if (h < 324)
    {*r=205; *g=0; *b=255;}
  else if (h < 360)
    {*r=255; *g=0; *b=154;}
  else 
    {*r=255; *g=255; *b=255;}
 }

// void MapViewer::HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
// {
//   int i;
//   float f, p, q, t;
//   if( s == 0 ) 
//     {
//       // achromatic (grey)
//       *r = *g = *b = v;
//       return;
//     }
//   h /= 60;// sector 0 to 5
//   i = floor( h );
//   f = h - i;// factorial part of h
//   p = v * ( 1 - s );
//   q = v * ( 1 - s * f );
//   t = v * ( 1 - s * ( 1 - f ) );
//   switch( i ) 
//     {
//     case 0:
//       *r = v;
//       *g = t;
//       *b = p;
//       break;
//     case 1:
//       *r = q;
//       *g = v;
//       *b = p;
//       break;
//     case 2:
//       *r = p;
//       *g = v;
//       *b = t;
//       break;
//     case 3:
//       *r = p;
//       *g = q;
//       *b = v;
//       break;
//     case 4:
//       *r = t;
//       *g = p;
//       *b = v;
//       break;
//     default:// case 5:
//       *r = v;
//       *g = p;
//       *b = q;
//       break;
//     }
//}


// /**
//  * Converts an HSV color to an RGB color, according to the algorithm described at http://en.wikipedia.org/wiki/HSL_and_HSV
//  *
//  * @param HSV the color to convert.
//  * @return the RGB representation of the color.
//  */
// void MapViewer::HSVtoRGB(float* r, float* g, float* b, float h, float s, float v)
// {
//   float Min;
//   float Chroma;
//   float Hdash;
//   float X;
  
//   Chroma = s * v;
//   Hdash = h / 60.0;
//   X = Chroma * (1.0 - abs(((int)Hdash % 2) - 1.0));
 
//   if(Hdash < 1.0)
//     {
//       *r = Chroma;
//       *g = X;
//     }
//   else if(Hdash < 2.0)
//     {
//       *r = X;
//       *g = Chroma;
//     }
//   else if(Hdash < 3.0)
//     {
//       *g = Chroma;
//       *b = X;
//     }
//   else if(Hdash < 4.0)
//     {
//       *g= X;
//       *b = Chroma;
//     }
//   else if(Hdash < 5.0)
//     {
//       *r = X;
//       *b = Chroma;
//     }
//   else if(Hdash <= 6.0)
//     {
//       *r = Chroma;
//       *b = X;
//     }
 
//   Min = v - Chroma;
 
//   *r += Min;
//   *g += Min;
//   *b += Min;

// }


