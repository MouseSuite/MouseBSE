// Copyright (C) 2025 The Regents of the University of California and
// the University of Southern California
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

#include <DS/runlengthsegmenter.h>
#include <graph.h>
#include <algorithm>

int graphFactor=10;

RunLengthSegmenter::RunLengthSegmenter() :
	mode(D6),
	cx(0), cy(0), cz(0),
	high(255), low(0),
	datasize(0),
	runcount(0),
	nsymbols(0),
	verbose(false),
	NMax(0),
	rlsPicked(-1),
	ensureCentered(true)
{
}

RunLengthSegmenter::~RunLengthSegmenter()
{
}

void RunLengthSegmenter::setup(const int cx_, const int cy_, const int cz_)
{
	cx=cx_;
	cy=cy_;
	cz=cz_;
	nregions = 0;
	high = 255;
	low = 0;
	runcount = 0;
	datasize = cz * cx * cy;
	runs.resize(datasize);
	linestart.resize(cz*cy+1);
}

void RunLengthSegmenter::segment(uint8 *imageIn, uint8 *imageOut, uint8 zero, uint8 one)
{
	runcount = 0;
	high = one;
	low = zero;
	encode(imageIn);
	label(imageOut);
}

void RunLengthSegmenter::makeGraph()
{
	switch (mode)
	{
		case D18 :
			makeGraph18();
			break;
		case D26 :
			makeGraph26();
			break;
		case D6 : 
		default:
			makeGraph6();
			break;
	}
}

void RunLengthSegmenter::encode(uint8 *buffer)
{
	const uint8 code=high;
	int index = 0;
	int state = 0;
	runcount = 0;
	RunLength newRun;
	int linecount = 0;
	int *pLinestart = &linestart[0];
	for (int z=0; z<cz; z++)
	for (int y=0; y<cy; y++)
	{
		pLinestart[linecount++] = runcount;
		state = (buffer[index]==code);
		if (state)
		{
			newRun.start = 0;
		}
		index++;
		for (int x=1; x<cx; x++, index++)
		{
			if (buffer[index]!=code)
			{
				if (state!=0) // terminate a code
				{
					newRun.stop = x-1;
					state = 0;
					runs[runcount++] = newRun;
				}
			}
			else
			{
				if (state==0) // start a new code
				{
					newRun.start = x;
					state = 1;
				}
			}
		}
		if (state!=0) // terminate the code.
		{
			newRun.stop = cx - 1;
			runs[runcount++] = newRun;
		}
	}
	pLinestart[linecount] = runcount;
}

int RunLengthSegmenter::presegmentBG(Vol3D<VBit> &v)
{
	setup(v.cx,v.cy,v.cz);
	runcount = 0;
	encode32BG(v.raw32());
	makeGraph();
	return rlsPicked;
}

int RunLengthSegmenter::presegmentFG(Vol3D<VBit> &v)
{
	setup(v.cx,v.cy,v.cz);
	runcount = 0;
	high = 255;
	low = 0;
	encode32FG(v.raw32());
  makeGraph();
	return rlsPicked;
}

int RunLengthSegmenter::segment32FG(unsigned int *imageIn, unsigned int *imageOut)
{
	runcount = 0;
	high = 255;
	low = 0;
	encode32FG(imageIn);
	makeGraph();
	label32FG(imageOut);
	return rlsPicked;
}

int RunLengthSegmenter::segmenttest32FG(unsigned int *imageIn, unsigned char *imageOut)
{
	runcount = 0;
	high = 255;
	low = 0;
	encode32FG(imageIn);
	makeGraph();
	label(imageOut);
	return rlsPicked;
}

int RunLengthSegmenter::segmenttest32FG(unsigned char *imageIn,unsigned int *imageOut)
{
  runcount = 0;
	high = 255;
	low = 0;
	encode(imageIn);
	makeGraph();
	label32FG(imageOut);
	return rlsPicked;
}

void RunLengthSegmenter::segment32BG(unsigned int *imageIn, unsigned int *imageOut)
{
  runcount = 0;
	encode32BG(imageIn);
	makeGraph();
	label32BG(imageOut);
}

void RunLengthSegmenter::segment32FG(uint8 *imageIn, unsigned int *imageOut)
{
	runcount = 0;
	high = 255;
	low = 0;
	encode(imageIn);
	makeGraph();
	label32FG(imageOut);
}

void RunLengthSegmenter::segment32BG(uint8 *imageIn, unsigned int *imageOut)
{
	runcount = 0;
	high = 0;
	low = 255;
	encode(imageIn);
	makeGraph();
	label32BG(imageOut);
}

