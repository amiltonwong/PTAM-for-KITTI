#include "VideoSource.h"
#include <cvd/image_io.h>
#include <cvd/image.h>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace CVD;
using namespace std;

VideoSource::VideoSource() 
{

  //imgloc = _imgloc;
  
  imgloc = "/home/ddetone/Projects/Multitarget_Tracking/Datasets/KITTI/training/image_02/0003/";
 
  imgindex = 0;
  
  string path = CreateCurrentFileName();

  //cout << path << endl;
  Image<byte> in; //Declares an 8 bit greyscale image
  in = img_load(path);
  mirSize = in.size();
  //cout << mirSize.x << " " << mirSize.y << endl;
  //img_save(in, "new_image.png");

}

string VideoSource::CreateCurrentFileName()
{
  ostringstream ss;
  int filenum = (int)(imgindex/1);
  ss << setw(6) << setfill('0') << filenum;
  string ret = imgloc + ss.str() + ".png"; 
  return (ret);
}

ImageRef VideoSource::Size()
{  
  return mirSize;
};

void VideoSource::GetAndFillFrameBWandRGB(Image<byte> &imBW, Image<Rgb<byte> > &imRGB)
{
  string path = CreateCurrentFileName();
  imBW = img_load(path);
  /*
  ImageRef a,b;
  a.x = 100;
  a.y = 100;
  b.x = 300;
  b.y = 100;
  Image<byte> imBWsmall;
  imBWsmall = imBW.sub_image(a,b);
  */

  imRGB = img_load(path);
  
  /*
  Image<Rgb<byte> > imRGBsmall;
  imRGBsmall = imRGB.sub_image(a,b);
  */

}

void VideoSource::IncreaseImageIndex()
{
  imgindex++;
}

