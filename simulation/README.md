
# Submodules

The following simulation components are part of the `alphasoft` project via the git submodule utility

```
# from the root folder
git submodule status # to verify their commit hash
git submodule update --init # to sync them
```


## CRY

[CRY](https://nuclear.llnl.gov/simulation/main.html "CRY at LLNL website")


## CADMesh

[CADMesh](https://github.com/christopherpoole/CADMesh "CADMesh on GitHub")


# Basic Requirements:

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
