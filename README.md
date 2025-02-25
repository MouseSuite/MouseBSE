# MouseBSE

[![build MouseBSE](https://github.com/MouseSuite/MouseBSE/actions/workflows/build.yml/badge.svg)](https://github.com/MouseSuite/MouseBSE/actions/workflows/build.yml)

This is an adaptation of BrainSuite's BSE brain extraction program designed to work on rodent MRI.

## Copyright
Copyright (C) 2025 The Regents of the University of California and the University of Southern California.

## License
MouseBSE is licensed under [GNU Lesser General Public License v2.1 only](https://spdx.org/licenses/LGPL-2.1-only.html).

## Attribution
MouseBSE is a variation of BSE, which is part of [BrainSuite](https://brainsuite.org). If you use MouseBSE in one of your papers, please cite one of the following papers:

- [Shattuck DW and Leahy RM (2002) BrainSuite: An Automated Cortical Surface Identification Tool Medical Image Analysis, 8(2):129-142](http://dx.doi.org/10.1006/nimg.2001.0756).

- [Shattuck DW, Sandor-Leahy SR, Schaper KA, Rottenberg DA, and Leahy RM (2001) Magnetic Resonance Image Tissue Classification Using a Partial Volume Model NeuroImage 13(5):856-876](http://dx.doi.org/10.1006/nimg.2001.0756).

## Documentation and Support
For more detailed information on MouseBSE, please visit our MouseBSE website, [https://mousesuite.org/mouseBSE/](https://mousesuite.org/mouseBSE/). 

You may also post comments or questions on our [MouseSuite discussion page](https://github.com/orgs/MouseSuite/discussions) or to our [issue tracker](https://github.com/MouseSuite/MouseBSE/issues).

## Usage
```
mousebse [settings]

required settings:
-i <input filename>            input MRI volume

optional settings:
--license                      show the license information
-o <output filename>           output brain-masked MRI volume
--xmin xplane                  zeros out data for x < xplane [default: 0]
--ymin yplane                  zeros out data for y < yplane [default: 0]
--zmin zplane                  zeros out data for z < zplane [default: 0]
--xmax xplane                  zeros out data for x > xplane [default: 2147483647]
--ymax yplane                  zeros out data for y > yplane [default: 2147483647]
--zmax zplane                  zeros out data for z > zplane [default: 2147483647]
--zpad nslices                 zeropad the image by nslices [default: 0]
-d <float>                     diffusion constant [default: 50]
-n <iterations>                diffusion iterations [default: 10]
-s <edge sigma>                edge detection constant [default: 0.64]
-r <size>                      radius of erosion/dilation filter [default: 1]
-c <size>                      closing size [default: 8]
-p dilation_radius             dilate final mask by dilation_radius (0==don't dilate) [default: 0]
--mask <filename>              save smooth brain mask
--init <filename>              initial brain mask
--select region                select region from connected components [default: -1]
--adf <filename>               diffusion filter output
--eroded <filename>            eroded edge map output
--edge <filename>              edge map output
-v <number>                    verbosity level (0=silent) [default: 1]
--norotate                     retain original orientation (default behavior will auto-rotate input NII files to RAS orientation
--timer                        show timing

example:
mousebse -i input_mri.img -o skull_stripped_mri.img
```