void RunLengthSegmenter::label32FG(unsigned int *imageOut)
{
	remap(newmap);
	int index = 0;
	int label = 0;
	int linecount = 0;
	const int extra = (cx&0x1F);
	const int wordsPerLine  = (cx>>5);
	const int wx = wordsPerLine + (extra!=0); // width of x
	const int wsize = wx * cy * cz;
	for (int d=0;d<wsize;d++) imageOut[d] = 0;
	int *pLinestart = &linestart[0];	
	for (int z=0;z<cz; z++)
	{
		for (int y=0; y<cy; y++)
		{
			const int first = pLinestart[linecount];
			const int last  = pLinestart[++linecount];
			for (int i=first; i<last; i++)
			{
				if (newmap[label]) // only need to label positives.
				{
					const int start = runs[i].start;
					const int stop = runs[i].stop;
					unsigned int *X = imageOut + index + (start>>5);
					unsigned int val = 0;
					int pos = start&0x1F;
					for (int j=start; j<=stop; j++)
					{	
						val|=0x80000000;
						if (((++pos) &= 0x1F)==0) { *X++ |= val; val=0;}
						val>>=1;
					}
					if (pos) { *X |= (val>>(31-pos)); }
				}
				label++;
			}
			index += wx;			
		}
	}	
}

void RunLengthSegmenter::label32BG(unsigned int *imageOut)
{
	remap(newmap);
	int *pLinestart = &linestart[0];
	int index = 0;
	int label = 0;
	int linecount = 0;
	const int extra = (cx&0x1F);
	const int wordsPerLine  = (cx>>5);
	const int wx = wordsPerLine + (extra!=0); // width of x
	int endwidth = 32 - extra; // extra bits in the code
	unsigned int edgecode=0xFFFFFFFF;
	edgecode>>=endwidth;
	int d = 0;
	for (int z=0;z<cz;z++)
	{
		for (int y=0;y<cy;y++)
		{
			for (int x=0;x<wordsPerLine;x++)
			{
				imageOut[d++] = 0xFFFFFFFF;
			}
			if (extra>0) imageOut[d++] = edgecode;
		}
	}
	for (int z=0;z<cz; z++)
	{
		for (int y=0; y<cy; y++)
		{
			int first = pLinestart[linecount];
			int last  = pLinestart[++linecount];
			for (int i=first; i<last; i++)
			{
				if (newmap[label]) // only need to label positives.
				{
					int start = runs[i].start;
					int stop = runs[i].stop;
					unsigned int *X = imageOut + index + (start>>5);
					unsigned int val = 0;
					int pos = start&0x1F;
					for (int j=start; j<=stop; j++)
					{	
						val|=0x80000000;
						if (((++pos) &= 0x1F)==0) { *X++ ^= val; val=0;}
						val>>=1;
					}
					if (pos) { *X ^= (val>>(31-pos)); }
				}
				label++;
			}
			index += wx;			
		}
	}
}

void RunLengthSegmenter::encode32BG(unsigned int *imageIn)
{
	int state = 0;
	runcount = 0;
	RunLength newRun;
	int linecount = 0;
	int *pLinestart = &linestart[0];
	unsigned int *cptr  = imageIn;
	for (int z=0; z<cz; z++)
	for (int y=0; y<cy; y++)
	{
		pLinestart[linecount++] = runcount;
		unsigned int val = *(cptr++);
		int p = 1;
		int bit = (val & 1);
		state = (bit==0); // grab the lowest bit
		val >>=1;
		if (state) newRun.start = 0;
		for (int x=1;x<cx;x++)
		{
			if (p==0) { val = *(cptr++); }
			(++p) &= 0x1F;			
			int bit = (val & 1);		
			if (bit!=0)
			{
				if (state!=0) // terminate a code
				{
					newRun.stop = x-1;
					state = 0;
					runs[runcount++] = newRun;
				}
			}
			else
			{
				if (state==0) // start a New code
				{
					newRun.start = x;
					state = 1;
				}
			}
			val >>=1;  // shift the next one into place
		}
		if (state!=0) // terminate the code.
		{
			newRun.stop = cx - 1;
			runs[runcount++] = newRun;
		}
	}
	pLinestart[linecount] = runcount;
}

