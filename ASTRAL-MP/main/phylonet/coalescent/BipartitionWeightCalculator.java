package phylonet.coalescent;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.Iterator;
import java.util.List;

import phylonet.tree.model.Tree;
import phylonet.tree.model.sti.STITreeCluster;

// TODO: why extend the abstract? It doesn't seem to follow the same pattern exactly
class BipartitionWeightCalculator extends AbstractWeightCalculatorConsumer<Tripartition> {

	WQInference inference;
	private WQDataCollection dataCollection;
	private int[] geneTreesAsInts;

	public BipartitionWeightCalculator(AbstractInference<Tripartition> inference,
			int[] geneAsInts) {
		super(false, null);
		this.dataCollection = (WQDataCollection) inference.dataCollection;
		this.inference = (WQInference) inference;
		this.geneTreesAsInts = geneAsInts;
	}

	
	class Intersects {
		long s0;
		long s1;
		long s2;
		long s3;


		public Intersects(long s0, long s1, long s2, long s3) {
			this.s0 = s0;
			this.s1 = s1;
			this.s2 = s2;
			this.s3 = s3;
		}
		
        public Intersects(Intersects side1, Intersects side2) {
            this(side1.s0+side2.s0,
					side1.s1+side2.s1,
					side1.s2+side2.s2,
					side1.s3+side2.s3);               
        }	

		public Intersects(Intersects other) {
			this.s0 = other.s0;
			this.s1 = other.s1;
			this.s2 = other.s2;
			this.s3 = other.s3;
		}

		public void addin(Intersects pop) {
			this.s0 += pop.s0;
			this.s1 += pop.s1;
			this.s2 += pop.s2;  
			this.s3 += pop.s3;
		}

		public void subtract(Intersects pop) {
			this.s0 -= pop.s0;
			this.s1 -= pop.s1;
			this.s2 -= pop.s2;         
			this.s3 -= pop.s3;   
		}

		public String toString() {
			return this.s0+","+this.s1+"|"+this.s2+","+this.s3;
		}
		
        public boolean isNotEmpty() {
        	return (this.s0 + this.s1 + this.s2 + this.s3) != 0;
        }
        
        public boolean hasEmpty() {
        	return this.maxPossible() == 0;
        }
        
        public long maxPossible() {
        	return (this.s0 * this.s1 * this.s2 * this.s3);
        }
	}

	private long allcases(Intersects side1, Intersects side2, Intersects side3) {
		return F(side1.s0,side2.s1,side3.s2,side3.s3)+
				F(side1.s1,side2.s0,side3.s2,side3.s3)+
				F(side1.s2,side2.s3,side3.s0,side3.s1)+
				F(side1.s3,side2.s2,side3.s0,side3.s1)+
				F(side3.s0,side2.s1,side1.s2,side1.s3)+
				F(side3.s1,side2.s0,side1.s2,side1.s3)+
				F(side3.s2,side2.s3,side1.s0,side1.s1)+
				F(side3.s3,side2.s2,side1.s0,side1.s1)+
				F(side1.s0,side3.s1,side2.s2,side2.s3)+
				F(side1.s1,side3.s0,side2.s2,side2.s3)+
				F(side1.s2,side3.s3,side2.s0,side2.s1)+
				F(side1.s3,side3.s2,side2.s0,side2.s1);
	}

	Intersects getSide(int i, Quadrapartition quart) {
		if (quart.cluster1.getBitSet().get(i)) {
			return new Intersects(1,0,0,0);
		} else if (quart.cluster2.getBitSet().get(i)) {
			return new Intersects(0,1,0,0);
		} else if (quart.cluster3.getBitSet().get(i)) {
			return  new Intersects(0,0,1,0);
		} else if (quart.cluster4.getBitSet().get(i)) {
			return  new Intersects(0,0,0,1);
		}
		else {
			return  new Intersects(0,0,0,0);
		}
	}
	
	class Results {
		double [] qs;
		int effn;
		
		Results (double [] q, int n){
			qs = q;
			effn = n;
		}
	}

