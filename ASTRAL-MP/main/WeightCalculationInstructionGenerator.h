#ifndef WeightCalculationInstructionGenerator_h
#define WeightCalculationInstructionGenerator_h
#include <iostream>
#include <vector>

using namespace std;

class WeightCalculationInstructionGenerator{
	const vector<int> recode;
	vector<int> inst;
	
	int transencode(int i) const{
		if (i < recode.size()) return recode[i];
		else return i;
	}
	
public:

	WeightCalculationInstructionGenerator(){}

	/**
	* Parameters:
	* targetLocationList: 
	* sourceLocationsList: 
	* xLocationList: 
	* yLocationsList: 
	* cList: a
	* n: 0~n-1 are known
	*/
	
	WeightCalculationInstructionGenerator(const vector<int> recode, const vector<int> targetLocationList, const vector<vector<int> > sourceLocationsList, const vector<int> xLocationList, const vector<vector<int> > yLocationsList, const vector<int> cList, int n);
	
	/**
	 * add function
	 * targetLocation: index of element to be calcualted in array 
	 * sourceLocations: vector contains x values
	 */
	void addClusterSizes(const int targetLocation, const vector<int> sourceLocations){
		cerr << targetLocation << " <- " << transencode(sourceLocations[0]) << " + " << transencode(sourceLocations[1]) << endl;
		inst.push_back(~targetLocation);
		inst.push_back(transencode(sourceLocations[0]));
		inst.push_back(transencode(sourceLocations[1]));
		for (int i = 2; i < sourceLocations.size(); i++){
			inst.push_back(~targetLocation);
			inst.push_back(targetLocation);
			inst.push_back(transencode(sourceLocations[i]));
		}
		cerr << inst.size() << endl;
	}
	
	/**
	 * g(x)
	 * xLocation: xlocation in the array a
	 * yLocation: the location of elements in setY in the array a
	 */
	void computeWeight(const int xLocation, const vector<int> yLocations, int c){
		if (yLocations.size() == 2){
			inst.push_back(transencode(xLocation));
			inst.push_back(transencode(yLocations[0]));
			inst.push_back(transencode(yLocations[1]));
			inst.push_back(c);
		}
		else {
			inst.push_back(~yLocations.size());
			inst.push_back(~transencode(xLocation));
			for (int i = 0; i < yLocations.size(); i++) inst.push_back(transencode(yLocations[i]));
			inst.push_back(c);
		}
	}    

	const vector<int>& getInstruction() const{
		for (int i: inst) cerr << i << " ";
		return inst;
	}
};

#endif
