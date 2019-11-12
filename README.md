# A-pro
ASTRAL for PaRalogs and Orthologs
## Required installations
Java (>=1.7), g++ or icc (supporting c++11 standard)

If you have trouble with ASTRAL, cd to `ASTRAL-MP` and run `bash make.sh`. For more information please see AVX2 section of https://github.com/smirarab/ASTRAL/tree/MP

## Input requirement
The input gene trees must be in the Newick format, either multi-labelled (eg. ((SpeciesA,SpeciesB),(SpeciesA,SpeciesC));) or with gene-name-to-species-name mapping provided in the following format (eg. for ((GeneA1,GeneB1),(GeneA2,GeneC1));):
```
GeneA1 SpeciesA
GeneA2 SpeciesA
GeneB1 SpeciesB
GeneC1 SpeciesC
```

## Running A-pro
cd to `ASTRAL-MP` and run for multi-labelled gene trees:
```
java -D"java.library.path=lib/" -jar astral.5.14.2.jar -i input -o output
```
cd to `ASTRAL-MP` and run for gene trees with gene-name-to-species-name mapping provided:
```
java -D"java.library.path=lib/" -jar astral.5.14.2.jar -i input -a mapping -o output
```
