#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>
#include "phylonet_coalescent_Polytree_PTNative.h"
#include "WeightCalculationInstructionGenerator.cpp"
#include "GenetreeAnnotator.cpp"
#include "x86intrin.h"

#define NUM_BITS_IN_WORD 64
#define BATCH_SIZE 32 

using namespace std;

__attribute__((vector)) __attribute__ ((always_inline)) inline uint64_t G(uint16_t xa16, uint16_t xc16, uint16_t ya16, uint16_t yb16, uint16_t yc16, uint16_t zb16){
	uint32_t ybm = xa16 - (uint16_t) 1, zbm1 = xa16 + yc16 - (uint16_t) 2, zbm2 = xc16 + ya16 - (uint16_t) 2, xa = xa16, yc = yc16, xc = xc16, ya = ya16, yb = yb16, zb = zb16;
	uint64_t xayc = xa * yc, xaycm = yb * ybm + zb * zbm1, xcya = xc * ya, xcyam = zb * zbm2;
	return xayc * xaycm + xcya * xcyam;
}

__attribute__((vector)) __attribute__ ((always_inline)) inline unsigned long long G(unsigned short a, unsigned short b, unsigned short c, unsigned short d, 
		unsigned short e, unsigned short f, unsigned short g, unsigned short h, unsigned short i){
	uint16_t xa = a, xb = b, xc = c, ya = d, yb = e, yc = f, za = g, zb = h, zc = i;
	return G(xa, xc, ya, yb, yc, zb) + G(xb, xa, yb, yc, ya, zc) + G(xc, xb, yc, ya, yb, za);
}

struct Polytree{
	int n = 0, listSize = 0, queueSize = 0, nWord = 0;
	int* __restrict__ queue = nullptr;
	stringstream* ptfile = nullptr;
	
	~Polytree(){
		if (queue) delete queue;
		if (ptfile) delete ptfile;
	}
	
	void compute(unsigned long long* __restrict__ result, const unsigned long long* __restrict__ b) const{
		__attribute__((aligned(64))) unsigned short (* __restrict__ lst)[3 * BATCH_SIZE] = new __attribute__((aligned(64))) unsigned short[listSize][3 * BATCH_SIZE]{};
		__attribute__((aligned(64))) unsigned long long weight[BATCH_SIZE] = {};
		
		for (int i = 0; i < n; i++){
			for (int j = 0; j < 3 * BATCH_SIZE; j++){
				lst[i][j] = (b[j * nWord + i / NUM_BITS_IN_WORD] & (1LL << (i % NUM_BITS_IN_WORD))) ? 1 : 0;
			}
		}
		
		for (int i = 0; i < queueSize; i++){
			int cmd = queue[i];
			if (cmd >= 0) {
				int y = cmd, x1 = queue[++i], x2 = queue[++i], c = queue[++i];
				if (c == 1) {
					#pragma ivdep
					#pragma simd vectorlength(32)
					#pragma vector always
					#pragma vector aligned
					for (int k = 0; k < BATCH_SIZE; k++){
						weight[k] += G(lst[x1][k], lst[x1][k + BATCH_SIZE], lst[x1][k + 2 * BATCH_SIZE], 
							lst[x2][k], lst[x2][k + BATCH_SIZE], lst[x2][k + 2 * BATCH_SIZE],
							lst[y][k], lst[y][k + BATCH_SIZE], lst[y][k + 2 * BATCH_SIZE]);
					}
				}
				else {
					#pragma ivdep
					#pragma simd vectorlength(32)
					#pragma vector always
					#pragma vector aligned
					for (int k = 0; k < BATCH_SIZE; k++){
						weight[k] += c * G(lst[x1][k], lst[x1][k + BATCH_SIZE], lst[x1][k + 2 * BATCH_SIZE], 
							lst[x2][k], lst[x2][k + BATCH_SIZE], lst[x2][k + 2 * BATCH_SIZE],
							lst[y][k], lst[y][k + BATCH_SIZE], lst[y][k + 2 * BATCH_SIZE]);
					}
				}
			}
			else{
				int cmd2 = queue[++i];
				int y = ~cmd, x1 = cmd2, x2 = queue[++i];
				#pragma ivdep
				#pragma simd vectorlength(32)
				#pragma vector always
				#pragma vector aligned
				for (int k = 0; k < 3 * BATCH_SIZE; k++){
					lst[y][k] = lst[x1][k] + lst[x2][k];
				}
			}
		}
		delete lst;
		#pragma ivdep
		#pragma simd vectorlength(32)
		#pragma vector always
		#pragma vector aligned
		for (int k = 0; k < BATCH_SIZE; k++){
			result[k] = weight[k];
		}
	}
	
