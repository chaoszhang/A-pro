import sys
import dendropy

def printSubtree(v):
	if v in children:
		print(v, children[v][0], children[v][1])
		for c in children[v]:
			printSubtree(c)

gene2species = {}

with open(sys.argv[1], "r") as ins:
	print(len([None for line in ins]))

if len(sys.argv) > 2:
	with open(sys.argv[2], "r") as ins:
		arr = line.split()
		gene2species[arr[0]] = arr[1]

with open(sys.argv[1], "r") as ins:
	for line in ins:
		children = {}
		parent = {}
		name = {}

		tree = dendropy.Tree.get_from_string(line, schema="newick", rooting="force-rooted", suppress_internal_node_taxa=True, suppress_leaf_node_taxa=True)
		if len(tree.seed_node.child_edges()) > 2:
			tree.reroot_at_edge(tree.seed_node.child_edges()[0])

		for p in tree.internal_nodes():
			children[hash(p)] = []
			for c in p.child_node_iter():
				children[hash(p)].append(hash(c))
				parent[hash(c)] = hash(p)

		for v in tree.leaf_nodes():
			s = v.label.replace(' ', '_')
			name[hash(v)] = gene2species[s] if s in gene2species else s
			
		print(len(name))
		print(len(children))
		print(hash(tree.seed_node))
		for n in name:
			print(name[n], n)
		printSubtree(hash(tree.seed_node))
		#print(tree.as_ascii_plot())