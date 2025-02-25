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

#include <mousebsetool.h>
#include <DS/timer.h>
#include <vol3d.h>
#include <marrhildrethedgedetector.h>
#include <volumescaler.h>
#include <vol3dops.h>
#include "anisotropicdiffusionfilter.h"

MouseBSETool::Settings::Settings() :
  diffusionIterations(3), diffusionConstant(25),
  edgeConstant(0.64f), erosionSize(1), removeBrainstem(false),
  dilateFinalMask(false), verbosity(1), selectRegion(-1)
{
}

MouseBSETool::MouseBSETool(): bseState(ADFilter), saveCortex(false)
{
}


template <class T>
double regionMean(Vol3D<T> &vIn, Vol3D<uint8> &vMask)
{
  if (vIn.isCompatible(vMask)==false) return 0;
  const int ds=vIn.size();
  if (ds<=0) return 0;
  double sum=0;
  double den=0;
  for (int i=0;i<ds;i++)
    if (vMask[i]) { sum += vIn[i]; den++; }
  return sum/den;
}

double regionMean(Vol3DBase *vIn, Vol3D<uint8> &vMask)
{
  switch (vIn->typeID())
  {
    case SILT::Uint8 : return regionMean(*static_cast< Vol3D<uint8> * >(vIn),vMask);
    case SILT::Sint16 : return regionMean(*static_cast< Vol3D<sint16> * >(vIn),vMask);
    case SILT::Uint16 : return regionMean(*static_cast< Vol3D<uint16> * >(vIn),vMask);
    case SILT::Sint32 : return regionMean(*static_cast< Vol3D<sint32> * >(vIn),vMask);
    case SILT::Uint32 : return regionMean(*static_cast< Vol3D<uint32> * >(vIn),vMask);
    case SILT::Float32: return regionMean(*static_cast< Vol3D<float32> * >(vIn),vMask);
    case SILT::Float64: return regionMean(*static_cast< Vol3D<float64> * >(vIn),vMask);
    default: break;
  }
  return 0;
}

bool MouseBSETool::concom(Vol3DBase *vIn, Vol3D<VBit> &vBitMask, int threshold)
{
  RunLengthSegmenter rls;
//  Vol3D<VBit> vBitMask;
//  vBitMask.encode(vMask);
  rls.segmentFG(vBitMask);
  Vol3D<uint8> vMask;
  Vol3D<VBit> vResult;
  DSPoint center(vIn->cx/2.0f,vIn->cy/2.0f,vIn->cz/2.0f);
  std::cout<<"voxel center is "<<center.x<<','<<center.y<<','<<center.z<<std::endl;
  bool selected=false;

  std::cout<<"region table\n";
  std::cout<<"#\tvoxels\tcent_x\tcent_y\tcent_z\tmean\td_cent\n";
  for (int i=0;i<rls.nRegions();i++)
  {
    if (rls.regionCount(i)>threshold)
    {
      for (auto &region : rls.regionInfo) region.selected=0;
      auto &currentRegion(rls.regionInfo[i]);
      DSPoint centroid(currentRegion.cx,currentRegion.cy,currentRegion.cz);
      std::cout<<i<<'\t'<<rls.regionCount(i)<<'\t'<<currentRegion.cx<<"\t"<<currentRegion.cy<<"\t"<<currentRegion.cz;
      currentRegion.selected=1;
      rls.label32FG(vBitMask);
      vBitMask.decode(vMask);
      auto mean=regionMean(vIn,vMask);
      auto dist=(centroid-center).mag();
      std::cout<<'\t'<<mean<<'\t'<<dist;
      if (mean==0) { std::cout<<" [rejected]\n"; continue; }
      if (settings.selectRegion>0)
      {
        if (settings.selectRegion==i)
        {
          selected=true;
          std::cout<<" [override]\n";
          vResult.copy(vBitMask);
          continue;
        }
      }
      if (!selected)
      {
        selected=true;
        std::cout<<" [selected]\n";
        vResult.copy(vBitMask);
        continue;
      }
//      std::ostringstream ofname;
//      ofname<<ap.ofname<<".r"<<i<<".mask.nii.gz";
//      if (!vMask.write(ofname.str())) {return CommonErrors::cantWrite(ofname.str()); }
      std::cout<<" [not selected]\n";
    }
    if (i>10) break;
  }
  if (selected) vBitMask.copy(vResult);
  return selected;
}

