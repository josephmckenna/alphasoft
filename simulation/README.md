
# Basic Requirements:

## CRY

[CRY][crylink]

[crylink]: https://nuclear.llnl.gov/simulation/main.html

```
wget https://nuclear.llnl.gov/simulation/cry_v1.7.tar.gz
```

Unpack and follow instructions contained in the README to install

For clarity and simplicity: put the following instruction into your configuration script (e.g. .bashrc)

```
. $HOME/packages/cry_v1.7/setup
```

## CADMesh

[CADMesh][cadmeshlink]

[cadmeshlink]: https://github.com/christopherpoole/CADMesh

```
git clone https://github.com/christopherpoole/CADMesh.git
cd CADMesh
git checkout v1.1
```
Follow instructions contained in the README to install

For clarity and simplicity: put the following instructions into your configuration script (e.g. .bashrc)

```
export CADMESH_HOME=$HOME/packages/CADMesh
export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:$CADMESH_HOME
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$CADMESE_HOME/lib
```

## Garfield++

[Garfield++][gpphome]

[gpphome]: http://garfieldpp.web.cern.ch/garfieldpp/

```
git clone https://gitlab.cern.ch/garfield/garfieldpp.git
```

Follow [this][gppinstall] instructions to install Garfield++

[gppinstall]: http://garfieldpp.web.cern.ch/garfieldpp/getting-started/




# Compilation Instructions

## CMake

[CMake][cmakeweb], **version >=3**

```
cd $AGRELEASE/simulation
cmake -DCMAKE_BUILD_TYPE=Release geant4
```


on CentOS7 is likely that the call looks like this

```
cmake3 -DCMAKE_BUILD_TYPE=Release geant4
```


It's easier to use the CMake curses interface

```
ccmake geant4
```

Set/change the variables for the required specifications then press "c" followed by "g"

[cmakeweb]:https://cmake.org/




## Compile with GNUMake

Once the Makefile has been generated

```
make -j
```



# Running the Simulation

## Set the output directory

It should have occured when you issued agconfig.sh.

```
if [[ -z "${MCDATA}" ]]; then echo "MCDATA not set"; else echo "good to go"; fi
```

If MCDATA is not set, set it to your favourite location.


## Simulate single pion

```
AGTPC runHeedInterface.mac
```

## Graphics with User Interface

```
AGTPC
```

the macro vis.mac is called automatically, to simulate a single pion issue

```
/control/exectute runHeedInterface
```


## Primary Generator conventions

You can switch between different types of simulation by calling

```
/AGTPC/setRunType <integer>
```

Available Options:
* 1: Chukman's simulation of "Up" Vs. "Down", filter by Victor M.;
* 2: Cosmic ray generator;
* 21: Cosmic ray generator -- horizontal;
* 4: annihilation on residual gas;
* 41: annihilation on z axis;
* 6: test single track at fixed location;
* 6i: test cases by Lars;
* default: annihilation uniform on the circle and uniform in z, the latter is parametrized by its centre and extension.