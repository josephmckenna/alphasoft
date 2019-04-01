
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