	unsigned long long count(){
		unsigned long long* lst = new unsigned long long[listSize]{};
		unsigned long long weight = 0;
		
		for (int i = 0; i < n; i++){
			lst[i] = 1;
		}
		
		for (int i = 0; i < queueSize; i++){
			int cmd = queue[i];
			if (cmd >= 0) {
				unsigned long long y = lst[cmd], x1 = lst[queue[++i]], x2 = lst[queue[++i]], c = queue[++i];
				if (c == 1) {
					weight += x1 * (x1 - 1) * x2 * (x2 - 1) / 2 + x1 * x2 * y * (x1 + x2 - 2);
				}
				else {
					weight += c * (x1 * (x1 - 1) * x2 * (x2 - 1) / 2 + x1 * x2 * y * (x1 + x2 - 2));
				}
			}
			else{
				int cmd2 = queue[++i];
				int y = ~cmd, x1 = cmd2, x2 = queue[++i];
				lst[y] = lst[x1] + lst[x2];
			}
		}
		delete lst;
		return weight;
	}
	
	unsigned long long support(const unsigned long long* __restrict__ b) const{
		unsigned long long (*lst)[4] = new unsigned long long[listSize][4]{};
		unsigned long long weight = 0;
		
		for (int i = 0; i < n; i++){
			for (int j = 0; j < 4; j++){
				lst[i][j] = (b[j * nWord + i / NUM_BITS_IN_WORD] & (1LL << (i % NUM_BITS_IN_WORD))) ? 1 : 0;
			}
		}
		
		for (int i = 0; i < queueSize; i++){
			int cmd = queue[i];
			if (cmd >= 0) {
				int y = cmd, x1 = queue[++i], x2 = queue[++i], c = queue[++i];
				unsigned long long x1a = lst[x1][0], x1b = lst[x1][1], x1c = lst[x1][2], x1d = lst[x1][3],
						x2a = lst[x2][0], x2b = lst[x2][1], x2c = lst[x2][2], x2d = lst[x2][3],
						ya = lst[y][0], yb = lst[y][1], yc = lst[y][2], yd = lst[y][3];
				if (c == 1) {
					weight += x1a*x1b*x2c*x2d + x1c*x1d*x2a*x2b + x1a*x1b*(x2c*yd+yc*x2d) + x1c*x1d*(x2a*yb+ya*x2b) + x2a*x2b*(x1c*yd+yc*x1d) + x2c*x2d*(x1a*yb+ya*x1b);
				}
				else {
					weight += c * (x1a*x1b*x2c*x2d + x1c*x1d*x2a*x2b + x1a*x1b*(x2c*yd+yc*x2d) + x1c*x1d*(x2a*yb+ya*x2b) + x2a*x2b*(x1c*yd+yc*x1d) + x2c*x2d*(x1a*yb+ya*x1b));
				}
			}
			else{
				int cmd2 = queue[++i];
				int y = ~cmd, x1 = cmd2, x2 = queue[++i];
				for (int k = 0; k < 4; k++){
					lst[y][k] = lst[x1][k] + lst[x2][k];
				}
			}
		}
		delete lst;
		return weight;
	}
} pt;

JNIEXPORT jobjectArray JNICALL Java_phylonet_coalescent_Polytree_00024PTNative_cppParse
		(JNIEnv *env, jclass, jstring jinput, jstring jmapping){
  	const char *cinput = env->GetStringUTFChars(jinput, NULL);
	string input(cinput);
	env->ReleaseStringUTFChars(jinput, cinput);
	const char *cmapping = env->GetStringUTFChars(jmapping, NULL);
	string mapping(cmapping);
	env->ReleaseStringUTFChars(jmapping, cmapping);
	
	tuple<string, string, stringstream*> result = annotate(input, mapping);
	
	pt.ptfile = get<2>(result);
	jobjectArray ret = (jobjectArray)env->NewObjectArray(2, env->FindClass("java/lang/String"),env->NewStringUTF(""));

    env->SetObjectArrayElement(ret, 0, env->NewStringUTF(get<1>(result).c_str()));
	env->SetObjectArrayElement(ret, 1, env->NewStringUTF(get<0>(result).c_str()));
    return(ret);
}

JNIEXPORT void JNICALL Java_phylonet_coalescent_Polytree_00024PTNative_cppInit
		(JNIEnv *env, jclass, jobjectArray jnames){
	//cerr << "Hello World!" << endl;
	unordered_map<string, int> name2id;
	pt.n = env->GetArrayLength(jnames);
	pt.nWord = (pt.n + 63) / 64;
	//cerr << "n = " << pt.n << "nword = " << pt.nWord << endl;
	for (int i = 0; i < pt.n; i++){
		jstring jname = (jstring) env->GetObjectArrayElement(jnames, i);
		const char *cname = env->GetStringUTFChars(jname, NULL);
		string name(cname);
		name2id[name] = i;
		//cerr << "name = " << name << "; id = " << i << endl;
		env->ReleaseStringUTFChars(jname, cname);
	}
	
	stringstream& fin(*pt.ptfile);
	int numId, numCluster, numSum, numScore;
	fin >> numId;
	//cerr << "numId = " << numId << endl;
	vector<int> idremap;
	for (int i = 0; i < numId; i++){
		string name;
		fin >> name;
		idremap.push_back(name2id[name]);
		//cerr << "from = " << i << "; to = " << name2id[name] << endl;
	}
	fin >> numCluster >> numSum;
	pt.listSize = numCluster;
	//cerr << "listSize = " << pt.listSize << endl;
	vector<int> sumTarget, scoreTarget, scoreCnt;
	vector<vector<int> > sumSources, scoreSources;
	for (int i = 0; i < numSum; i++){
		int y, x0, x1;
		fin >> y >> x0 >> x1;
		sumTarget.push_back(y);
		sumSources.push_back({x0, x1});
	}
	fin >> numScore;
	for (int i = 0; i < numScore; i++){
		int y, x0, x1, c;
		fin >> y >> x0 >> x1 >> c;
		scoreTarget.push_back(y);
		scoreSources.push_back({x0, x1});
		scoreCnt.push_back(c);
	}
	WeightCalculationInstructionGenerator ig(idremap, sumTarget, sumSources, scoreTarget, scoreSources, scoreCnt, numId);
	const vector<int>& inst = ig.getInstruction();
	pt.queueSize = inst.size();
	pt.queue = new int[inst.size()];
	memcpy(pt.queue, inst.data(), inst.size() * sizeof(int));
	//cerr << "queueSize = " << pt.queueSize << endl;
	//for (int i: inst) cerr << i << " ";
}

