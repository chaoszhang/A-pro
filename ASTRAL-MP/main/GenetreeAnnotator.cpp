#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <random>

using namespace std;

void display(vector<pair<string, int> > tr){
	for (auto e: tr) cerr << e.second << "\t" << e.first << endl;
	cerr << endl;
}

vector<string> sample(vector<pair<string, int> > tr, int level){
	vector<string> result;
	vector<pair<string, int> > stack;
	for (auto &e: tr){
		if (e.second <= level){
			for (int i = 0, n = 1 << (level - e.second); i < n; i++) result.push_back(e.first);
			continue;
		}
		stack.push_back(e);
		while (stack.size() >= 2 && stack[stack.size() - 2].second == stack.back().second){
			if (rand() & 1) stack[stack.size() - 2].first = stack.back().first;
			stack[stack.size() - 2].second--;
			stack.pop_back();
		}
		if (stack.back().second == level){
			result.push_back(stack.back().first);
			stack.pop_back();
		}
	}
	return result;
}

class DynamicBitset{
	int size = 0;
	vector<uint64_t> vec;
	
public:
	DynamicBitset(){}
	DynamicBitset(int sz): size(sz), vec((sz + 63) / 64){}
	
	void set(int i){
		if (i >= size){
			size = i + 1;
			if ((size + 63) / 64 > vec.size()){
				vec.resize((size + 63) / 64);
			}
		}
		vec[i / 64] |= (1LL << (i % 64));
	}
	
	DynamicBitset operator|(const DynamicBitset &b) const{
		if (size < b.size) return b | *this;
		DynamicBitset res(size);
		for (int i = 0; i < b.vec.size(); i++){
			res.vec[i] = vec[i] | b.vec[i];
		}
		for (int i = b.vec.size(); i < vec.size(); i++){
			res.vec[i] = vec[i];
		}
		return res;
	}
	
	DynamicBitset operator&(const DynamicBitset &b) const{
		if (size < b.size) return b & *this;
		DynamicBitset res(b.size);
		for (int i = 0; i < b.vec.size(); i++){
			res.vec[i] = vec[i] & b.vec[i];
		}
		return res;
	}
	
	DynamicBitset operator^(const DynamicBitset &b) const{
		if (size < b.size) return b ^ *this;
		DynamicBitset res(size);
		for (int i = 0; i < b.vec.size(); i++){
			res.vec[i] = vec[i] ^ b.vec[i];
		}
		for (int i = b.vec.size(); i < vec.size(); i++){
			res.vec[i] = vec[i];
		}
		return res;
	}
	
	DynamicBitset operator-(const DynamicBitset &b) const{
		DynamicBitset res(size);
		for (int i = 0; i < vec.size(); i++){
			if (i < b.vec.size()) res.vec[i] = vec[i] & ~b.vec[i];
			else res.vec[i] = vec[i];
		}
		return res;
	}
	
	bool operator==(const DynamicBitset &b) const{
		if (size < b.size) return b == *this;
		for (int i = 0; i < b.vec.size(); i++){
			if (vec[i] != b.vec[i]) return false;
		}
		for (int i = b.vec.size(); i < vec.size(); i++){
			if (vec[i] != 0) return false;
		}
		return true;
	}
	
	bool operator!=(const DynamicBitset &b) const{
		return !(*this == b);
	}
	
	bool isDisjointTo(const DynamicBitset &b) const{
		if (size < b.size) return b.isDisjointTo(*this);
		for (int i = 0; i < b.vec.size(); i++){
			if ((vec[i] & b.vec[i]) != 0) return false;
		}
		return true;
	}
	
	vector<int> setBits() const{
		vector<int> res;
		for (int i = 0; i < vec.size(); i++){
			for (int j = 0; j < 64; j++){
				if (vec[i] & (1LL << j)) res.push_back(64 * i + j);
			}
		}
		return res;
	}
};

struct ClusterHash{
	uint64_t h[2] = {};
	
	ClusterHash(){}
	
	ClusterHash(uint64_t a, uint64_t b){
		h[0] = a;
		h[1] = b;
	}
	
	ClusterHash operator + (const ClusterHash &c) const{
		return ClusterHash(h[0] + c.h[0], h[1] + c.h[1]);
	}
	
