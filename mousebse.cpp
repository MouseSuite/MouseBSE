// Copyright (C) 2025 The Regents of the University of California 
//
// Created by David W. Shattuck, Ph.D.
//
// This file is part of MouseBSE.
//
// MouseBSE is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation, version 2.1.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//

#include <vol3dlib.h>
#include <vol3dsimple.h>
#include <DS/timer.h>
#include <volumeloader.h>
#include "mousebseparser.h"
#include "mousebsetool.h"

template <class T>
std::unique_ptr<Vol3DBase> zeroPad(Vol3D<T> &vIn, int nslices)
{
  auto newVol=std::make_unique<Vol3D<T>>();
  newVol->makeCompatible(vIn);
  newVol->setsize(vIn.cx,vIn.cy,vIn.cz+2*nslices);
  for (int z=0;z<vIn.cz;z++)
    for (int y=0;y<vIn.cy;y++)
      for (int x=0;x<vIn.cx;x++)
        (*newVol)(x,y,z+nslices)=vIn(x,y,z);
  std::cout<<"ZPAD"<<std::endl;
  return newVol;
}

std::unique_ptr<Vol3DBase> zeroPad(Vol3DBase *vIn, int nslices)
{
  switch (vIn->typeID())
  {
    case SILT::Uint8 : return zeroPad(*static_cast< Vol3D<uint8> * >(vIn),nslices);
    case SILT::Sint16 :return  zeroPad(*static_cast< Vol3D<sint16> * >(vIn),nslices);
    case SILT::Uint16 : return zeroPad(*static_cast< Vol3D<uint16> * >(vIn),nslices);
    case SILT::Sint32 : return zeroPad(*static_cast< Vol3D<sint32> * >(vIn),nslices);
    case SILT::Uint32 : return zeroPad(*static_cast< Vol3D<uint32> * >(vIn),nslices);
    case SILT::Float32: return zeroPad(*static_cast< Vol3D<float32> * >(vIn),nslices);
    case SILT::Float64: return zeroPad(*static_cast< Vol3D<float64> * >(vIn),nslices);
    default: break;
  }
  return 0;
}

MouseBSEParser::MouseBSEParser(MouseBSETool &mouseBSE) : ArgParserBase("mousebse"), closingSize(8)
{
  description = "mouse brain surface extractor (mouseBSE)\n"
                "This program performs automated skull and scalp removal on T1-weighted MRI volumes.\n"
                "mouseBSE is adapted from the BrainSuite collection of tools.\n"
                "For more information, please see: https://brainsuite.org";
	copyright = "Copyright (C) 2025 The Regents of the University of California and the University of Southern California";

  usageDisplayWidth=30;
  bind("i",ifname,"<input filename>","input MRI volume",true);
  bind("o",ofname,"<output filename>","output brain-masked MRI volume",false);
  bind("-xmin",xMin,"xplane","zeros out data for x < xplane");
  bind("-ymin",yMin,"yplane","zeros out data for y < yplane");
  bind("-zmin",zMin,"zplane","zeros out data for z < zplane");
  bind("-xmax",xMax,"xplane","zeros out data for x > xplane");
  bind("-ymax",yMax,"yplane","zeros out data for y > yplane");
  bind("-zmax",zMax,"zplane","zeros out data for z > zplane");
  bind("-zpad",zpad,"nslices","zeropad the image by nslices");
  bind("d",mouseBSE.settings.diffusionConstant,"<float>","diffusion constant",false);
  bind("n",mouseBSE.settings.diffusionIterations,"<iterations>","diffusion iterations",false);
  bind("s",mouseBSE.settings.edgeConstant,"<edge sigma>","edge detection constant",false);
  bind("r",mouseBSE.settings.erosionSize,"<size>","radius of erosion/dilation filter",false);
  bind("c",closingSize,"<size>","closing size",false);
  bind("p",mouseBSE.settings.dilateFinalMask,"dilation_radius","dilate final mask by dilation_radius (0==don't dilate)");
//  bindFlag("-trim",mouseBSE.settings.removeBrainstem,"trim brainstem");
  bind("-mask",mfname,"<filename>","save smooth brain mask",false);
  bind("-init",initBrainFilename,"<filename>","initial brain mask",false);
  bind("-select",mouseBSE.settings.selectRegion,"region","select region from connected components");
  bind("-adf",adfFilename,"<filename>","diffusion filter output",false);
  bind("-eroded",erodedMaskFilename,"<filename>","eroded edge map output",false);
  bind("-edge",edgeFilename,"<filename>","edge map output",false);
//  bind("-hires",hiresMask,"<filename>","save detailed brain mask",false);
//  bind("-cortex",cortexFilename,"<filename>","cortex file",false);
  bind("v",mouseBSE.settings.verbosity,"<number>","verbosity level (0=silent)",false);
//  bind("-neckfile",noneckFilename,"<filename>","save image after neck removal",false,true);
  bindFlag("-norotate",Vol3DBase::noRotate,"retain original orientation (default behavior will auto-rotate input NII files to RAS orientation");
  example = progname + " -i input_mri.img -o skull_stripped_mri.img";
}

