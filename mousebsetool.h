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

#ifndef MouseBSETool_H
#define MouseBSETool_H

#include <vol3d.h>
#include <DS/runlengthsegmenter.h>
#include <DS/morph32.h>
#include <iostream>

class MouseBSETool {
public:
  MouseBSETool();
  enum BSESteps { AutoTune=0, ADFilter=1, EdgeDetect=2, ErodeBrain=3, FindBrain=4, FinishBrain=5, Finished=6 };
  class Settings {
  public:
    Settings();
    int diffusionIterations;
    float diffusionConstant;
    float edgeConstant;
    int erosionSize;
    bool removeBrainstem;
    int dilateFinalMask;
    int verbosity;
    int selectRegion;
  };
  typedef Vol3DBase *Vol3DBasePtr;

  Settings settings;
  BSESteps bseState;
  RunLengthSegmenter runLengthSegmenter;
  Morph32 morphology;

// main interface steps
  void doAll(Vol3D<uint8> &maskVolume, Vol3DBase *&referenceVolume, const Vol3DBase *volume);
  bool stepForward(Vol3D<uint8> &maskVolume, Vol3DBase *&referenceVolume, const Vol3DBase *volume);
  bool stepBack(Vol3D<uint8> &maskVolume, Vol3DBase *&, const Vol3DBase *); // go back one step, return false if at beginning
  bool goForward(Vol3D<uint8> &maskVolume, Vol3DBase *&referenceVolume, const Vol3DBase *volume); // run next step, return false if at end
  bool reset(); // reset to the beginning, clear stored data
  bool concom(Vol3DBase *vIn, Vol3D<VBit> &vBitMask, int threshold=100000);

  std::string nextStepName();
// the individual steps
  void adf(Vol3DBasePtr &referenceVolume, Vol3D<uint8> *volume, const int nIterations, const float diffusionConstant, int verbosity=1);
  bool initialize(Vol3DBase *& referenceVolume, const Vol3DBase *volume);
  bool edgeDetect(Vol3D<uint8> &maskVolume, const Vol3DBase *referenceVolume, const float edgeConstant);
  bool erodeBrain(Vol3D<uint8> &maskVolume, int erosionSize);
  bool findBrain(Vol3D<uint8> &maskVolume, const Vol3DBase *volume);
  bool finishBrain(Vol3D<uint8> &maskVolume, const int erosionSize, bool removeBrainstem=false);
  // this trims brainstem / spinal cord
  void stemTrim(Vol3D<uint8> &vmask, int nOpen=2, int nDilate=4);
  template <class T>
  void marrHildrethEdgeDetection(Vol3D<uint8> &vMask, Vol3D<T> *vIn, const float sigma);
  bool saveCortex;
  Vol3D<VBit> edgemask, erodedBrain, initBrain, vCortex;
  Vol3D<uint8> vBuf;
  std::string errorMessage;
};

#endif

