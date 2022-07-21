from setuptools import setup
from torch.utils.cpp_extension import BuildExtension, CppExtension

setup(
    name='vexorder',
    ext_modules=[
        CppExtension(
            name='vexorder',
            sources=[
            'testxiali.cpp'
            ],
            extra_compile_args=['-O3', '-fopenmp', '-mcx16'],
            libraries=["numa", "tcmalloc_minimal"]
         )
    ],
    cmdclass={
        'build_ext': BuildExtension
    })

