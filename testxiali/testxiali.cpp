#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/count.hpp>

#include <torch/extension.h>
#include <vector>
#include <string.h>
#include <cstdlib>
#include <map>


#define CHECK_CONTIGUOUS(x) TORCH_CHECK(x.is_contiguous(), #x " must be contiguous")
#define CHECK_INPUT(x) CHECK_CONTIGUOUS(x)

const int K = 10000;
const int MARK = 32;                                    //初始标记
const int STDNNZ = 10000;                               //非零元个数阈值

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

//求中位数
int getMedium(auto input_data_ptr, auto hash_ptr, int num_edges, int num_vexs)
{
    int i;
    int half_num = num_edges / 2;
    int sum = 0;
    for(i = 0; i < num_vexs; i++)
    {
        sum += input_data_ptr[hash_ptr[i]];
        if(sum > half_num) break;
    }

    return i;
}

/*
//求方差
double getD(auto input_data_ptr, auto hash_ptr, int num_mean, int left, int right)
{
    double sum = 0;
    for(int i = start; i < right; i++)
    {
        double tag = input_data_ptr[hash_ptr[i]] - num_mean;
        sum = sum + (tag*tag);
    }
    return sum;
}


//求一次导和二次导（一次差分和二次差分用于替代作一次导和二次导）
int getSlope(
    auto Slope_ptr,
    auto input_data_ptr,
    auto hash_ptr,
    int num_edges,
    int num_vexs
)
{
    if(num_vexs <= 0) return 0;
    for(int i = 1; i < num_vexs; i++)
    {
        int fst = input_data_ptr[hash_ptr[i-1]];
        int scd = input_data_ptr[hash_ptr[i]];
        int dx = scd - fst;
        Slope_ptr[0][i] = dx;
        if(i > 1)
        {
            fst = Slope_ptr[0][i-1];
            scd = Slope_ptr[0][i];
            int dx2 = scd - fst;
            Slope_ptr[1][i] = dx2;
        }
    }

    return 1;
}
*/

void partition(
    auto marks_vct_ptr,
    int *sums,
    int num_vexs,
    int mark_size
)
{
    int nnz_tag = 0;
    for(int i = 0; i < mark_size; i++)
    {

    }
}

//分流框架
torch::Tensor AutoFrameWork(
    torch::Tensor input_data,
    torch::Tensor hash_table,
    int num_edges,
    int num_vexs,
    int num_mean
)
{
    CHECK_INPUT(input_data);
    const int numvexs = degrees.size(0);
    auto input_data_ptr = input_data.accessor<int, 1>();            //访问器，所以input_data必须要求是连续的
    auto hash_table_ptr = hash_table.accessor<int, 1>();
    torch::Tensor out_data_put = torch::zeros_like(input_data);

    //(判断分区)看看是否需要分区（以及大概分几个区)
    if(num_vexs > STDNNZ)
    {
        //(标定)(用于不同算法的分类问题)
        std::vector<int> marks_vct;
        int sums[num_vexs+1];
        int j = MARK;
        sums[0] = 0;
        for(int i = 0; i < num_vexs; i++)
        {
            int tag = input_data_ptr[hash_table_ptr[i]];
            sums[i+1] = sums[i] + tag;
            if(tag == j)
            {
                marks_vct.hash_table(i);
                j <<= 1;
            }
        }

        auto marks_vct_ptr = marks_vct.accessor<int, 1>();
        int mark_vct_size = mark_vct.size();
        //分区


    }


    //(第一层分区)按线程块线程能处理的边数检查

    //(第二层分区)


    return out_data_put;
}

//排序算法
std::vector<torch::Tensor> reorder(
    torch::Tensor degrees,
    torch::Tensor in_edge_index
    int num_edges,
    int num_vexs
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

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) {
  m.def("reorder", &reorder, "Get the reordered node id mapping: old_id --> new_id");
}


