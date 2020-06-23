# A-pro
ASTRAL-Pro stands for ASTRAL for PaRalogs and Orthologs.

### Publication:

[To be added]

## Required installations
- Java (>=1.7)

- If you need to compile (you may not), you will also need JDK and g++ or icc (supporting c++11 standard)

## Installation 

Normally, you would not need to install. You can just run A-Pro, as we show bellow.

If you have trouble with running the packaged A-Pro, 

1. `cd ASTRAL-MP`
2. `bash make.sh`. 

For more information please see AVX2 section of https://github.com/smirarab/ASTRAL/tree/MP

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
java -D"java.library.path=lib" -jar astral.<version_number>.jar -i input -o output
```
cd to `ASTRAL-MP` and run for gene trees with gene-name-to-species-name mapping provided:
```
java -D"java.library.path=lib" -jar astral.<version_number>.jar -i input -a mapping -o output
```
Note that instead of `cd` to `ASTRAL-MP`, you can replace `astral.<version_number>.jar` and `-D"java.library.path=lib/"` with absolute path to A-Pro. For example, if my A-Pro is located on `/Users/smirarab/A-pro`, you can use:

```
java -D"java.library.path=/Users/smirarab/A-pro/lib" -jar /Users/smirarab/A-pro/astral.<version_number>.jar -i input -o output
```
