Tool to produce input data for ML quality assestment of ITS QC plots; 

# Requirements:

- Installed version of AliceO2 and QualityControl
- Active alienv token in .globus

# Installation

- enter alienv enviroment: `alienv enter QualityControl/latest O2/latest`
- create `build` directory
- from `./build` :  cmake ..
- instalation: cmake --build .. -jN , where N - number of your cores

# Usage

- enter alienv enviroment
- run from the main folder: ./build/its-qm -i ./inputs -o ./download; the directory with input periods or outputs could be configured with -i and -o flags

# Format for input data:
- The list of runs to analyse has the following structure:
    - inputs/Periods.txt: list of LHC periouds to analyze, given in format  period_name, collision_system, energy, a-pass name, for example: LHC25ae,OO,5.4,apass2
    - Run list for each period is saved in inputs/PERIOD_NAME.txt and is given as list of runs for each row

# Format for output data:
    - Output data is saved in ./downloads/COLLISION_SYSTEM/ENERGY/runXXXX.root, each run in separate file
    - .root file contains the list of objects:

	 {"/ITS/MO/Clusters/mw/Layer0/ClusterOccupation", "TH2"},
     {"/ITS/MO/Clusters/mw/Layer1/ClusterOccupation", "TH2"},
     {"/ITS/MO/Clusters/mw/Layer2/ClusterOccupation", "TH2"},
     {"/ITS/MO/Clusters/mw/Layer3/ClusterOccupation", "TH2"},
     {"/ITS/MO/Clusters/mw/Layer4/ClusterOccupation", "TH2"},
     {"/ITS/MO/Clusters/mw/Layer5/ClusterOccupation", "TH2"},
     {"/ITS/MO/Clusters/mw/Layer6/ClusterOccupation", "TH2"},
     {"/ITS/MO/Tracks/mw/AngularDistribution", "TH2"},
     {"/ITS/MO/Tracks/mw/NClusters", "TH1"},
     {"/ITS/MO/Tracks/mw/Ntracks", "TH1"},
     {"/ITS/MO/ITSTrackTask/NClustersPerTrackEta","TH2"}  


    - Each object of the list has BAD or GOOD flag, BAD quality is defined as **missing stave** for a whole duration of 3 minutes when the QC object was filled. Status of the ITS staves is defined via TimeDeadMap framework defined by Nicolo Valle