	ClusterHash operator - (const ClusterHash &c) const{
		return ClusterHash(h[0] - c.h[0], h[1] - c.h[1]);
	}
	
	bool operator == (const ClusterHash &c) const{
		return h[0] == c.h[0] && h[1] == c.h[1];
	}
	
	bool operator < (const ClusterHash &c) const{
		return h[0] < c.h[0] || (h[0] == c.h[0] && h[1] < c.h[1]);
	}
};

struct ClusterHasher{
	size_t operator()(const ClusterHash &c) const{
		return c.h[0];
	}
};

struct ClusterHashLinkedListNode{
	ClusterHash c;
	ClusterHashLinkedListNode *next = nullptr;
	~ClusterHashLinkedListNode(){
		if (next != nullptr) delete next;
	}
	
	bool operator == (const ClusterHashLinkedListNode &n) const{
		if (this == nullptr && &n == nullptr) return true;
		if (this == nullptr || &n == nullptr) return false;
		return c == n.c && *next == *n.next;
	}
};

struct PartitionHash{
	ClusterHash y, x0, x1;
	//shared_ptr<ClusterHashLinkedListNode> xs;
	
	PartitionHash(){}
	
	PartitionHash(const ClusterHash &y, const ClusterHash &x0, const ClusterHash &x1): y(y), x0(min(x0, x1)), x1(max(x0, x1)){}
	
	PartitionHash(const ClusterHash &y, vector<ClusterHash> &xlist): y(y){
		sort(xlist.begin(), xlist.end());
		x0 = xlist[0];
		x1 = xlist[1];
		/*if (xlist.size() > 2){
			ClusterHashLinkedListNode *p = new ClusterHashLinkedListNode;
			p->c = xlist[2];
			xs.reset(p);
			for (int i = 3; i < xlist.size(); i++){
				p->next = new ClusterHashLinkedListNode;
				p = p->next;
				p->c = xlist[i];
			}
		}*/
	}
	
	bool operator == (const PartitionHash &p) const{
		return y == p.y && x0 == p.x0 && x1 == p.x1  ;//&& *xs == *(p.xs);
	}
	
	size_t hash() const{
		uint64_t res = y.h[0] + x0.h[0] * x0.h[1] + x1.h[0] * x1.h[1];
		/*for (ClusterHashLinkedListNode* p = xs.get(); p != nullptr; p = p->next){
			res += p->c.h[0] * p->c.h[1]; 
		}*/
		return res;
	}
};

struct PartitionHasher{
	size_t operator()(const PartitionHash &p) const{
		uint64_t res = p.y.h[0] + p.x0.h[0] * p.x0.h[1] + p.x1.h[0] * p.x1.h[1];
		/*for (ClusterHashLinkedListNode* q = p.xs.get(); q != nullptr; q = q->next){
			res += q->c.h[0] * q->c.h[1]; 
		}*/
		return res;
	}
};

mt19937_64 generator;
uniform_int_distribution<uint64_t> distribution;

class GenetreeAnnotator{
public:
	struct Polytree{
		vector<ClusterHash> singletons;
		unordered_set<ClusterHash, ClusterHasher> clusters;
		unordered_set<PartitionHash, PartitionHasher> additions;
		unordered_map<PartitionHash, int, PartitionHasher> partitions;
		
		void write(stringstream &fout, const vector<string> &id2name) const{
			fout << singletons.size() << endl;
			for (const string &s: id2name) fout << s << endl;
			unordered_map<ClusterHash, int, ClusterHasher> clusterId;
			int idCounter = 0;
			for (auto &c: singletons) clusterId[c] = idCounter++;
			for (auto &c: clusters){
				if (clusterId.count(c) == 0) clusterId[c] = idCounter++;
			}
			fout << clusterId.size() << "\t" << additions.size() << endl;
			for (auto &p: additions) fout << clusterId[p.y] << "\t" << clusterId[p.x0] << "\t" << clusterId[p.x1] << endl;
			fout << partitions.size() << endl;
			for (auto &p: partitions) fout << clusterId[p.first.y] << "\t" << clusterId[p.first.x0] << "\t" << clusterId[p.first.x1] << "\t" << p.second << endl;
		}
	};
private:
	struct Node{
		int leftChildId = -1, rightChildId = -1;
		DynamicBitset label;
		bool isDuplication = false;
		bool isLeaf = false;
		int score = -1;
		int leafId = -1;
	};
	
