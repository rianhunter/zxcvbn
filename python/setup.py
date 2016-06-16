from setuptools import setup

setup(
    name="zxcvbn",
    packages=["zxcvbn"],
    setup_requires=["cffi>=1.0.0"],
    cffi_modules=["build_zxcvbn.py:ffi"],
    install_requires=[
        "cffi>=1.0.0",
    ],
)