JNIEXPORT void JNICALL Java_phylonet_coalescent_Polytree_00024PTNative_cppBatchCompute
		(JNIEnv *env, jclass, jlongArray jres, jobjectArray jarr1, jobjectArray jarr2, jobjectArray jarr3){
	unsigned long long result[BATCH_SIZE] = {};
	unsigned long long* b = new unsigned long long[3 * BATCH_SIZE * pt.nWord]{};
	int size = env->GetArrayLength(jres);
	jlong *jpres = env->GetLongArrayElements(jres, 0);
	for (int i = 0; i < size; i += BATCH_SIZE){
		for (int j = 0; j < BATCH_SIZE && j < size - i; j++){
			jlongArray jb1 = (jlongArray) env->GetObjectArrayElement(jarr1, i + j);
			jlongArray jb2 = (jlongArray) env->GetObjectArrayElement(jarr2, i + j);
			jlongArray jb3 = (jlongArray) env->GetObjectArrayElement(jarr3, i + j);
			int len1 = env->GetArrayLength(jb1);
			jlong *jpb1 = env->GetLongArrayElements(jb1, 0);
			memcpy(b + (0 * BATCH_SIZE + j) * pt.nWord, jpb1, len1 * sizeof(unsigned long long));
			env->ReleaseLongArrayElements(jb1, jpb1, 0);
			int len2 = env->GetArrayLength(jb2);
			jlong *jpb2 = env->GetLongArrayElements(jb2, 0);
			memcpy(b + (1 * BATCH_SIZE + j) * pt.nWord, jpb2, len2 * sizeof(unsigned long long));
			env->ReleaseLongArrayElements(jb2, jpb2, 0);
			int len3 = env->GetArrayLength(jb3);
			jlong *jpb3 = env->GetLongArrayElements(jb3, 0);
			memcpy(b + (2 * BATCH_SIZE + j) * pt.nWord, jpb3, len3 * sizeof(unsigned long long));
			env->ReleaseLongArrayElements(jb3, jpb3, 0);
		}
		pt.compute(result, b);
		for (int j = 0; j < BATCH_SIZE && j < size - i; j++){
			jpres[i + j] = result[j];
		}
	}
	env->ReleaseLongArrayElements(jres, jpres, 0);
	delete b;
}

JNIEXPORT jlong JNICALL Java_phylonet_coalescent_Polytree_00024PTNative_cppCountEqClass
		(JNIEnv *env, jclass){
	return pt.count();
}

JNIEXPORT jlong JNICALL Java_phylonet_coalescent_Polytree_00024PTNative_cppSupport
  (JNIEnv *env, jclass, jlongArray jb1, jlongArray jb2, jlongArray jb3, jlongArray jb4){
	unsigned long long* b = new unsigned long long[4 * pt.nWord]{};
	
	int len1 = env->GetArrayLength(jb1);
	jlong *jpb1 = env->GetLongArrayElements(jb1, 0);
	memcpy(b + 0 * pt.nWord, jpb1, len1 * sizeof(unsigned long long));
	env->ReleaseLongArrayElements(jb1, jpb1, 0);
	int len2 = env->GetArrayLength(jb2);
	jlong *jpb2 = env->GetLongArrayElements(jb2, 0);
	memcpy(b + 1 * pt.nWord, jpb2, len2 * sizeof(unsigned long long));
	env->ReleaseLongArrayElements(jb2, jpb2, 0);
	int len3 = env->GetArrayLength(jb3);
	jlong *jpb3 = env->GetLongArrayElements(jb3, 0);
	memcpy(b + 2 * pt.nWord, jpb3, len3 * sizeof(unsigned long long));
	env->ReleaseLongArrayElements(jb3, jpb3, 0);
	int len4 = env->GetArrayLength(jb4);
	jlong *jpb4 = env->GetLongArrayElements(jb4, 0);
	memcpy(b + 3 * pt.nWord, jpb4, len4 * sizeof(unsigned long long));
	env->ReleaseLongArrayElements(jb4, jpb4, 0);
	unsigned long long result = pt.support(b);
	delete b;
	return result;
}

