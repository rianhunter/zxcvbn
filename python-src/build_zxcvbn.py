import io
import os
import shutil
import subprocess
import sys

from cffi import FFI
ffi = FFI()

root_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

src_dir = os.path.join(root_dir, "native-src")

# first make sure {_frequency_lists,adjacency_graphs}.{hpp,cpp} are built
module_src_dir = os.path.join(src_dir, "zxcvbn")

for fnout in ["_frequency_lists.hpp", "_frequency_lists.cpp"]:
    subprocess.check_call([sys.executable, os.path.join(root_dir, "data-scripts", "build_frequency_lists.py"),
                           os.path.join(root_dir, "data"), os.path.join(module_src_dir, fnout)])

for fnout in ["adjacency_graphs.hpp", "adjacency_graphs.cpp"]:
    subprocess.check_call([sys.executable, os.path.join(root_dir, "data-scripts", "build_keyboard_adjacency_graphs.py"),
                           os.path.join(module_src_dir, fnout)])

# now produce amalgamation *shudders*
amalg = io.BytesIO()

for source_filename in ["zxcvbn.cpp",
                        "matching.cpp",
                        "scoring.cpp",
                        "frequency_lists.cpp",
                        "util.cpp",
                        "adjacency_graphs.cpp",
                        "_frequency_lists.cpp",
                        ]:
    with open(os.path.join(module_src_dir, source_filename), "rb") as f:
        shutil.copyfileobj(f, amalg)
        amalg.write(b"\n")

if sys.version_info[0] >= 3:
    PyMODINIT_FUNC = 'extern "C" __attribute__ ((visibility ("default"))) PyObject *'
else:
    PyMODINIT_FUNC = 'extern "C" __attribute__ ((visibility ("default"))) void'

EXTRA_COMPILE_ARGS = ["-std=c++14", "-fvisibility=hidden", "-Os", "-flto"]
EXTRA_LINK_ARGS = ["-fvisibility=hidden", "-Os", "-flto"]

ffi.set_source("zxcvbncpp._zxcvbncpp", amalg.getvalue().decode('utf-8'),
               include_dirs=[src_dir],
               extra_compile_args=EXTRA_COMPILE_ARGS,
               extra_link_args=EXTRA_LINK_ARGS,
               define_macros=[("PyMODINIT_FUNC", PyMODINIT_FUNC)],
               source_extension=".cpp")

ffi.cdef("""
typedef double zxcvbn_guesses_t;

struct zxcvbn_match_sequence;
typedef struct zxcvbn_match_sequence *zxcvbn_match_sequence_t;

int zxcvbn_password_strength(const char *pass, const char *const *user_inputs,
                             zxcvbn_guesses_t *guesses,
                             zxcvbn_match_sequence_t *mseq
                             );
void zxcvbn_match_sequence_destroy(zxcvbn_match_sequence_t);
""")