	vector<Node> node;
	vector<string> id2name;
	unordered_map<string, int> name2id;
	
	tuple<int, int, int> createSubtree(const unordered_map<long long, string> &leafname, const unordered_map<long long, pair<long long, long long> > &children, const long long cur, vector<int> &rootId){
		if (children.count(cur) == 0){
			int curId = node.size();
			node.emplace_back();
			node[curId].isLeaf = true;
			node[curId].score = 0;
			if (name2id.count(leafname.at(cur)) == 0){
				name2id[leafname.at(cur)] = id2name.size();
				id2name.push_back(leafname.at(cur));
			}
			node[curId].leafId = name2id[leafname.at(cur)];
			node[curId].label.set(name2id[leafname.at(cur)]);
			return make_tuple(curId, -1, -1);
		}
		
		tuple<int, int, int> left = createSubtree(leafname, children, children.at(cur).first, rootId), right = createSubtree(leafname, children, children.at(cur).second, rootId);
		int cur0 = node.size();
		node.emplace_back();
		int cur1 = node.size();
		node.emplace_back();
		int cur2 = node.size();
		node.emplace_back();
		node[cur0].leftChildId = get<0>(left);
		node[cur0].rightChildId = get<0>(right);
		
		node[cur1].leftChildId = get<0>(left);
		node[cur2].leftChildId = get<0>(right);
		if (get<1>(left) != -1) node[get<1>(left)].rightChildId = cur2;
		if (get<2>(left) != -1) node[get<2>(left)].rightChildId = cur2;
		if (get<1>(right) != -1) node[get<1>(right)].rightChildId = cur1;
		if (get<2>(right) != -1) node[get<2>(right)].rightChildId = cur1;
		
		int root1 = node.size();
		node.emplace_back();
		node[root1].leftChildId = get<0>(left);
		node[root1].rightChildId = cur2;
		rootId.push_back(root1);
		int root2 = node.size();
		node.emplace_back();
		node[root2].leftChildId = get<0>(right);
		node[root2].rightChildId = cur1;
		rootId.push_back(root2);
		
		return make_tuple(cur0, cur1, cur2);
	}
	
	int scoreSubtree(int cur){
		if (node[cur].score != -1) return node[cur].score;
		node[cur].score = scoreSubtree(node[cur].leftChildId) + scoreSubtree(node[cur].rightChildId);
		node[cur].label = node[node[cur].leftChildId].label | node[node[cur].rightChildId].label;
		if (!node[node[cur].leftChildId].label.isDisjointTo(node[node[cur].rightChildId].label)){
			node[cur].score++;
			node[cur].isDuplication = true;
			if (node[cur].label != node[node[cur].leftChildId].label) node[cur].score++;
			if (node[cur].label != node[node[cur].rightChildId].label) node[cur].score++;
		}
		return node[cur].score;
	}
	
	static vector<pair<string, int> > merge(const vector<pair<string, int> > &a, const vector<pair<string, int> > &b){
		vector<pair<string, int> > c;
		int i = -1, j = -1;
		vector<int> p, q;
		
		while (i + 1 != a.size() || j + 1 != b.size()){
			
			if (p.size() == 0 && q.size() == 0){
				i++;
				j++;
				if (a[i].second > b[j].second){
					for (int v = b[j].second + 1; v <= a[i].second; v++) p.push_back(v);
					c.push_back({"(" + a[i].first + "," + b[j].first + ")", a[i].second});
				}
				else{
					for (int v = a[i].second + 1; v <= b[j].second; v++) q.push_back(v);
					c.push_back({"(" + a[i].first + "," + b[j].first + ")", b[j].second});
				}
			}
			else if (p.size() != 0){
				i++;
				int r = p.back();
				p.pop_back();
				for (int v = r + 1; v <= a[i].second; v++) p.push_back(v);
				c.push_back({"(" + a[i].first + "," + b[j].first + ")", a[i].second});
			}
			else {
				j++;
				int r = q.back();
				q.pop_back();
				for (int v = r + 1; v <= b[j].second; v++) q.push_back(v);
				c.push_back({"(" + a[i].first + "," + b[j].first + ")", b[j].second});
			}
		}
		return c;
	}
	
