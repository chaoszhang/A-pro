#include "WeightCalculationInstructionGenerator.h"
#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>

using namespace std;

/**
 * targetLocationH: the nth location in the array a, where from 0 to n-1 are the given x
 * targetLocationListH: the list of locations that need to use add function to calculate
 * sourceLocationsListH: the x values used to calculate the unknown x
 * d: check if a value to be calculated appear more than once
 * g: reference to the WeightCalculationInstructionGenerator object to be used when calling addClusterSizes
 */
void addClusterSizesHelper(int targetLocationH, const vector<int>& targetLocationListH, const vector<vector<int>>& sourceLocationsListH, unordered_set<int>& d, WeightCalculationInstructionGenerator& g, unordered_map<int,int>& t){
    //Visit every elements in the targetLocationList starts from the unkonow element at the position with index targetLocationH in the targetLocationListH
    for(int i = targetLocationH, sizeI = targetLocationListH.size(); i < sizeI; i++){
        // Visited the sourceLocationList in case the add function for unknown x contains unknown x
        for(int j = 0, sizeJ = sourceLocationsListH[i].size(); j < sizeJ; j++){
        	auto it = t.find(sourceLocationsListH[i][j]);
        	
            if(it != t.end()){            	
                int idx = it->second;
                t.erase(it);              
            	addClusterSizesHelper(idx, targetLocationListH, sourceLocationsListH, d, g, t);
            }
        }

        if(d.find(targetLocationListH[i]-1) == d.end()){
        	g.addClusterSizes(targetLocationListH[i], sourceLocationsListH[i]);
        	d.insert(targetLocationListH[i]-1);
        }
	}
}

void recursivelyAddCluster(int target, int n, const vector<int>& targetLocationListH, const vector<vector<int>>& sourceLocationsListH, unordered_set<int>& d, WeightCalculationInstructionGenerator& g, const unordered_map<int,int>& t){
	if (target < n || d.count(target)) return;
	d.insert(target);
	int i = t.at(target);
	for (int j: sourceLocationsListH[i]) recursivelyAddCluster(j, n, targetLocationListH, sourceLocationsListH, d, g, t);
	g.addClusterSizes(targetLocationListH[i], sourceLocationsListH[i]);
}


WeightCalculationInstructionGenerator::WeightCalculationInstructionGenerator(const vector<int> recode, const vector<int> targetLocationList, const vector<vector<int> > sourceLocationsList, const vector<int> xLocationList, const vector<vector<int> > yLocationsList, const vector<int> cList, int n): recode(recode){
	unordered_set<int> duplicated; //Check if and unknown x has been calculated already
    unordered_map<int, int> targetLocation;  //Used to find if the element to be calculated exists in the targetLocationList
    //Initialize the unordered map with key and values from the targetLocationList
    for(int i = 0, siz = targetLocationList.size(); i < siz; i++ ){
    	targetLocation.insert({targetLocationList[i], i});
    }

	//calculate unknown x value	
	for(int i = 0, siz = targetLocationList.size(); i < siz; i++ ){
    	recursivelyAddCluster(targetLocationList[i], n, targetLocationList, sourceLocationsList, duplicated, *this, targetLocation);
    }
	//calculate g(x)

	for(size_t i = 0, size = xLocationList.size(); i < size; i++){
      computeWeight(xLocationList[i], yLocationsList[i],cList[i]);
	}

}

