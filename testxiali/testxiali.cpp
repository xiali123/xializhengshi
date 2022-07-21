#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/count.hpp>

#include <torch/extension.h>
#include <vector>
#include <string.h>
#include <cstdlib>
#include <map>


#define CHECK_CONTIGUOUS(x) TORCH_CHECK(x.is_contiguous(), #x " must be contiguous")
#define CHECK_INPUT(x) CHECK_CONTIGUOUS(x)

struct {
    bool operator()(std::tuple<int, int, int> a, std::tuple<int, int, int> b) const
    {
        int t = std::get<2>(a);
        int k = std::get<2>(b);
        return t - k;
    }
} customLess;


torch::Tensor reorder(
    torch::Tensor degrees,
    torch::Tensor in_edge_index
) {
  int src, dst;

  using boost::adaptors::transformed;
  std::cerr << "Number of threads: " << omp_get_max_threads() << std::endl;

  CHECK_INPUT(in_edge_index);
  CHECK_INPUT(degrees);

  const int numedges = in_edge_index.size(1);
  // const int dim0 = in_edge_index.size(1);
  // vint is size_t
  std::vector<std::tuple<int, int, int>> edges;
  auto in_edge_index_ptr = in_edge_index.accessor<int, 2>();
  auto tag_degrees_ptr = degrees.accessor<int, 1>();
  // prepare the input
  for(int i = 0; i < numedges; i++){
    src = in_edge_index_ptr[0][i];
    dst = in_edge_index_ptr[1][i];
    edges.push_back(std::make_tuple(src, dst, tag_degrees_ptr[src]));
  }

  sort(edges.begin(), edges.end(), customLess);

  torch::Tensor out_edge_index = torch::zeros_like(in_edge_index);
  auto out_edge_index_ptr = out_edge_index.accessor<int, 2>();


  // generate the edge_list.
  for(int i = 0; i < numedges; i++){
    src = std::get<0>(edges[i]);
    dst = std::get<1>(edges[i]);
    out_edge_index_ptr[0][i] = src;
    out_edge_index_ptr[1][i] = dst;
  }

  return out_edge_index;
}


// binding to python
PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) {
  m.def("reorder", &reorder, "Get the reordered node id mapping: old_id --> new_id");
}