bool writeByte(std::string ofname, Vol3D<VBit> &vBit)
{
  Vol3D<uint8> vMask;
  vBit.decode(vMask);
  return vMask.write(ofname);
}

void status(std::string s) { std::cout<<s<<std::endl; }

double regionMean(Vol3DBase *vIn, Vol3D<uint8> &vMask);

int main(int argc, char *argv[])
{
  Timer t;
  t.start();
	bool timer=false;
  MouseBSETool mouseBSE;
  mouseBSE.settings.diffusionConstant=50;
  mouseBSE.settings.diffusionIterations=10;


  MouseBSEParser ap(mouseBSE);
  ap.bindFlag("-timer",timer,"show timing",false);
  if (!ap.parseAndValidate(argc,argv)) { return ap.usage(); }

  auto vIn = VolumeLoader::load(ap.ifname);
	if (!vIn) return CommonErrors::cantRead(ap.ifname);
  if (ap.zpad>0)
  {
    auto vPad=zeroPad(vIn.get(),ap.zpad);
    if (vPad)
    {
      std::cout<<"VPAD"<<std::endl;
      vIn=std::move(vPad);
      vIn->write("zpad.nii.gz");
    }
  }
  // initial cropping
    Timer cropTimer;cropTimer.start();
    Vol3D<uint8> vCroppedMask;
    vCroppedMask.makeCompatible(*vIn);
    if (ap.xMin<0) ap.xMin=0;
    if (ap.yMin<0) ap.yMin=0;
    if (ap.zMin<0) ap.zMin=0;
    if (ap.xMax>=vCroppedMask.cx) ap.xMax=vCroppedMask.cx-1;
    if (ap.yMax>=vCroppedMask.cy) ap.yMax=vCroppedMask.cy-1;
    if (ap.zMax>=vCroppedMask.cz) ap.zMax=vCroppedMask.cz-1;
    // should test if cropping necessary!
    vCroppedMask.set(0);
    for (int z=ap.zMin;z<=ap.zMax;z++)
      for (int y=ap.yMin;y<=ap.yMax;y++)
        for (int x=ap.xMin;x<=ap.xMax;x++)
          vCroppedMask(x,y,z)=255;
    vIn->maskWith(vCroppedMask);
    cropTimer.stop();
    if (mouseBSE.settings.verbosity>2) std::cout<<"crop took "<<cropTimer.elapsedSecs()<<std::endl;
    //vCroppedMask.write("crop.mask.nii.gz");


  Vol3DBase *referenceVolume=0;
  if (!vIn)	return CommonErrors::cantRead(ap.ifname);

  int retcode = 0;
  Vol3D<uint8> maskVolume;
  {
    Vol3D<uint8> edgeMap;
    {
      if (mouseBSE.settings.verbosity>1) { std::cout<<"Performing anisotropic diffusion filter"<<std::endl; }
      if (!mouseBSE.initialize(referenceVolume, vIn.get())) return 1;
      if (ap.adfFilename.empty()==false)
      {
        if (referenceVolume->write(ap.adfFilename))
        {
          if (mouseBSE.settings.verbosity>0) std::cout<<"Wrote anisotropic diffusion filtered volume "<<ap.adfFilename<<std::endl;
        }
        else
        {
          retcode |= ::CommonErrors::cantWrite(ap.adfFilename);
        }
      }
      if (mouseBSE.settings.verbosity>1) { std::cout<<"Performing edge detection"<<std::endl; }
      if (!mouseBSE.edgeDetect(maskVolume,referenceVolume,mouseBSE.settings.edgeConstant)) return 1;
      if (ap.edgeFilename.empty()==false)
      {
        mouseBSE.edgemask.decode(edgeMap);
        if (writeByte(ap.edgeFilename,mouseBSE.edgemask))
        {
          if (mouseBSE.settings.verbosity>0) std::cout<<"Wrote edge mask "<<ap.edgeFilename<<std::endl;
        }
        else
        {
          retcode |= ::CommonErrors::cantWrite(ap.edgeFilename);
        }
      }
      if (mouseBSE.settings.verbosity>1) { std::cout<<"Eroding brain"<<std::endl; }
      if (!mouseBSE.erodeBrain(maskVolume,mouseBSE.settings.erosionSize)) { std::cerr<<"error in eroding brain";
        //return 1;
      }
      if (mouseBSE.settings.verbosity>1) { std::cout<<"Eroded brain"<<std::endl; }

      if (ap.erodedMaskFilename.empty()==false)
      {
        mouseBSE.erodedBrain.decode(edgeMap);
        if (writeByte(ap.erodedMaskFilename,mouseBSE.edgemask))
        {
          if (mouseBSE.settings.verbosity>0) std::cout<<"Wrote edge mask "<<ap.erodedMaskFilename<<std::endl;
        }
        else
        {
          retcode |= ::CommonErrors::cantWrite(ap.edgeFilename);
        }
      }
      // now we diverge!

      std::cout<<"cropped region mean is "<<regionMean(referenceVolume,vCroppedMask)<<std::endl;
      mouseBSE.concom(referenceVolume,mouseBSE.erodedBrain);
      {
        Morph32 morphology;
        morphology.setup(mouseBSE.erodedBrain);
        for (int i=0;i<mouseBSE.settings.erosionSize;i++)
        {
          if (i&1) // alternate cube/diamond
          {
            if (mouseBSE.settings.verbosity>1) std::cout<<'C';
            morphology.dilateC(mouseBSE.erodedBrain);
          }
          else
          {
            if (mouseBSE.settings.verbosity>1) std::cout<<'D';
            morphology.dilateR(mouseBSE.erodedBrain);
          }

        }
        if (!ap.initBrainFilename.empty()) writeByte(ap.initBrainFilename,mouseBSE.erodedBrain);
        if (mouseBSE.settings.verbosity>1) std::cout<<"dilating "<<ap.closingSize<<" : ";
        for (int i=0;i<ap.closingSize;i++)
        {
          if (i&1) // alternate cube/diamond
          {
            if (mouseBSE.settings.verbosity>1) std::cout<<'C';
            morphology.dilateC(mouseBSE.erodedBrain);
          }
          else
          {
            if (mouseBSE.settings.verbosity>1) std::cout<<'D';
            morphology.dilateR(mouseBSE.erodedBrain);
          }
        }
        if (mouseBSE.settings.verbosity>1) std::cout<<"\n";
        RunLengthSegmenter rls;
        rls.segmentBG(mouseBSE.erodedBrain);
        if (mouseBSE.settings.verbosity>1) std::cout<<"eroding "<<ap.closingSize<<" : ";;
        for (int i=0;i<ap.closingSize;i++)
        {
          if (i&1) // alternate cube/diamond
          {
            if (mouseBSE.settings.verbosity>1) std::cout<<'C';
            morphology.erodeC(mouseBSE.erodedBrain);
          }
          else
          {
            if (mouseBSE.settings.verbosity>1) std::cout<<'D';
            morphology.erodeR(mouseBSE.erodedBrain);
          }
        }
        if (mouseBSE.settings.verbosity>1) std::cout<<"\n";
        if (mouseBSE.settings.dilateFinalMask>0)
        {
          if (mouseBSE.settings.verbosity>0) std::cout<<"dilating final mask ";
          for (int i=0;i<mouseBSE.settings.erosionSize;i++)
          {
            if (i&1) // alternate cube/diamond
            {
              if (mouseBSE.settings.verbosity>1) std::cout<<'C';
              morphology.dilateC(mouseBSE.erodedBrain);
            }
            else
            {
              if (mouseBSE.settings.verbosity>1) std::cout<<'D';
              morphology.dilateR(mouseBSE.erodedBrain);
            }
          }
          if (mouseBSE.settings.verbosity>0) std::cout<<"\n";
        }
        mouseBSE.erodedBrain.decode(maskVolume);
      }
    }
  }
  if (ap.mfname.empty()==false)
  {
    std::ostringstream description;
    maskVolume.description = description.str();
    if (maskVolume.write(ap.mfname))
    {
      if (mouseBSE.settings.verbosity>0) std::cout<<"Wrote mask file "<<ap.mfname<<std::endl;
    }
    else
      retcode |= ::CommonErrors::cantWrite(ap.mfname);
  }
  if (ap.ofname.empty()==false)
  {
    vIn->maskWith(maskVolume);
    if (vIn->write(ap.ofname))
    {
      if (mouseBSE.settings.verbosity>0) std::cout<<"Wrote skull-stripped MRI volume "<<ap.ofname<<std::endl;
    }
    else
    {
      retcode |= ::CommonErrors::cantWrite(ap.mfname);
    }
  }
  t.stop();
  if ((mouseBSE.settings.verbosity>1)||(timer))
  {
    std::cout<<"BSE took "<<t.elapsed()<<std::endl;
  }
  delete referenceVolume; referenceVolume=0;
  return retcode;
}
