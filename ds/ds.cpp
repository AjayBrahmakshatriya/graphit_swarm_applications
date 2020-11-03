
#include "graph.h"
#include "pls/pls_api.h"
#include "pls/algorithm.h"


float *new_ranks;
float *old_ranks;
float *contrib;
GraphT<int> edges;
float beta_score;
float damp = 0.85;



__attribute__((noinline, swarmify, assertswarmified))
static void pr_scc(pls::Timestamp, const int s) {
	for (int round = 0; round < 20;  round++) {
		for (int i = 0; i < edges.num_vertices; i++) {
			contrib[i] = old_ranks[i] / edges.h_get_degree(i);
		}
		for (int i = 0; i < edges.num_vertices; i++) {
			for (int j = edges.h_src_offsets[i]; j < edges.h_src_offsets[i+1]; j++) {
				// DO a PUSH
				int32_t dst = edges.h_edge_dst[j];
				new_ranks[dst] += contrib[i];
			}
		}	
		for (int i = 0; i < edges.num_vertices; i++) {
			new_ranks[i] = beta_score + damp * new_ranks[i];
			old_ranks[i] = new_ranks[i];
			new_ranks[i] = 0.0;
		}
	}
}
int main(int argc, char* argv[]) {
	load_graph(edges, argv[1], false);
	std::cout << edges.num_vertices << std::endl;
	std::cout << edges.num_edges << std::endl;
	
	beta_score = (1.0 - damp)/edges.num_vertices;

	new_ranks = new float[edges.num_vertices];
	old_ranks = new float[edges.num_vertices];
	contrib = new float[edges.num_vertices];
	
	for (int i = 0; i < edges.num_vertices; i++) {
		new_ranks[i] = 0;
		old_ranks[i] = 1.0/edges.num_vertices;
	}	

	pls::enqueue(pr_scc, 0, NOHINT, 0);
	pls::run();

	for (int i = 0; i < edges.num_vertices; i++) {
		std::cout << i << ", " << old_ranks[i] << std::endl;
	}
}