void RunLengthSegmenter::label(uint8 *buffOut)
{
	remap(newmap);
	int *pLinestart = &linestart[0];
	for (int d=0;d<datasize;d++) buffOut[d] = low;
	int index = 0;
	int label = 0;
	int linecount = 0;
	for (int z=0;z<cz; z++)
	{
		for (int y=0; y<cy; y++)
		{
			int first = pLinestart[linecount];
			int last  = pLinestart[++linecount];
			for (int i=first; i<last; i++)
			{
				int start = index + runs[i].start;
				int stop = index + runs[i].stop;
				for (int j=start; j<=stop; j++) buffOut[j] = newmap[label];
				label++;
			}
			index += cx;			
		}
	}
}

inline bool RunLengthSegmenter::regionInfoGE(const RegionInfo &ri, const RegionInfo &ri2)
{
	return (ri.count > ri2.count);
}

inline int RunLengthSegmenter::intersect(RunLength& r1, RunLength& r2)
{
	return ((r1.start<=r2.stop)&&(r2.start<=r1.stop));
}

int RunLengthSegmenter::labelID(const int xIn, const int yIn, const int zIn)
{
	const int start = linestart[zIn * cy + yIn ];
	const int stop  = linestart[zIn * cy + yIn +1];
	for (int i=start;i<stop;i++)
	{
		if ((xIn>=runs[i].start)&&(xIn<=runs[i].stop))
		{
			return map[i];
		}
	}
	return -1;
}

void RunLengthSegmenter::population()
{
	regionInfo.resize(nsymbols+1);
	nregions=nsymbols+1;
	RegionInfo *ri = &regionInfo[0];

	int *pLinestart = &linestart[0];
	for (int c=0;c<=nsymbols;c++) 
	{
		ri[c].count = 0;
		ri[c].cx = 0;
		ri[c].cy = 0;
		ri[c].cz = 0;
		ri[c].label = 0;
		ri[c].selected = 0;
  }
  int label = 0;
	int linecount = 0;
	for (int z=0;z<cz; z++)
	{
		for (int y=0; y<cy; y++)
		{
			int first = pLinestart[linecount];
			int last  = pLinestart[++linecount];
			for (int i=first; i<last; i++)
			{
				int length = (runs[i].stop - runs[i].start) + 1;
				if (length>0)
				{
					if (map[label]<0)
					{
						std::cerr<<"error: illegal code in segment map"<<std::endl;
						return;
					}
					RegionInfo &r = ri[map[label]];
					r.count += length;
					r.cx += runs[i].start * length + (length * (length - 1))/2;
					r.cy += y * length;
					r.cz += z * length;
					r.label = map[label];
				}
				label++;
      }
		}
	}
	for (int c=0;c<=nsymbols;c++)
		if (ri[c].count>0)
		{
			ri[c].cx /= ri[c].count;
			ri[c].cy /= ri[c].count;
			ri[c].cz /= ri[c].count;
		}
	std::sort(regionInfo.begin(),regionInfo.end(),RunLengthSegmenter::regionInfoGE);
}

void RunLengthSegmenter::makeGraph6()
{
	std::vector<int> &pLinestart(linestart);
  int linecount = 0;
  Graph graph(datasize/graphFactor);
	graph.reset(runcount+1);
	for (int z=0;z<cz; z++)
	{
// Since the first line of each slice (y==0) is not connected to anything 
// above it (there are no voxels for (y<0), there is nothing to link.
		linecount++;
		for (int y=1; y<cy; y++)
		{
			int first  = pLinestart[linecount];
			int runA   = pLinestart[linecount-1]; // previous
			int prevline = linecount - cy;
			int last   = pLinestart[linecount+1];
			linecount++;
			if (last<=first) continue;					// the line is empty
			if (z>0)
			{
				int curr = first;
				int upStart= pLinestart[prevline]; // previous slice
				int upStop = pLinestart[prevline+1]; // previous slice
				int up   = upStart;
				for (;;)
				{
					if (up  >=upStop) break;
					if (curr>=last  ) break;
					if (intersect(runs[up],runs[curr]))
					{
							graph.link(up  ,curr);
              graph.link(curr,up  );
					}
					int newUp   = up;
					int newCurr = curr;
					if (runs[up].stop<=runs[curr].stop) newUp++;
					if (runs[up].stop>=runs[curr].stop) newCurr++;
					up   = newUp;
					curr = newCurr;
				}
			}
			int runB = first;
			for (;;)
			{
				if (runA>=first) break;
				if (runB>=last ) break;
				if (intersect(runs[runA],runs[runB]))
				{
					graph.link(runA,runB);
          graph.link(runB,runA);
				}
				int newA = runA;
				int newB = runB;
				if (runs[runA].stop<=runs[runB].stop) newA++;
				if (runs[runA].stop>=runs[runB].stop) newB++;
				runA = newA;
				runB = newB;
			}
		}
	}
	map.resize(runcount+1);
	newmap.resize(runcount+1);
	nsymbols = graph.makemap(&map[0]);
	population();
	findmax();
}

