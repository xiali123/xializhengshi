import sys
import time
import argparse
import os.path as osp
import torch
import torch.nn.functional as F
from tqdm import *
from scipy.sparse import *
import numpy as np
a = np.array([1,3,2,4,4])#[1,1,2,3,4]]
b = np.array([0,0,1,2,3])
c = 5
# c 使用了关键字参数 sin_array
np.savez("./osdi-ae-graphs/mycielskian3.npz", src_li = a, dst_li = b, num_nodes = c)
