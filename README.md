# A-pro
ASTRAL for PaRalogs and Orthologs
## Required installations
Java (>=1.7), Python 2.7 (with Dendropy), g++ or icc (supporting c++11 standard)

## Input requirement
The input gene trees must be in the Newick format, either multi-labelled (eg. ((SpeciesA,SpeciesB),(SpeciesA,SpeciesC));) or with gene-name-to-species-name mapping provided in the following format (eg. for ((GeneA1,GeneB1),(GeneA2,GeneC1));):
```
GeneA1 SpeciesA
GeneA2 SpeciesA
GeneB1 SpeciesB
GeneC1 SpeciesC
```

## Running A-pro
In the A-pro repository run for multi-labelled gene trees:
```
bash A-pro.bash input output
```
In the A-pro repository run for gene trees with gene-name-to-species-name mapping provided:
```
bash A-pro.bash input output mapping
```
