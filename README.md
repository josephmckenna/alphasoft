ALPHA2 Virtual Monte Carlo (a2MC)

///< --------------------------------------------------------------------------
///< -1. HOW TO GET IT
///< --------------------------------------------------------------------------
git clone https://gitlab.cern.ch/unibs/a2mc.git
It stores the Alpha2 Monte Carlo in the "a2mc" subdirectory
------------------------------------ oOo --------------------------------------


///< --------------------------------------------------------------------------
///< 1. PREREQUISITES
///< --------------------------------------------------------------------------
Root, Geant4, VMC, VGM and Geant4_vmc need to be installed in your system.
Instructions on how to install such packages are available in install_VMC.txt.
Shell scripts ("pre_setup.sh" and a "env_setup.sh") are also available in the 
input subdirectory.
The a2mc has been tested with: 
-> root 6.22/06, geant4 10.07,     VGM 4.8, VMC 1.0.p3, geant4_vmc 5.3
-> root 6.22/02, geant4 10.06.p02, VGM 4.8, VMC 1.0.p3, geant4_vmc 5.2
------------------------------------ oOo --------------------------------------


///< --------------------------------------------------------------------------
///< 2. CONFIG, COMPILE AND INSTALL
///< --------------------------------------------------------------------------
cd a2mc
source setup.sh
===> ***     Edit/change "./cmake_config_install.sh" to match your local 
                       (GEANT4/VMC/VGM) installation paths.            *** <===
source cmake_config_install.sh
------------------------------------ oOo --------------------------------------


///< --------------------------------------------------------------------------
///< 3. GENERATION PARAMETERS
///< --------------------------------------------------------------------------
Some parameters of the generation can be handled/tuned in a user card, a text
file (a2MC.ini) that can be changed accordingly to the generation needs. 
For example, it is possible to generate pbars or muons, to turn on and off the
magnetic field, to insert some geometry elements (detectors, magnets) etc. 
Please see a2MC.ini for a "in-line" descriptions of the parameters.
------------------------------------ oOo --------------------------------------


///< --------------------------------------------------------------------------
///< 4. RUN
///< --------------------------------------------------------------------------
./g4vmc_a2MC -events 10000 -run 0

This command generate 10000 events for the run number 0.
If a generation with the same run number took previously place, the execution 
will stop with an error message. In other words, it is not possible to produce 
two generations with the same run number.
------------------------------------ oOo --------------------------------------


///< --------------------------------------------------------------------------
///< 5. OUTPUT
///< --------------------------------------------------------------------------
The a2mc generates outputs in two subdirectories.
"root" subdir
    The main output of the MC is a root file containing information about hits
    and particles. It is named as "a2MC-YYYY-MM-DD-HH-MM-SS_RUN.root", where 
    YYYY=year, MM=month, DD=day, HH=hour, MM=min, SS=sec and RUN=run number.
"output" subdir 
    The geometry is stored in a file named "a2mcApparatus-RUN.root",
    where RUN=run number. 
    The a2mc also saves a log file for each run. This file (a2mc-RUN.log)
    It contains information about the run start and the run (random generation)
    seed, and a copy of the following files:
    a2MC.ini/a2mcApparatus.cxx/a2mcGenerator.cxx/a2mcVirtualMC.cxx. 
    In this way it is always possible to recover the condition and the 
    parameters used for the generation.
------------------------------------ oOo --------------------------------------


///< --------------------------------------------------------------------------
///< 6. QC [QUICK CHECKS/QUALITY CONTROL]
///< --------------------------------------------------------------------------
In the subdir "analysis", it is possible to find some macros to quickly check
the generation. For example, the show_gen.C macro. It can be used as follows:
"root show_gen.C\(RUN\)". It will show some distributions about the primary 
particle (vertex, momentum, etc.). 
In the subdir "a2mcEve", it is possibile to check/inspect the geometry, with
the macro checkGeo.C ["root checkGeo.C\(RUN\)"]. 
------------------------------------ oOo --------------------------------------


///< --------------------------------------------------------------------------
///< 7. SIMULATION VIEWER
///< --------------------------------------------------------------------------
In the subdir "a2mcEve" it is possible to visualize the MC output (geometry,
hits, tracks). For the tracks, actually, the "store_tracks" flag in a2MC.ini
should be turned ON. To start the viewer: "root runViewer.C\(RUN\)".
------------------------------------ oOo --------------------------------------

For any questions: germano.bonomi@cern.ch
