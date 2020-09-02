# A-pro
ASTRAL-Pro stands for ASTRAL for PaRalogs and Orthologs. ASTRAL is a tool for estimating an unrooted species tree given a set of unrooted gene trees. ASTRAL is statistically consistent under the multi-species coalescent model (and thus is useful for handling incomplete lineage sorting, i.e., ILS). ASTRAL-pro extends ASTRAL to allow multi-copy genes. ASTRAL-pro finds the species tree that has the maximum number of shared induced quartet tree equivalent classes with the set of gene trees, subject to the constraint that the set of bipartitions in the species tree comes from a predefined set of bipartitions. ASTRAL-pro code is based on ASTRAL-MP.

### Publication:

[To be added]

## Required installations
- Java (>=1.7)

- If you need to compile (you may not), you will also need JDK and g++ or icc (supporting c++11 standard)

## Installation 

Download:

You simply need to download the zip file (under clone and download zip) and extract the contents to a folder of your choice. Alternatively, you can clone the [github repository](https://github.com/chaoszhang/A-pro).
    
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
Taxon names cannot have quotation marks in their names (sorry!). This means you also cannot have weird characters like ? in the name (underscore is fine).

## Output options
The output in is Newick format and gives: 

* the species tree topology, 
* branch lengths in coalescent units,
* branch supports measured as [local posterior probabilities](http://mbe.oxfordjournals.org/content/early/2016/05/12/molbev.msw079.short?rss=1). 
* It can also annotate branches with other quantities, such as quartet support, as described in the [tutorial](ASTRAL-MP/astral-tutorial-template.md).

The ASTRAL tree leaves the branch length of terminal branches empty. Some tools for visualization and tree editing do not like this (e.g., ape). In FigTree, if you open the tree several times, it eventually opens up (at least on our machines). In ape, if you ask it to ignore branch lengths all together, it works. In general, if you tool does not like the lack of terminal branches, you can add a dummy branch length, [as in this script](https://github.com/smirarab/global/blob/master/src/mirphyl/utils/add-bl.py). 

## Running A-pro
cd to `ASTRAL-MP` and run for multi-labelled gene trees:
```
java -D"java.library.path=lib" -jar astral.<version_number>.jar -i input
```
The results will be outputted to the standard output. To save the results in a file use the `-o` option (**Strongly recommended**):
```
java -D"java.library.path=lib" -jar astral.<version_number>.jar -i input -o output
```
To save the logs (**also recommended**), run:
```
java -D"java.library.path=lib" -jar astral.<version_number>.jar -i input -o output 2>out.log
```
cd to `ASTRAL-MP` and run for gene trees with gene-name-to-species-name mapping provided:
```
java -D"java.library.path=lib" -jar astral.<version_number>.jar -i input -a mapping -o output
```
Note that instead of `cd` to `ASTRAL-MP`, you can replace `astral.<version_number>.jar` and `-D"java.library.path=lib/"` with absolute path to A-Pro. For example, if my A-Pro is located on `/Users/smirarab/A-pro`, you can use:
```
java -D"java.library.path=/Users/chaoszhang/A-pro/lib" -jar /Users/chaoszhang/A-pro/astral.<version_number>.jar -i input -o output
```

Examples:

cd to `ASTRAL-MP` and run for multi-labelled gene trees:
```
java -jar -D"java.library.path=lib" astral.1.1.2.jar -i ../example/example1.tre
```
cd to `ASTRAL-MP` and run for gene trees with gene-name-to-species-name mapping provided:
```
java -jar -D"java.library.path=lib" astral.1.1.2.jar -i ../example/example2.tre -a ../example/example2map.txt
```

### Memory:
For big datasets (say more than 5000 taxa), increasing the memory available to Java can result in speedups. Note that you should give Java only as much free memory as you have available on your machine. So, for example, if you have 3GB of free memory, you can invoke ASTRAL using the following command to make all the 3GB available to Java:

```
java -Xmx3000M -D"java.library.path=lib/" -jar astral.<version_number>.jar -i input
```

Acknowledgment
-----------
ASTRAL code uses bytecode and some reverse engineered code from PhyloNet package (with permission from the authors).


Bug Reports:
-----------
contact ``astral-users@googlegroups.com``


Alternative:
-----------
If it is hard to install or run ASTRAL-Pro on your machine, please try C++ version [here](https://github.com/chaoszhang/FEAST).