	public Results getWeight(Quadrapartition [] quad ) {
		long [] fi = {0l,0l,0l};
		double [] weight = {0l,0l,0l};
		int effectiven = 0;
		
		for (int i=0; i<3; i++){
			fi[i] = Polytree.PTNative.cppSupport(quad[i].cluster1.getBitSet().words, quad[i].cluster2.getBitSet().words,
				quad[i].cluster3.getBitSet().words, quad[i].cluster4.getBitSet().words);
		}
		long q = (quad[0].cluster1.getClusterSize() * quad[0].cluster2.getClusterSize() * quad[0].cluster3.getClusterSize() * quad[0].cluster4.getClusterSize());
		effectiven = (int)((fi[0] + fi[1] + fi[2]) / q);
		
		/*for (STITreeCluster s: dataCollection.treeAllClusters){
			if (quad[0].cluster1.getBitSet().intersectionSize(s.getBitSet()) == 0) continue;
			if (quad[0].cluster2.getBitSet().intersectionSize(s.getBitSet()) == 0) continue;
			if (quad[0].cluster3.getBitSet().intersectionSize(s.getBitSet()) == 0) continue;
			if (quad[0].cluster4.getBitSet().intersectionSize(s.getBitSet()) == 0) continue;
			effectiven++;
		}
		effectiven /= 16;
		*/
		
		for (int i=0; i<3; i++){
			weight[i] = (fi[i] + 0.0) / q;
		}
		return new Results(weight, effectiven);
	}
	
/*	private boolean checkFutileCalcs(Intersects side1, Intersects side2) {
		return ((side1.s0+side2.s0 == 0? 1 :0) +
    			(side1.s1+side2.s1 == 0? 1 :0) + 
    			(side1.s2+side2.s2 == 0? 1:0) > 1);
	}*/
	

	private long F(long a,long b,long c, long d) {
		if (a<0 || b<0 || c<0|| d<0) {
			throw new RuntimeException("negative side not expected: "+a+" "+b+" "+c);
		}
		return a*b*c*d;
	}	

	class Quadrapartition {

		STITreeCluster cluster1;
		STITreeCluster cluster2;	
		STITreeCluster cluster3;
		STITreeCluster cluster4;
		private int _hash = 0;


		public Quadrapartition(STITreeCluster c1, STITreeCluster c2, STITreeCluster c3,STITreeCluster c4) {

			initialize(c1, c2, c3, c4);
		}
		
		private void initialize(STITreeCluster c1, STITreeCluster c2,
                STITreeCluster c3, STITreeCluster c4) {
            if (c1 == null || c2 == null || c3 == null) {
                throw new RuntimeException("none cluster" +c1+" "+c2+" "+c3);
            }
            int n1 = c1.getBitSet().nextSetBit(0), n2 = c2.getBitSet().nextSetBit(0), 
                    n3 = c3.getBitSet().nextSetBit(0), n4=c4.getBitSet().nextSetBit(0);
            int ntg1;
            int ntg2;
            STITreeCluster cluster_tmp1;
            STITreeCluster cluster_tmp2;    
            STITreeCluster cluster_tmp3;
            STITreeCluster cluster_tmp4;
            if (n1 < n2 ) {
                ntg1 = n1;
                cluster_tmp1 = c1;
                cluster_tmp2 = c2;
            }
            else {
                ntg1 = n2;
                cluster_tmp1 = c2;
                cluster_tmp2 = c1;
            }
            if (n3<n4) {
                ntg2 = n3;
                cluster_tmp3 = c3;
                cluster_tmp4 = c4;
            }
            else {
                ntg2 = n4;
                cluster_tmp3 = c4;
                cluster_tmp4 = c3;
            }
            
            if(ntg1<ntg2){
                cluster1 = cluster_tmp1;
                cluster2 = cluster_tmp2;
                cluster3 = cluster_tmp3;
                cluster4 = cluster_tmp4;
            }
            else{
                cluster1 = cluster_tmp3;
                cluster2 = cluster_tmp4;
                cluster3 = cluster_tmp1;
                cluster4 = cluster_tmp2;
            }
        }

		@Override
		public boolean equals(Object obj) {
			Quadrapartition trip = (Quadrapartition) obj; 

			return this == obj ||
					((trip.cluster1.equals(this.cluster1) && 
							trip.cluster2.equals(this.cluster2) && 
							trip.cluster4.equals(this.cluster4) && 
							trip.cluster3.equals(this.cluster3)));					
		}
		@Override
		public int hashCode() {
			if (_hash == 0) {
				_hash = cluster1.hashCode() * cluster2.hashCode()
						* cluster4.hashCode() * cluster3.hashCode();
			}
			return _hash;
		}
		@Override
		public String toString() {		
			return cluster1.getBitSet().toString2()+"|"+cluster2.getBitSet().toString2()+
					"#"+cluster3.getBitSet().toString2()+"|"+cluster4.getBitSet().toString2();
		}
		public String toString2() {
			return cluster1.toString()+"|"+cluster2.toString()+
					"#"+cluster3.toString()+ "|"+cluster4.toString();
		}


	}


	@Override
	public void preCalculateWeights(List<Tree> trees, List<Tree> extraTrees) {
		// TODO Auto-generated method stub

	}
	
	@Override
	protected Long[] calculateWeight(Tripartition[] t) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	Tripartition[] convertToSingletonArray(Tripartition t) {
		return new Tripartition[]{t};
	}

}