void MouseBSETool::doAll(Vol3D<uint8> &maskVolume, Vol3DBase *&referenceVolume, const Vol3DBase *volume)
{
  Timer t;
  t.start();
  for (int i=0;i<6;i++)
  {
    bool flag = false;
    switch (bseState)
    {
      case ADFilter   : flag = stepForward(maskVolume, referenceVolume, volume); break;
      case EdgeDetect : flag = stepForward(maskVolume, referenceVolume, volume); break;
      case ErodeBrain : flag = stepForward(maskVolume, referenceVolume, volume); break;
      case FindBrain  : flag = stepForward(maskVolume, referenceVolume, volume); break;
      case FinishBrain: flag = stepForward(maskVolume, referenceVolume, volume); break;
      case Finished   : flag = false; break;
      default: flag = false; break;
    }
    if (!flag) break;
  }
  t.stop();
  if (settings.verbosity>1)
  {
    std::cout<<"BSE took "<<t.elapsed()<<std::endl;
  }
}

std::string MouseBSETool::nextStepName()
{
  switch (bseState)
  {
    case ADFilter : return "anisotropic diffusion filter";
    case EdgeDetect : return "edge detection";
    case ErodeBrain : return "erode edge map";
    case FindBrain : return "find initial brain mask";
    case FinishBrain : return "refine brain mask";
    case Finished : return "skull stripping completed!";
    default :  return ""; // impossible case
  }
}

bool MouseBSETool::stepForward(Vol3D<uint8> &maskVolume, Vol3DBase *&referenceVolume, const Vol3DBase *volume)
{
  bool retcode = false;
  if (volume)
  {
    switch (bseState)
    {
      case AutoTune : // skip this if it is called in MouseBSETool
      case ADFilter :
        {
          if (settings.verbosity>1) { std::cout<<"Performing anisotropic diffusion filter"<<std::endl; }
          retcode = initialize(referenceVolume, volume);
          if (retcode) referenceVolume->filename = StrUtil::getBasename(volume->filename) + "[filtered]";
        }
        break;
      case EdgeDetect :
        if (settings.verbosity>1) { std::cout<<"Performing edge detection"<<std::endl; }
        retcode = edgeDetect(maskVolume,referenceVolume,settings.edgeConstant);
        break;
      case ErodeBrain :
        if (settings.verbosity>1) { std::cout<<"Eroding brain"<<std::endl; }
        retcode = erodeBrain(maskVolume,settings.erosionSize);
        break;
      case FindBrain :
        if (settings.verbosity>1) { std::cout<<"Finding brain"<<std::endl; }
        retcode = findBrain(maskVolume,volume);
        break;
      case FinishBrain:
        if (settings.verbosity>1) { std::cout<<"Finishing brain"<<std::endl; }
        retcode = finishBrain(maskVolume,settings.erosionSize,settings.removeBrainstem);
        break;
      case Finished :
        break;
      default:
        errorMessage = "error: unrecognized state in MouseBSETool";
        return false;
    }
  }
  return retcode;
}

bool MouseBSETool::stepBack(Vol3D<uint8> &maskVolume, Vol3DBase *&/*referenceVolume*/, const Vol3DBase * /*volume*/)
// go back one step, return false if already at beginning
{
  switch (bseState)
  {
    case AutoTune    : return false;
    case ADFilter    :
      bseState = MouseBSETool::AutoTune;
      break;
    case EdgeDetect  :
      bseState = MouseBSETool::ADFilter;
      break;
    case FindBrain   :
      bseState = MouseBSETool::EdgeDetect;
      break;
    case FinishBrain :
      bseState = MouseBSETool::FindBrain;
      edgemask.decode(maskVolume);
      break;
    case Finished    :
      bseState = MouseBSETool::FinishBrain;
      break;
    default:
      return false;
  }
  return true;
}