	static bool eq(const vector<pair<string, int> > &a, const vector<pair<string, int> > &b){
		if (a.size() != b.size()) return false;
		for (int i = 0; i < a.size(); i++){
			if (a[i].first != b[i].first || a[i].second != b[i].second) return false;
		}
		return true;
	}
	
	pair<vector<pair<string, int> >, vector<pair<string, int> > > breakSubtree(int cur, bool keeplost) const{
		if (node[cur].isLeaf){
			string name = id2name[node[cur].leafId];
			return {vector<pair<string, int> >(1, {name, 0}), vector<pair<string, int> >(1, {name, 0})};
		}
		auto left = breakSubtree(node[cur].leftChildId, keeplost), right = breakSubtree(node[cur].rightChildId, keeplost);
		if (node[cur].isDuplication){
			vector<pair<string, int> > first, second;
			if (eq(left.first, left.second)){
				first = left.first;
			}
			else {
				for (auto &e: left.first) first.push_back({e.first, e.second + 1});
				for (auto &e: left.second) first.push_back({e.first, e.second + 1});
			}
			if (eq(right.first, right.second)){
				second = right.first;
			}
			else {
				for (auto &e: right.first) second.push_back({e.first, e.second + 1});
				for (auto &e: right.second) second.push_back({e.first, e.second + 1});
			}
			if (keeplost){
				string outgroupL, outgroupR;
				bool singleL = true, singleR = true;
				for (int i: (node[cur].label - node[node[cur].leftChildId].label).setBits()){
					if (outgroupL == "") outgroupL = id2name[i];
					else {
						singleL = false;
						outgroupL += "," + id2name[i];
					}
				}
				if (!singleL) outgroupL = "(" + outgroupL + ")";
				for (int i: (node[cur].label - node[node[cur].rightChildId].label).setBits()){
					if (outgroupR == "") outgroupR = id2name[i];
					else {
						singleR = false;
						outgroupR += "," + id2name[i];
					}
				}
				if (!singleR) outgroupR = "(" + outgroupR + ")";
				vector<pair<string, int> > outL(1, {outgroupL, 0}), outR(1, {outgroupR, 0});
				return {outgroupL == "" ? first : merge(first, outL), outgroupR == "" ? second : merge(second, outR)};
			}
			return {first, second};
		}
		else {
			return {merge(left.first, right.first), merge(left.second, right.second)};
		}
	}

	ClusterHash buildPolytreePre(int cur, unordered_map<int, ClusterHash> &nodeCluster, Polytree& pt, bool isRoot = false) const{
		if (node[cur].isLeaf) return nodeCluster[cur] = pt.singletons[node[cur].leafId];
		
		int left = node[cur].leftChildId, right = node[cur].rightChildId;
		if (node[cur].isDuplication){
			ClusterHash lc = buildPolytreePre(left, nodeCluster, pt, isRoot);
			ClusterHash rc = buildPolytreePre(right, nodeCluster, pt, isRoot);
			if (node[left].label == node[cur].label) return nodeCluster[cur] = lc;
			if (node[right].label == node[cur].label) return nodeCluster[cur] = rc;
			ClusterHash c = lc;
			for (int i: (node[cur].label - node[left].label).setBits()){
				ClusterHash nc = c + pt.singletons[i];
				if (!isRoot) pt.clusters.insert(nc);
				if (!isRoot) pt.additions.insert(PartitionHash(nc, c, pt.singletons[i]));
				c = nc;
			}
			return nodeCluster[cur] = c;
		}
		else {
			ClusterHash lc = buildPolytreePre(left, nodeCluster, pt);
			ClusterHash rc = buildPolytreePre(right, nodeCluster, pt);
			ClusterHash c = lc + rc;
			if (!isRoot) pt.clusters.insert(c);
			if (!isRoot) pt.additions.insert(PartitionHash(c, lc, rc));
			return nodeCluster[cur] = c;
		}
	}
	
