# Basic Requirements:

* [root][rootlink], **supported version >= 6.14**

  [rootlink]: https://root.cern.ch/

* [rootana][rootanalink]

  [rootanalink]: https://midas.triumf.ca/MidasWiki/index.php/ROOTANA


OS (strongly recommended):

* [cern-centos7][cern-centos7link]

  [cern-centos7link]: http://linux.web.cern.ch/linux/centos7/

Others OS (only building tested):
* Ubuntu 16
* Fedora 28

# INSTALLATION:

```
git clone https://bitbucket.org/ttriumfdaq/agdaq.git
cd agdaq
git checkout development
. agconfig.sh
#Optional if you don't have rootana installed
#. scripts/gitChecker/Get_rootana.sh 
make [-j]
```

# RUNNING:

```
./agana.exe run01234sub*.lz4
```


## EVENT DISPLAY

```
./agana.exe run01234sub*.lz4 -- --aged
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

