import os

from setuptools import setup

setup(
    name="zxcvbncpp",
    version='1.0.1',
    description='Password strength estimator',
    author='Rian Hunter',
    author_email='rian@alum.mit.edu',
    url='https://github.com/rianhunter/zxcvbn-cpp',
    package_dir={'' : 'python-src'},
    packages=["zxcvbncpp"],
    setup_requires=["cffi>=1.0.0"],
    cffi_modules=[os.path.join("python-src", "build_zxcvbn.py:ffi")],
    install_requires=[
        "cffi>=1.0.0",
    ],
    classifiers=[
        'Intended Audience :: Developers',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.6',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.2',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
    ],
)
