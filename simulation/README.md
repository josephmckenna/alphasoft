
# Basic Requirements:

## CRY

[CRY](https://nuclear.llnl.gov/simulation/main.html "CRY at LLNL website")

```
wget https://nuclear.llnl.gov/simulation/cry_v1.7.tar.gz
```

Unpack and follow instructions contained in the README to install

For clarity and simplicity: put the following instruction into your configuration script (e.g. .bashrc)

```
. $HOME/packages/cry_v1.7/setup
```

## CADMesh

[CADMesh](https://github.com/christopherpoole/CADMesh "CADMesh on GitHub")

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

[Garfield++](http://garfieldpp.web.cern.ch/garfieldpp/ "Garfield++ at CERN")

```
git clone https://gitlab.cern.ch/garfield/garfieldpp.git
```

Follow [this][gppinstall] instructions to install Garfield++

[gppinstall]: http://garfieldpp.web.cern.ch/garfieldpp/getting-started/




# Compilation Instructions

[CMake](https://cmake.org/ "cmake website"), **version >=3**

```
cd $AGRELEASE/simulation/geant4/
mkdir run && cd run
cmake -DCMAKE_BUILD_TYPE=Release geant4
```


on CentOS7 is likely that the call looks like this

```
cmake3 -DCMAKE_BUILD_TYPE=Release geant4
```


Finally, compile with default tool

```
cmake3 --build . -- -j`nproc --ignore=2`
```


if you need `cmake` *version 3* you can try to follow [these instructions](./install.cmake.from.source.md)



# Running the Simulation

## Set the output directory

It should have occured when you issued agconfig.sh.

```
if [[ -z "${MCDATA}" ]]; then echo "MCDATA not set"; else echo "good to go"; fi
```

If MCDATA is not set, set it to your favourite location.


## Simulate single pion

```
rTPCsim runHeedInterface.mac
```

## Graphics with User Interface

```
rTPCsim
```

the macro vis.mac is called automatically, to simulate a single pion issue

```
/control/execute runHeedInterface.mac
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
* 6*: test cases by Lars;
* default: annihilation uniform on the circle and uniform in z, the latter is parametrized by its centre and extension.
