
#include "graph.h"
#include "pls/pls_api.h"
#include "pls/algorithm.h"

#include "thread_local_queues.h"


int32_t* ID;
GraphT<int> edges;

pls::MultiQueue<unsigned> frontiers;

__attribute__((noinline, swarmify, assertswarmified))
static void cc_scc(pls::Timestamp, const int s) {
	frontiers.init(edges.num_vertices);
	for (int i = 0; i < edges.num_vertices; i++)
		frontiers.push(0, i);

	int round = 0;
	while (!frontiers.empty(round)) {
		frontiers.for_each(round, [round] (unsigned current) {
			int32_t edgeZero = edges.h_src_offsets[current];
			int32_t edgeLast = edges.h_src_offsets[current+1];
			for (int i = edgeZero; i < edgeLast; i++) {
				int ngh = edges.h_edge_dst[i];
				// Apply the UDF
				int src_id = ID[current];
				int dst_id = ID[ngh];
				if (ID[src_id] < ID[dst_id]) {
					ID[dst_id] = ID[src_id];
					frontiers.push(round+1, dst_id);
				}
				if (ID[dst_id] < ID[src_id]) {
					ID[src_id] = ID[dst_id];
					frontiers.push(round+1, src_id);
				}
			}
			int update = 1;
			while (update) {
				int local_update = 0;
				for (int i = 0; i < edges.num_vertices; i++) {
					// APPLY UDF
					int y = ID[i];
					int x = ID[y];
					if (x != y) {
						ID[i] = x;
						local_update = 1;
					}
				}		
				if (local_update != 1)
					update = 0;
			}
		});
		round++;
	}	
}

struct applyEdge {
	int &round;
	applyEdge(int &round_ref): round(round_ref) {}

	void operator() (int src, int dst) {
		int src_id = ID[src];
		int dst_id = ID[dst];
		if (ID[src_id] < ID[dst_id]) {
			ID[dst_id] = ID[src_id];
			frontiers.push(round+1, dst_id);
		}
		if (ID[dst_id] < ID[src_id]) {
			ID[src_id] = ID[dst_id];
			frontiers.push(round+1, src_id);
		}
	}
	
};
struct applyPjump {
	int &local_update;
	applyPjump(int &local_update_ref): local_update(local_update_ref) {}
	
	void operator() (int i) {
		int y = ID[i];
		int x = ID[y];
		if (x != y) {
			ID[i] = x;
			local_update = 1;
		}	
	}
};

__attribute__((noinline, swarmify, assertswarmified))
static void cc_scc_functor(pls::Timestamp, const int s) {
	frontiers.init(edges.num_vertices);
	for (int i = 0; i < edges.num_vertices; i++)
		frontiers.push(0, i);

	int round = 0;
	while (!frontiers.empty(round)) {
		frontiers.for_each(round, [round] (unsigned current) {	
			int round_c = round;
			struct applyEdge func(round_c);
			int32_t edgeZero = edges.h_src_offsets[current];
			int32_t edgeLast = edges.h_src_offsets[current+1];
			
			for (int i = edgeZero; i < edgeLast; i++) {
				int ngh = edges.h_edge_dst[i];
				// Apply the UDF
				func(current, ngh);
			}
			int update = 1;
			while (update) {
				int local_update = 0;
				struct applyPjump func_pjump(local_update);	
				for (int i = 0; i < edges.num_vertices; i++) {
					// Apply the UDF
					func_pjump(i);
				}		
				if (local_update != 1)
					update = 0;
			}
		});
		round++;
	}	
}
int main(int argc, char* argv[]) {
	load_graph(edges, argv[1], false);
	std::cout << edges.num_vertices << std::endl;
	std::cout << edges.num_edges << std::endl;	

	ID = new int32_t[edges.num_vertices];
	
	for (int i = 0; i < edges.num_vertices; i++) {
		ID[i] = i;
	}	

#ifdef USE_FUNCTOR
	pls::enqueue(cc_scc_functor, 0, NOHINT, 0);
#else
	pls::enqueue(cc_scc, 0, NOHINT, 0);
#endif
	pls::run();

	for (int i = 0; i < edges.num_vertices; i++) {
		std::cout << i << ", " << ID[i] << std::endl;
	}
}