	void buildPolytreePost(int cur, unordered_map<int, ClusterHash> &nodeCluster, Polytree& pt, const ClusterHash &s, bool isRoot = false) const{
		if (node[cur].isLeaf) return;
		
		int left = node[cur].leftChildId, right = node[cur].rightChildId;
		if (node[cur].isDuplication){
			buildPolytreePost(left, nodeCluster, pt, s, isRoot);
			buildPolytreePost(right, nodeCluster, pt, s, isRoot);
		}
		else {
			ClusterHash lc = nodeCluster[left], rc = nodeCluster[right];
			pt.partitions[PartitionHash(s, lc, rc)]++;
			ClusterHash ls = s + rc;
			pt.clusters.insert(ls);
			if (!isRoot) pt.additions.insert(PartitionHash(ls, s, rc));
			buildPolytreePost(left, nodeCluster, pt, ls);
			ClusterHash rs = s + lc;
			pt.clusters.insert(rs);
			if (!isRoot) pt.additions.insert(PartitionHash(rs, s, lc));
			buildPolytreePost(right, nodeCluster, pt, rs);
		}
	}
		
public:
	const vector<string> &leafnames() const{
		return id2name;
	}
	
	int annotateTree(const unordered_map<long long, string> &leafname, const unordered_map<long long, pair<long long, long long> > &children, const long long root){
		vector<int> rootId;
		tuple<int, int, int> left = createSubtree(leafname, children, children.at(root).first, rootId), right = createSubtree(leafname, children, children.at(root).second, rootId);
		if (get<1>(left) != -1) node[get<1>(left)].rightChildId = get<0>(right);
		if (get<2>(left) != -1) node[get<2>(left)].rightChildId = get<0>(right);
		if (get<1>(right) != -1) node[get<1>(right)].rightChildId = get<0>(left);
		if (get<2>(right) != -1) node[get<2>(right)].rightChildId = get<0>(left);
		
		int curId = node.size();
		node.emplace_back();
		node[curId].leftChildId = get<0>(left);
		node[curId].rightChildId = get<0>(right);
		rootId.push_back(curId);
		
		int bestscore = 999999, bestroot = -1;
		for (int root: rootId){
			int score = scoreSubtree(root);
			if (score < bestscore){
				bestscore = score;
				bestroot = root;
			}
		}
		return bestroot;
	}
	
	vector<pair<string, int> > breakGenetree(int root, bool keeplost = false) const{
		pair<vector<pair<string, int> >, vector<pair<string, int> > > result = breakSubtree(root, keeplost);
		vector<pair<string, int> > ret;
		if (eq(result.first, result.second)){
			for (auto &e: result.first) ret.push_back({e.first + ";", e.second});
		}
		else {
			for (auto &e: result.first) ret.push_back({e.first + ";", e.second + 1});
			for (auto &e: result.second) ret.push_back({e.first + ";", e.second + 1});
		}
		return ret;
	}
	
	void buildPolytree(int root, Polytree& pt) const{
		while (pt.singletons.size() < id2name.size()){
			ClusterHash c(distribution(generator), distribution(generator));
			pt.clusters.insert(c);
			pt.singletons.push_back(c);
		}
		unordered_map<int, ClusterHash> nodeCluster;
		buildPolytreePre(root, nodeCluster, pt, true);
		ClusterHash s;
		pt.clusters.insert(s);
		buildPolytreePost(root, nodeCluster, pt, s, true);
	}
};

unordered_map<string, string> leafname_mapping;
string TEXT;
long long nodecnt = 0;
int pos = 0;

string MAPPING(int begin, int end){
	string s;
	for (int i = begin; i < end && TEXT[i] != ':'; i++){
		if (TEXT[i] != '\"' && TEXT[i] != '\'') s += TEXT[i];
	}
	if (leafname_mapping.count(s)) return leafname_mapping[s];
	else return s;
}

