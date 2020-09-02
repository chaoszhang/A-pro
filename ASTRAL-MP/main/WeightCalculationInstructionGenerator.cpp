#include "WeightCalculationInstructionGenerator.h"
#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>

using namespace std;

int addCluster(int target, int n, unordered_set<int>& present, WeightCalculationInstructionGenerator& g, const unordered_map<int, vector<int> >& children){
	if (present.count(target) == 0){
		if (children.count(target)){
			int a = addCluster(children.at(target)[0], n, present, g, children);
			int b = addCluster(children.at(target)[1], n, present, g, children);
			
			g.addClusterSizes(target, {a,b});
		}
		present.insert(target);
	}
	return target;
}


WeightCalculationInstructionGenerator::WeightCalculationInstructionGenerator(const vector<int> recode, const vector<int> targetLocationList, const vector<vector<int> > sourceLocationsList, const vector<int> xLocationList, const vector<vector<int> > yLocationsList, const vector<int> cList, int n): recode(recode){
	unordered_set<int> present; //Check if and unknown x has been calculated already
    unordered_map<int, vector<int> > children;  //Used to find if the element to be calculated exists in the targetLocationList
    
    //Initialize the unordered map with key and values from the targetLocationList
    for(int i = 0, size = targetLocationList.size(); i < size; i++ ){
    	children[targetLocationList[i]] = sourceLocationsList[i];
    }

	//calculate g(x)
	for(size_t i = 0, size = xLocationList.size(); i < size; i++){
		addCluster(xLocationList[i], n, present, *this, children);
    	addCluster(yLocationsList[i][0], n, present, *this, children);
    	addCluster(yLocationsList[i][1], n, present, *this, children);
    	computeWeight(xLocationList[i], yLocationsList[i], cList[i]);
	}

}

