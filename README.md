# Basic Requirements:

* [root][rootlink], **supported version >= 6.14**

  [rootlink]: https://root.cern.ch/

* [rootana][rootanalink], it can be obtained as a git submodule

  [rootanalink]: https://midas.triumf.ca/MidasWiki/index.php/ROOTANA
  
* [CMake][cmakelink] **version >=3.0**

  [cmakelink]: https://cmake.org/ "CMake website"
  
## Optional Requirements:

* [Midas][midaslink], this is useful for "online analysis"

  [midaslink]: https://midas.triumf.ca/MidasWiki/index.php/Main_Page "MIDAS Wiki"

OS (strongly recommended):

* [cern-centos7][cern-centos7link]

  [cern-centos7link]: http://linux.web.cern.ch/linux/centos7/

Succefully tested on [Ubuntu 20.04LTS][ubuntu-link]

[ubuntu-link]: https://ubuntu.com/blog/ubuntu-20-04-lts-arrives "Canonical Announcement"



# INSTALLATION:

```
git clone https://bitbucket.org/expalpha/alphasoft.git --recursive
cd alphasoft
. agconfig.sh
mkdir build && cd build
```

`cmake3` if OS is CentOS7, `cmake` for Ubuntu

```
cmake3 ../
cmake3 --build . --target install
```

for multi-process build

```
cmake3 --build . --target install -- -j`nproc --ignore=2`
```


# RUNNING:

Basic invocation

```
agana.exe run01234sub*.mid.lz4
```


Additional options

```
--mt                    : Enable multithreaded mode.
-O/path/to/newfile.root : Specify output root file filename

--: All following arguments are passed to the analyzer modules Init() method

    --recoff                                Turn off reconstruction
    --aged                                  Turn on event display
    --diag                                  Enable histogramming
    --anasettings /path/to/settings.json    Load analysis settings
```

For example:

```
agana.exe --mt -Otest01234.root run01234sub*.mid.lz4 -- --diag --anasettings ana/cosm.json

```

 


# Internal definitions:


Spacepoints:

* -1    Anode wire outside range
* -2    Negative time
* -3    X or Y not a number
* -4    Z not defined
* -5    R outside fiducial volume
* -6   	X, Y or Z errors not a number
* -7    Z error not defined
*  1    Good point


Helix:

*  -1	Number of spacepoints is less than 5
*   0	Fitted helix
*   1	Good helix - NOT used for vertexing
*   2	Seed helix - used for vertexing 
*   3	Added helix - used for vertexing
*  -2	R fit failed
*  -3	Z fit failed
*  -4	R chi^2 cut failed
*  -5	Z chi^2 cut failed
*  -6	D cut failed
*  -7	duplicated
* -14   R chi^2 too small
* -15   Z chi^2 too small
* -11   Too few points


Vertex:

* -2	no good helices
*  0	only one good helix
* -1	failed to find minimum-distance-pair
*  1	only two good helices
*  2	didn't improve vertex (more than 2 helices)
*  3	vertex improved