long long parse(unordered_map<long long, string> &leafname, unordered_map<long long, pair<long long, long long> > &children){
	int i = pos;
	long long cur;
	while (TEXT[pos] != '(' && TEXT[pos] != ',') pos++;
	if (TEXT[pos] == '(') {
		pos++;
		cur = parse(leafname, children);
		while (TEXT[pos] != ',') pos++;
	}
	else {
		cur = nodecnt++;
		leafname[cur] = MAPPING(i, pos);
	}
	vector<long long> lst;
	lst.push_back(cur);
	while (TEXT[pos] != ')'){
		i = ++pos;
		while (TEXT[pos] != ')' && TEXT[pos] != '(' && TEXT[pos] != ',') pos++;
		/*
		if (TEXT[pos] == '(') {
			pos++;
			long long left = cur, right = parse(leafname, children);
			cur = nodecnt++;
			children[cur] = {left, right};
			while (TEXT[pos] != ',' && TEXT[pos] != ')') pos++;
		}
		else {
			long long left = cur, right = nodecnt++;
			leafname[right] = MAPPING(i, pos);
			cur = nodecnt++;
			children[cur] = {left, right};
		}
		*/
		long long temp = lst[rand() % lst.size()];
		if (TEXT[pos] == '(') {
			pos++;
			long long left = nodecnt++, right = parse(leafname, children);
			while (TEXT[pos] != ',' && TEXT[pos] != ')') pos++;
			if (leafname.count(temp)) {
				leafname[left] = leafname[temp];
				leafname.erase(temp);
			}
			else children[left] = children[temp];
			children[temp] = {left, right};
			lst.push_back(left);
			lst.push_back(right);
		}
		else {
			long long left = nodecnt++, right = nodecnt++;
			leafname[right] = MAPPING(i, pos);
			if (leafname.count(temp)) {
				leafname[left] = leafname[temp];
				leafname.erase(temp);
			}
			else children[left] = children[temp];
			children[temp] = {left, right};
			lst.push_back(left);
			lst.push_back(right);
		}
	}
	pos++;
	return cur;
}

tuple<string, string, stringstream*> annotate(string input, string mapping) {
	int ti = 3, tx = 3, pw = 4;
	ifstream fcfg("astral-pro.config");
	if (fcfg >> ti){
		fcfg >> tx >> pw;
	}
	cerr << "ti: " << ti << "\ntx: " << tx << "\npw: " << pw << endl;
	if (mapping != ""){
		ifstream fmap(mapping);
		string gname, sname;
		while (fmap >> gname){
			fmap >> sname;
			leafname_mapping[gname] = sname;
		}
	}
	ifstream fin(input);
	string line;
	while (getline(fin, line)) TEXT += line;
	string sout, sx;
	stringstream* spoly = new stringstream();
	GenetreeAnnotator ga;
	GenetreeAnnotator::Polytree pt;
	int cnt = 0, thd = 0, k = 0;
	for (char c: TEXT) if (c == ';') k++;
	if (k > 500) thd = 0;
	else thd = (500 - k) / 4;
	while (pos < TEXT.size()){
		while (pos < TEXT.size() && TEXT[pos] != '(') pos++;
		if (pos < TEXT.size()) {
			int leafCnt = 1, internalCnt = 0;
			for (int i = pos; i < TEXT.size() && TEXT[i] != ';'; i++){
				if (TEXT[i] == '(') internalCnt++;
				if (TEXT[i] == ',') leafCnt++;
			}
			if (internalCnt < leafCnt - 2) {
				cerr << "Non-binary input tree(s) detected!\nCurrently ASTRAL-Pro does not guarentee correct output if input trees contain polytomies!";
				exit(0);
			}
			
			pos++;
			unordered_map<long long, string> leafname;
			unordered_map<long long, pair<long long, long long> > children;
			long long root = parse(leafname, children);
			int iroot = ga.annotateTree(leafname, children, root);
			ga.buildPolytree(iroot, pt);
			cnt++;
			auto trees = ga.breakGenetree(iroot);
			for (auto e: trees) if (count(e.first.begin(), e.first.end(), '(') >= tx) sx += e.first + "\n";
			for (string s: sample(trees, pw)) if (count(s.begin(), s.end(), '(') >= ti) sout += s + "\n";
			if (cnt < thd){
				for (auto e: trees) if (count(e.first.begin(), e.first.end(), '(') >= 3) sout += e.first + "\n";
				auto treeswithoutgroup = ga.breakGenetree(iroot, true);
				for (auto e: treeswithoutgroup) if (count(e.first.begin(), e.first.end(), '(') >= 3) sout += e.first + "\n";
			}
		}
	}
	//cerr << "sx:\n" << sx << "\nsout\n" << sout << "\nspoly\n" << spoly << endl;
	cerr << "The number of input tree read is " << cnt << endl;
	pt.write(*spoly, ga.leafnames());
	return make_tuple(sx, sout, spoly);
}