void RunLengthSegmenter::findmax()
// Find the single most populous label
// Can check if centroid of object is near the center of image
{
	int region = 0;
	if (ensureCentered&&(high!=0))
	{
    int xMin = cx/10;
    int xMax = cx - xMin - 1;
    int yMin = cy/10;
    int yMax = cy - yMin - 1;
    int zMin = cz/10;
    int zMax = cz - zMin - 1;
    const size_t maxR = (regionInfo.size()>7) ? 7 : regionInfo.size(); // TODO: should change this from hard code (7) to a parameter.
		for(size_t r=0;r<maxR;r++)
		{
			RegionInfo &regInf = regionInfo[r];
      if ((regInf.cx<xMin)||(regInf.cx>xMax)) continue;
      if ((regInf.cy<yMin)||(regInf.cy>yMax)) continue;
      if ((regInf.cz<zMin)||(regInf.cz>zMax)) continue;
			region = r;
			break;
		}
	}
	regionInfo[region].selected = 1;
	rlsPicked = region;
}

void RunLengthSegmenter::remap(std::vector<LabelType> &newMap)
{
	std::vector<int> relabel(nsymbols+1,low);
	for (int i=0;i<nsymbols+1;i++)
	{
		if (regionInfo[i].selected)
			relabel[regionInfo[i].label] = high;
	}
	for (int i=0;i<runcount+1;i++)
	{	
		newMap[i] = relabel[map[i]];
	}
}

int RunLengthSegmenter::findRegion(const int x, const int y, const int z)
{
	int start = linestart[z*cy * y ];
	int stop  = linestart[z*cy * y +1];
	int term = -1;
	for (int i=start; i<stop; i++)
	{
		if (runs[i].stop<x) continue;
		if (runs[i].start>x) continue;
		term = i;
		break;
	}
	return term;
}

void RunLengthSegmenter::makeGraph26()
{
	int linecount = 0;
	int linkcount = 0;
  Graph graph(datasize/graphFactor);
	graph.reset(runcount+1);
	std::vector<int> &pLinestart(linestart);
	for (int z=0;z<cz; z++)
	{
// Since the first line of each slice (y==0) is not connected to anything 
// above it (there are no voxels for (y<0), there is nothing to link.
		linecount++;
		for (int y=1; y<cy; y++)
		{
			int first  = pLinestart[linecount];
			int runA   = pLinestart[linecount-1]; // previous
			int prevline = linecount - cy;
			int last   = pLinestart[linecount+1];
			linecount++;
			if (last<=first) continue;					// the line is empty
			if (z>0) // check previous slice.
			{
				int curr = first;
				{
					int upStart= pLinestart[prevline-1]; // previous slice
					int upStop = pLinestart[prevline  ]; // previous slice
					int up   = upStart;
					for (;;)
					{
						if (up  >=upStop) break;
						if (curr>=last  ) break;
						if (runs[up].neighbors(runs[curr]))
						{
								graph.link(up  ,curr);
								graph.link(curr,up  );
							linkcount++;
						}
						int newUp   = up;
						int newCurr = curr;
						if (runs[up].stop<=runs[curr].stop) newUp++;
						if (runs[up].stop>=runs[curr].stop) newCurr++;
						up   = newUp;
						curr = newCurr;
					}
				}
				curr = first;
				{
					int upStart= pLinestart[prevline]; // previous slice
					int upStop = pLinestart[prevline+1]; // previous slice
					int up   = upStart;
					for (;;)
					{
						if (up  >=upStop) break;
						if (curr>=last  ) break;
						if (runs[up].neighbors(runs[curr]))
						{
								graph.link(up  ,curr);
								graph.link(curr,up  );
							linkcount++;
						}
						int newUp   = up;
						int newCurr = curr;
						if (runs[up].stop<=runs[curr].stop) newUp++;
						if (runs[up].stop>=runs[curr].stop) newCurr++;
						up   = newUp;
						curr = newCurr;
					}
				}
			}
			int runB = first;
			for (;;)
			{
				if (runA>=first) break;
				if (runB>=last ) break;
				if (runs[runA].neighbors(runs[runB]))
				{
					graph.link(runA,runB);
					graph.link(runB,runA);
					linkcount++;
				}
				int newA = runA;
				int newB = runB;
				if (runs[runA].stop<=runs[runB].stop) newA++;
				if (runs[runA].stop>=runs[runB].stop) newB++;
				runA = newA;
				runB = newB;
			}
		}
	}
	map.resize(runcount+1);
	newmap.resize(runcount+1);
	nsymbols = graph.makemap(&map[0]);
	if (verbose) std::cout<<"There are "<<nsymbols<<" symbols."<<std::endl;
	population();
	findmax();
}

