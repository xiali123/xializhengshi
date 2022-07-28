from setuptools import setup
from torch.utils.cpp_extension import BuildExtension, CUDAExtension


setup(
    name='vexorder',
    ext_modules=[
        CUDAExtension(
        name='vexorder',
        sources=[
                    'testxiali.cpp',
                    'testxiali_cuda.cu'
                ]
        )
    ],
    cmdclass={
        'build_ext': BuildExtension
    })
