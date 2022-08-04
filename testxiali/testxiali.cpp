#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/count.hpp>

#include <torch/extension.h>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <cstdlib>
#include <map>


#define CHECK_CONTIGUOUS(x) TORCH_CHECK(x.is_contiguous(), #x " must be contiguous")
#define CHECK_INPUT(x) CHECK_CONTIGUOUS(x)

const int K = 10000;
const int MARK = 1;                                    //初始标记
const int STDNNZ = 10000;                               //非零元个数阈值

typedef struct
{
    int id;
    int sid;
    int num;
}VexNode;

typedef struct
{
    int index;
    int count;
}ParNode;

typedef struct
{
    int start;
    int end;
    int count;
}TableNode;

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

//求方差

//划分,因为有多流，所以按照窗口划分没问题
int partition(
    auto marks_vct_ptr,
    int **record_ptr,
    int *rcecord_size,
    int mark_size,
    int num_vexs,
    int win_size,
    int stdnnz
)
{
    //edgenum == 1单独处理
    int start = 0;
    std::vector<TableNode> tagPars;
    TableNode tagNode;
    int tag_num = marks_vct_ptr[0][1];
    if(stdnnz < tag_num)
    {
        tagNode.start = marks_vct_ptr[0][0];
        tagNode.end = (mark_size == 1)? num_vexs-1:marks_vct_ptr[1][0] - 1;
        tagNode.count = marks_vct_ptr[0][1];
        tagPars.push_back(tagNode);
        start = 1;
    }
    int end = mark_size;
    int tag = 0;
    for(int i = start; i < end; i++)
    {
        if(i % win_size == start && tag > stdnnz)
        {
            tagNode.end = marks_vct_ptr[i][0] - 1;
            tagNode.count = tag;
            tag = 0;
            tagPars.push_back(tagNode);
            tagNode.start = marks_vct_ptr[i][0];

        }

        tag += marks_vct_ptr[i+j][1];
    }

    if(tag > stdnnz)
    {
        tagNode.end = marks_vct_ptr[i-1][0] - 1;
        tagNode.count = tag;
        tagPars.push_back(tagNode);
    }
    else
    {
        TableNode *tag_ptr = tagPars.back();
        tag_ptr->count += tag;
        tag_ptr->end = end-1;
    }

    record_ptr = (int **)malloc(sizeof(int*)*tagNode.size());
    for(int i = 0; i < tagNode.size(); i++)
    {
        record_ptr[i] = (int*)malloc(sizeof(int)*4);
        record_ptr[i].start = tagNode[i].start;
        record_ptr[i].end = tagNode[i].end;
        record_ptr[i].count = tagNode[i].count;
    }

    return 1;
}

//划分记录
int partition_table(
    int **record_ptr,
    int **table,
    int win_size,
    int stdnnz
)
{

    return 1;
}

int getWinSize(

)
{

}

//分流框架
torch::Tensor AutoFrameWork(
    torch::Tensor input_data,
    torch::Tensor hash_table,
    int *sums,
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

    //(标定并记录非零元个数)(用于不同算法的分类问题)
    std::vector<ParNode> marks_vct;
    int j = MARK;
    for(int i = 0; i < num_vexs; i++)
    {
        ParNode tagVal;
        int tag = input_data_ptr[hash_table_ptr[i]];
        if(tag == j)
        {
            tagVal.index = hash_table[i];
            tagVal.count = 0;
            ParNode *par_ptr = marks_vct.back();
            if(par_ptr) par_ptr->count = sums[i] - sums[par_ptr->index];
            marks_vct.push_back(tagVal);
            j <<= 1;
        }
    }

    ParNode *par_ptr = marks_vct.back();
    if(par_ptr) par_ptr->count = sums[num_vexs] - sums[par_ptr->index];

    //划分
    int win_size = getWinSize();                               //还需要自动调整（可以自动调整更新）
    auto marks_vct_ptr = marks_vct.accessor<int, 2>();
    int **record_ptr;
    int record_size;
    partition(marks_vct_ptr, record_ptr, record_size, marks_vct.size(), num_vexs, win_size, stdnnz);

    for(int i = 0; i < record_size; i++)
    {
        setMostMin(input_data_ptr, hash_ptr, record_ptr, record_size, start, end, count);
    }

)

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