bool MouseBSETool::finishBrain(Vol3D<uint8> &maskVolume, const int erosionSize, bool removeBrainstem)
{
  if (settings.verbosity>2)
  {
    std::cout<<"segmenting background"<<std::endl;
  }
  Vol3D<VBit> vBit;
  vBit.encode(maskVolume);
  RunLengthSegmenter rls;
  rls.segmentBG(vBit);
  if (settings.verbosity>2)
  {
    std::cout<<"dilating with operator size "<<erosionSize<<std::endl;
  }
  for (int i=0;i<erosionSize;i++) morphology.dilateR(vBit);
  if (settings.verbosity>2)
  {
    std::cout<<"closing"<<std::endl;
  }
  morphology.dilateO2(vBit);
  rls.segmentFG(vBit);
  rls.segmentBG(vBit);
  morphology.erodeO2(vBit);

  if (settings.verbosity>2)
  {
    std::cout<<"decoding"<<std::endl;
  }
  vBit.decode(maskVolume);
  if (settings.verbosity>2)
  {
    std::cout<<"finished"<<std::endl;
  }
  bseState = Finished;

  if (removeBrainstem)
  {
    if (settings.verbosity>1)
    {
      std::cout<<"Removing brainstem."<<std::endl;
    }
    stemTrim(maskVolume);
  }
  if (settings.dilateFinalMask)
  {
    if (settings.verbosity>1) { std::cout<<"Dilating final mask."<<std::endl; }
    Vol3D<VBit> vm0;
    vm0.encode(maskVolume);
    morphology.dilateR(vm0);
    vm0.decode(maskVolume);
  }
  if (saveCortex)
  {
    vCortex.copy(vBit);
    morphology.dilateR(vCortex);
    setDifference(vCortex,vBit);
  }
  return true;
}

void MouseBSETool::stemTrim(Vol3D<uint8> &vmask, int nOpen, int nDilate)
{
  Vol3D<uint8> vtrimmed;
  Morph32 m32;
  RunLengthSegmenter rls;
  Vol3D<VBit> vm0;
  vm0.encode(vmask);
  for (int i=0;i<nOpen;i++) m32.erodeO2(vm0);
  rls.segmentFG(vm0);
  for (int i=0;i<nOpen;i++) m32.dilateO2(vm0);
  for (int i=0;i<nDilate;i++) morphology.dilateO2(vm0);
  vm0.decode(vtrimmed);
  mask(vmask,vtrimmed);
}

bool MouseBSETool::erodeBrain(Vol3D<uint8> &maskVolume, int erosionSize)
{
  erodedBrain.encode(maskVolume);
  morphology.setup(erodedBrain);
  if (settings.verbosity>1)
  {
    std::cout<<"eroding with operator size "<<erosionSize<<" : "<<std::flush;
  }
  for (int i=0;i<erosionSize;i++)
  {
    if (i&1) // alternate cube/diamond
    {
      if (settings.verbosity>1) std::cout<<"C";
      morphology.erodeC(erodedBrain);
    }
    else
    {
      if (settings.verbosity>1)
      std::cout<<"D";
      morphology.erodeR(erodedBrain);
    }
  }
  if (settings.verbosity>1)
    std::cout<<'\n';
  return true;
}

bool MouseBSETool::findBrain(Vol3D<uint8> &maskVolume, const Vol3DBase *volume)
{
  if (settings.verbosity>1)
  {
    std::cout<<"segmenting foreground"<<std::endl;
  }
  int firstlabel = runLengthSegmenter.segmentFG(initBrain);

  Vol3D<uint8> vm;
  initBrain.decode(vm);
  // check the top 3 regions to see if they are reasonably bright.
  // this avoids selecting large regions of noisy background
  bool failed = true;
  int labeled = firstlabel;
  float globalMean = (float)Vol3DOps::mean(volume);
  for (int i=0;i<10;i++)
  {
    float roiMean = (float)Vol3DOps::mean(volume,vm);
    if (roiMean>globalMean) { failed = false; break; }
    runLengthSegmenter.regionInfo[labeled].selected = 0;
    labeled++;
    runLengthSegmenter.regionInfo[labeled].selected = 1;
    runLengthSegmenter.label32FG(initBrain);
    initBrain.decode(vm);
  }
  if (failed)
  {
    runLengthSegmenter.regionInfo[labeled].selected = 0;
    runLengthSegmenter.regionInfo[firstlabel].selected = 1;
    runLengthSegmenter.label32FG(initBrain);
  }
  bseState = FinishBrain;
  initBrain.decode(maskVolume);
  return true;
}

