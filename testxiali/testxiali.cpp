#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/count.hpp>

#include <torch/extension.h>
#include <vector>
#include <string.h>
#include <cstdlib>
#include <map>


#define CHECK_CONTIGUOUS(x) TORCH_CHECK(x.is_contiguous(), #x " must be contiguous")
#define CHECK_INPUT(x) CHECK_CONTIGUOUS(x)


typedef struct
{
    int id;
    int sid;
    int num;
}VexNode;

struct {
    bool operator()(VexNode a, VexNode b) const
    {
        int t = a.num;
        int k = b.num;
        return t - k;
    }
}VexHTCmp;

std::vector<torch::Tensor> reorder(
    torch::Tensor degrees,
    torch::Tensor in_edge_index
)
{
    CHECK_INPUT(in_edge_index);
    CHECK_INPUT(degrees);
    const int numedges = in_edge_index.size(1);
    const int numvexs = degrees.size(0);
    int start = 0;

    auto vex_hash_table_ptr = degrees.accessor<int, 1>();
    auto in_edge_index_ptr = in_edge_index.accessor<int, 2>();
    std::vector<VexNode> Vexs;

    for(int i = start; i < numvexs; i++)
    {
        VexNode vex;
        vex.sid = i;
        vex.num = vex_hash_table_ptr[i];
        Vexs.push_back(vex);
    }

    sort(Vexs.begin(), Vexs.end(), VexHTCmp);
    std::map<int, int> vexMap;
    std::map<int, int> vexNum;

    torch::Tensor out_hash_table = torch::zeros_like(degrees);
    torch::Tensor out_edge_index = torch::zeros_like(in_edge_index);

    auto out_edge_index_ptr = out_edge_index.accessor<int, 2>();
    auto out_hash_table_ptr = out_hash_table.accessor<int, 1>();

    vexMap[Vexs[start].sid] = start;
    Vexs[start].id = start;
    for(int i = start + 1; i < numvexs; i++)
    {
        Vexs[i].id = i;
        out_hash_table_ptr[Vexs[i].sid] = i;
        vexMap[Vexs[i].sid] = Vexs[i-1].num + vexMap[Vexs[i-1].sid];
        vexNum[Vexs[i].sid] = 0;
    }

    for(int i = start; i < numedges; i++)
    {
        int src = in_edge_index_ptr[0][i];
        int dst = in_edge_index_ptr[1][i];
        int tag1 = vexMap[src];
        int tag2 = vexNum[src];
        out_edge_index_ptr[0][tag1+tag2] = src;
        out_edge_index_ptr[1][tag1+tag2] = dst;
        (vexNum[src])++;
    }
    return {out_edge_index, out_hash_table};
}

/*
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

*/
// binding to python
PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) {
  m.def("reorder", &reorder, "Get the reordered node id mapping: old_id --> new_id");
}