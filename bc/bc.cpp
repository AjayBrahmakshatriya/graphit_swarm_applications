
#include "graph.h"
#include "pls/pls_api.h"
#include "pls/algorithm.h"
#include "thread_local_queues.h"
#include <vector>


int32_t* num_paths;
float * dependences;
bool * visited;

GraphT<int> edges;

pls::MultiQueue<unsigned> frontiers;

std::vector<pls::UnorderedQueue<int>> frontier_list;

__attribute__((noinline, swarmify, assertswarmified))
static void bc_scc(pls::Timestamp, const int s) {
	frontiers.init(edges.num_vertices);
	frontiers.push(0, s);
	visited[s] = true;
	num_paths[s] = 1;

	int round = 0;
	while (!frontiers.empty(round)) {
		//frontier_list[round] = std::vector<int>();
		frontier_list[round].init(edges.num_vertices);
		frontiers.for_each(round, [round] (unsigned current) {
			frontier_list[round].push(current);
			int32_t edgeZero = edges.h_src_offsets[current];
			int32_t edgeLast = edges.h_src_offsets[current+1];
			for (int i = edgeZero; i < edgeLast; i++) {
				int ngh = edges.h_edge_dst[i];
				// Apply the UDF
				if (visited[ngh] == false) {
					visited[ngh] = true;
					frontiers.push(round+1, ngh);
					num_paths[ngh] += num_paths[current];
				}
			}
		});	
		round++;
	}	
	
	// We need a transposed graph here, but for now just use a symmetric graph for correctness

	for (int i = 0; i < edges.num_vertices; i++) 
		visited[i] = false;		
	
	// Pop off the empty frontier
	round--;

	int32_t * current_frontier = new int32_t[edges.num_vertices];
		
	while (round >= 0) {
		int total = frontier_list[round].materialize(current_frontier);
		round--;
		for (int i = 0; i < total; i++) {
			int32_t current = current_frontier[i];
			visited[current] = true;	
    			dependences[current] += 1 / (double)num_paths[current];
		}
		for (int i = 0; i < total; i++) {
			int32_t current = current_frontier[i];
			int32_t edgeZero = edges.h_src_offsets[current];
			int32_t edgeLast = edges.h_src_offsets[current+1];
			for (int j = edgeZero; j < edgeLast; j++) {
				int ngh = edges.h_edge_dst[i];
				if (visited[ngh] == false) {
    					dependences[ngh] += dependences[current];
				}	
			}
		}	
	}
	
	for (int i = 0; i < edges.num_vertices; i++) {
		if (num_paths[i] != 0)
			dependences[i] = (dependences[i] - 1 / num_paths[i]) * num_paths[i];
		else
			dependences[i] = 0;
	}
		
}

int main(int argc, char* argv[]) {
	load_graph(edges, argv[1], false);
	std::cout << edges.num_vertices << std::endl;
	std::cout << edges.num_edges << std::endl;	

	visited = new bool[edges.num_vertices];
	num_paths = new int[edges.num_vertices];
	dependences = new float[edges.num_vertices];
	
	for (int i = 0; i < edges.num_vertices; i++) {
		visited[i] = false;
	}	

#ifdef USE_FUNCTOR
#else
	pls::enqueue(bc_scc, 0, NOHINT, 0);
#endif
	pls::run();

	for (int i = 0; i < edges.num_vertices; i++) {
		std::cout << i << ", " << dependences[i] << std::endl;
	}
/*
	for (int i = 0; i < frontier_list.size(); i++) {
		std::cout << frontier_list[i].size() << std::endl;
	}
*/
}