bool MouseBSETool::edgeDetect(Vol3D<uint8> &maskVolume, const Vol3DBase *referenceVolume, const float edgeConstant)
{
  switch (referenceVolume->typeID())
  {
    case SILT::Uint8  : marrHildrethEdgeDetection(maskVolume,(Vol3D<uint8> *)referenceVolume,edgeConstant); bseState=ErodeBrain; break;
    case SILT::Sint8  : marrHildrethEdgeDetection(maskVolume,(Vol3D<sint8> *)referenceVolume,edgeConstant); bseState=ErodeBrain; break;
    case SILT::Uint16 : marrHildrethEdgeDetection(maskVolume,(Vol3D<uint16> *)referenceVolume,edgeConstant); bseState=ErodeBrain; break;
    case SILT::Sint16 : marrHildrethEdgeDetection(maskVolume,(Vol3D<sint16> *)referenceVolume,edgeConstant); bseState=ErodeBrain; break;
    default:
      errorMessage = "error: datatype ("+referenceVolume->datatypeName()+") is not currently supported for BSE.";
      std::cerr<<errorMessage<<std::endl;
      return false;
  }
  edgemask.encode(maskVolume);
  return true;
}

template <class T>
void MouseBSETool::marrHildrethEdgeDetection(Vol3D<uint8> &vMask, Vol3D<T> *vIn, const float sigma)
{
  MarrHildrethEdgeDetector<T> mh;
  mh.sigma = sigma;
  mh.detect(*vIn,vMask);
}

bool MouseBSETool::initialize(Vol3DBase *& referenceVolume, const Vol3DBase *volume)
{
  switch (volume->typeID())
  {
    case SILT::Uint8 :
    case SILT::Sint8 :  break;
    case SILT::Uint16 : VolumeScaler::scaleToUint8(vBuf,*(Vol3D<uint16> *)volume); volume = &vBuf; break;
    case SILT::Sint16 : VolumeScaler::scaleToUint8(vBuf,*(Vol3D<sint16> *)volume); volume = &vBuf; break;
    case SILT::Float32 : VolumeScaler::scaleToUint8(vBuf,*(Vol3D<float32> *)volume); volume = &vBuf; break;
    case SILT::Float64 : VolumeScaler::scaleToUint8(vBuf,*(Vol3D<float64> *)volume); volume = &vBuf; break;
    default:
      errorMessage = "error: datatype ("+volume->datatypeName()+") is not currently supported for BSE.";
      std::cout<<errorMessage<<std::endl;
      return false;
  }
  if (volume->typeID()!=SILT::Uint8 && volume->typeID()!=SILT::Sint8)
  {
    errorMessage = "error: datatype ("+volume->datatypeName()+") is not currently supported for BSE.";
    return false; // this is impossible, but serves as a reminder for later changes in the code
  }
  adf(referenceVolume,(Vol3D<uint8> *)volume,settings.diffusionIterations,settings.diffusionConstant,settings.verbosity);
  bseState=EdgeDetect;
  return true;
}

void MouseBSETool::adf(Vol3DBasePtr &ref, Vol3D<uint8> *vol, const int n, const float c, int verbosity)
{

  Vol3D<uint8> *result=0;
  if (ref)
  {
    if (ref->typeID()==vol->typeID()) result = (Vol3D<uint8> *)ref;
    else { delete ref; ref = 0; }
  }
  if (!result)
  {
    result = new Vol3D<uint8>;
    if (verbosity>1) std::cout<<"made new reference volume"<<std::endl;
  }
  if (n==0)
  {
    result->copy(*vol);
  }
  else
  {
    AnisotropicDiffusionFilter f(n,c);
    f.filter(*result,*vol,verbosity);
  }
  ref = result;
}