void RunLengthSegmenter::makeGraph18()
{
	int linecount = 0;
	int linkcount = 0;
  Graph graph(datasize/graphFactor);
	graph.reset(runcount+1);
	std::vector<int> &pLinestart(linestart);
	for (int z=0;z<cz; z++)
	{
// Since the first line of each slice (y==0) is not connected to anything 
// above it (there are no voxels for (y<0), there is nothing to link.
		linecount++;
		for (int y=1; y<cy; y++)
		{
			int first  = pLinestart[linecount];
			int runA   = pLinestart[linecount-1]; // previous
			int prevline = linecount - cy;
			int last   = pLinestart[linecount+1];
			linecount++;
			if (last<=first) continue;					// the line is empty
			if (z>0) // check previous slice.
			{
				int curr = first;
				{
					int upStart= pLinestart[prevline-1]; // previous slice
					int upStop = pLinestart[prevline  ]; // previous slice
					int up   = upStart;
					for (;;)
					{
						if (up  >=upStop) break;
						if (curr>=last  ) break;
						if (runs[up].intersects(runs[curr]))
						{
								graph.link(up  ,curr);
								graph.link(curr,up  );
							linkcount++;
						}
						int newUp   = up;
						int newCurr = curr;
						if (runs[up].stop<=runs[curr].stop) newUp++;
						if (runs[up].stop>=runs[curr].stop) newCurr++;
						up   = newUp;
						curr = newCurr;
					}
				}
				curr = first;
				{
					int upStart= pLinestart[prevline]; // previous slice
					int upStop = pLinestart[prevline+1]; // previous slice
					int up   = upStart;
					for (;;)
					{
						if (up  >=upStop) break;
						if (curr>=last  ) break;
						if (runs[up].neighbors(runs[curr]))
						{
								graph.link(up  ,curr);
								graph.link(curr,up  );
							linkcount++;
						}
						int newUp   = up;
						int newCurr = curr;
						if (runs[up].stop<=runs[curr].stop) newUp++;
						if (runs[up].stop>=runs[curr].stop) newCurr++;
						up   = newUp;
						curr = newCurr;
					}
				}
			}
			int runB = first;
			for (;;)
			{
				if (runA>=first) break;
				if (runB>=last ) break;
				if (runs[runA].neighbors(runs[runB]))
				{
					graph.link(runA,runB);
					graph.link(runB,runA);
					linkcount++;
				}
				int newA = runA;
				int newB = runB;
				if (runs[runA].stop<=runs[runB].stop) newA++;
				if (runs[runA].stop>=runs[runB].stop) newB++;
				runA = newA;
				runB = newB;
			}
		}
	}
	map.resize(runcount+1);
	newmap.resize(runcount+1);
	nsymbols = graph.makemap(&map[0]);
	if (verbose) std::cout<<"There are "<<nsymbols<<" symbols."<<std::endl;
	population();
	findmax();
}

void RunLengthSegmenter::encode32FG(unsigned int *imageIn)
{
	const uint8 code=high;
	int state = 0;
	runcount = 0;
	RunLength newRun;
	int linecount = 0;
	unsigned int *cptr = imageIn;
	std::vector<int> &pLinestart(linestart);
	for (int z=0; z<cz; z++)
	{
		for (int y=0; y<cy; y++)
		{
			pLinestart[linecount++] = runcount;
			state = (*cptr)&1;		// equiv to (imageIn[index]==code);
			if (state)
			{
				newRun.start = 0;
			}
			unsigned int val=0;
			int p = 0;
			for (int x=0;x<cx;x++)
			{
				if (p==0) { val = *(cptr++); }
				(++p) &= 0x1F;
        uint8 lowbit = 0xFF * (val & 1);	// grab the lowest bit
				val >>=1;													// shift the next one into place				
				if (x==0) continue;
        if (lowbit!=code)
				{
					if (state!=0) // terminate a code
					{
						newRun.stop = x-1;
						state = 0;
						runs[runcount++] = newRun;
					}
				}
				else
				{
					if (state==0) // start a New code
					{
						newRun.start = x;
						state = 1;
					}
				}
			}
			if (state!=0) // terminate the code.
			{
				newRun.stop = cx - 1;
				runs[runcount++] = newRun;
			}
		}
	}
	pLinestart[linecount] = runcount;
}
