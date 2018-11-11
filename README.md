# Basic Requirements:

* [root][rootlink], version >= 6.14 

  [rootlink]: https://root.cern.ch/

* [rootana][rootanalink]

  [rootanalink]: https://midas.triumf.ca/MidasWiki/index.php/ROOTANA



# INSTALLATION:

```
git clone https://bitbucket.org/ttriumfdaq/agdaq.git

cd agdaq

git checkout development

. agconfig.sh

make [-j]
```

# RUNNING:

```
./agana.exe run01234sub*.lz4
```



## Internal definitions:

Spacepoints:

* -1    Anode wire outside range
* -2    Negative time
* -3    X or Y not a number
* -4    Z not defined
* -5    R outside fiducial volume
* -6   	X, Y or Z errors not a number
* -7    Z error not defined
* 1     good point


Helix:

* -1	Number of spacepoints is less than 5
* 0	Fitted helix
* 1	Good helix - NOT used for vertexing
* 2	Seed helix - used for vertexing 
* 3	Added helix - used for vertexing
* -2	R fit failed
* -3	Z fit failed
* -4	R chi^2 cut failed
*-5	Z chi^2 cut failed
* -6	D cut failed
* -7	duplicated
* -14   R chi^2 too small
* -15   Z chi^2 too small
* -11   Too few points


Vertex:

* -2	no good helices
* 0	only one good helix
* -1	failed to find minimum-distance-pair
* 1	only two good helices
* 2	didn't improve vertex (more than 2 helices)
* 3	vertex improved

