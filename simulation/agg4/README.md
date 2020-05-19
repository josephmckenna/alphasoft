## Compilation Instructions
```
mkdir -p run && cd run
cmake3 ..
cmake3 --build . -- -j
```

## Detector Settings

```
settings.dat
```

1. PWB time resolution in ns
2. FMC32 time resolution in ns
3. Magnetic field in T
4. CO2 fraction
5. activate all material (bool)
6. simulate protoype (bool)




## General Info

Hbar annihilation data: `annihilation.dat`

x,y,z,t=0

Physics List: `examples/extended/electromagnetic/TestEm8`

Simulation of cosmics:

```
$ source /usr/local/cry_v1.7/setup
```

settings for cosmics: `cry.file`
