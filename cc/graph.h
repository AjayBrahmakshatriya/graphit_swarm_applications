#ifndef GPU_GRAPH_H
#define GPU_GRAPH_H

#include <assert.h>
#include <iostream>
#include <cstring>
typedef int int32_t;

template <typename EdgeWeightType>
struct GraphT { // Field names are according to CSR, reuse for CSC
	typedef EdgeWeightType EdgeWeightT;
	int32_t num_vertices;
	int32_t num_edges;

	// Host pointers
	int32_t *h_src_offsets; // num_vertices + 1;
	int32_t *h_edge_src; // num_edges;
	int32_t *h_edge_dst; // num_edges;
	EdgeWeightType *h_edge_weight; // num_edges;


	int32_t h_get_degree(int32_t vertex_id) {
		return h_src_offsets[vertex_id + 1] - h_src_offsets[vertex_id];
	}
		
};
void consume(int32_t _) {
}
#define CONSUME consume
template <typename EdgeWeightType>
void static sort_with_degree(GraphT<EdgeWeightType> &graph) {
	assert(false && "Sort with degree not yet implemented\n");
	return;
}
static bool string_ends_with(const char* str, const char* sub_str) {
	if (strlen(sub_str) > strlen(str))
		return false;
	int32_t len1 = strlen(str);
	int32_t len2 = strlen(sub_str);
	if (strcmp(str + len1 - len2, sub_str) == 0)
		return true;
	return false;
}

static int32_t identify_block_id (int32_t vid, int32_t blocking_size) {
	return vid / blocking_size;
}


template <typename EdgeWeightType>
static void load_graph(GraphT<EdgeWeightType> &graph, std::string filename, bool to_sort = false) {
	int flen = strlen(filename.c_str());
	const char* bin_extension = to_sort?".graphit_sbin":".graphit_bin";
	char bin_filename[1024];
	strcpy(bin_filename, filename.c_str());

	if (string_ends_with(filename.c_str(), bin_extension) == false)	 {
		strcat(bin_filename, ".");
		strcat(bin_filename, typeid(EdgeWeightType).name());
		strcat(bin_filename, bin_extension);
	}
	
	FILE *bin_file = fopen(bin_filename, "rb");
	if (!bin_file && string_ends_with(filename.c_str(), bin_extension)) {
		std::cout << "Binary file not found" << std::endl;
		exit(-1);
	}
	if (bin_file) {
		CONSUME(fread(&graph.num_vertices, sizeof(int32_t), 1, bin_file));
		CONSUME(fread(&graph.num_edges, sizeof(int32_t), 1, bin_file));
		
		graph.h_edge_src = new int32_t[graph.num_edges];
		graph.h_edge_dst = new int32_t[graph.num_edges];
		graph.h_edge_weight = new EdgeWeightType[graph.num_edges];
		
		graph.h_src_offsets = new int32_t[graph.num_vertices + 1];
		
		CONSUME(fread(graph.h_edge_src, sizeof(int32_t), graph.num_edges, bin_file));
		CONSUME(fread(graph.h_edge_dst, sizeof(int32_t), graph.num_edges, bin_file));
		CONSUME(fread(graph.h_edge_weight, sizeof(EdgeWeightType), graph.num_edges, bin_file));

		CONSUME(fread(graph.h_src_offsets, sizeof(int32_t), graph.num_vertices + 1, bin_file));
		fclose(bin_file);	
	} else {
		assert(false && "Only bin files are supported\n");
	}


}

template <typename EdgeWeightType>
static int32_t builtin_getVertices(GraphT<EdgeWeightType> &graph) {
	return graph.num_vertices;
}


#endif
