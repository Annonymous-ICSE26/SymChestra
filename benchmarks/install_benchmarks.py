import os
import argparse
from multiprocessing import Process

configs = {
	'path': os.path.abspath(os.getcwd())
}

benchmark_url = {
    'xorriso-1.5.2' : 'https://ftp.gnu.org/gnu/xorriso/xorriso-1.5.2.tar.gz',
    'gawk-5.1.0' : 'https://ftp.gnu.org/gnu/gawk/gawk-5.1.0.tar.gz',
    'gcal-4.1' : 'https://ftp.gnu.org/gnu/gcal/gcal-4.1.tar.xz',
    'grep-3.6' : 'https://ftp.gnu.org/gnu/grep/grep-3.6.tar.gz', 
    'diffutils-3.7' : 'https://ftp.gnu.org/gnu/diffutils/diffutils-3.7.tar.xz',
    'sed-4.8' : 'https://ftp.gnu.org/gnu/sed/sed-4.8.tar.gz',
    'coreutils-8.32' : 'https://ftp.gnu.org/gnu/coreutils/coreutils-8.32.tar.gz',
    'patch-2.7.6' : 'https://ftp.gnu.org/gnu/patch/patch-2.7.6.tar.gz',
    'enscript-1.6.6' : 'https://ftp.gnu.org/gnu/enscript/enscript-1.6.6.tar.gz',
    'm4-1.4.18' : 'https://ftp.gnu.org/gnu/m4/m4-1.4.18.tar.gz',
    'trueprint-5.4' : 'https://ftp.gnu.org/gnu/trueprint/trueprint-5.4.tar.gz',
    'combine-0.4.0' : 'https://ftp.gnu.org/gnu/combine/combine-0.4.0.tar.gz'
}

benchmark_dir = {
    'xorriso-1.5.2' : 'xorriso',
    'gawk-5.1.0' : '', 
    'gcal-4.1' : 'src',
    'grep-3.6' : 'src',
    'diffutils-3.7' : 'src',
    'sed-4.8' : 'sed',
    'coreutils-8.32' : 'src',
    'patch-2.7.6' : 'src',
    'enscript-1.6.6' : 'src',
    'm4-1.4.18' : 'src',
    'trueprint-5.4' : 'src',
    'combine-0.4.0' : 'src',
}

def download(benchmark):
    url = benchmark_url[benchmark]
    os.system(f"wget {url}")
    filename = url.split('/')[-1]
    
    if filename.endswith(".tar.xz"):
        tar_cmd = f"tar -xf {filename}"
    elif filename.endswith(".tar.gz"):
        tar_cmd = f"tar -zxvf {filename}"
    else:
        tar_cmd = None

    if tar_cmd:
        print(f"[DOWNLOAD] {tar_cmd}")
        os.system(tar_cmd)

def build_llvm(benchmark, dirs):
    os.chdir(os.path.join(configs['path'], benchmark))
    os.mkdir("obj-llvm")
    os.chdir(os.path.join(configs['path'], benchmark, "obj-llvm"))

    cmd = "CC=wllvm CFLAGS=\"-g -O1 -Xclang -disable-llvm-passes -D__NO_STRING_INLINES -D_FORTIFY_SOURCE=0 -U__OPTIMIZE__\" ../configure --disable-nls --disable-largefile --disable-job-server --disable-load"
    os.system(cmd)
    os.system("make")
    os.system("extract-bc make")
    
    if dirs:
        os.chdir(dirs)
    
    cmd = "find . -executable -type f | xargs -I \'{}\' extract-bc \'{}\'" 
    os.system(cmd)
 
def build_gcov(benchmark, dirs):
    os.chdir(configs['path'] + "/" + benchmark)

    dir_name = "obj-gcov"
    
    os.mkdir(dir_name)
    os.chdir(configs['path'] + "/" + benchmark + "/" + dir_name)
    os.system("CFLAGS=\"-g -fprofile-arcs -ftest-coverage -g -O0\" ../configure --disable-nls --disable-largefile --disable-job-server --disable-load")
    os.system("make")
    os.chdir(configs['path'] + "/" + benchmark)

def install_single(benchmark):
    print(f"[START] Installing {benchmark}")

    download(benchmark)
    build_llvm(benchmark, benchmark_dir[benchmark])
    build_gcov(benchmark, benchmark_dir[benchmark])

    print(f"[DONE] {benchmark} installation complete.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--benchmark", type=str, help="Name of benchmark to install (default: all)", default=None)
    args = parser.parse_args()

    if args.benchmark:
        if args.benchmark not in benchmark_url:
            print(f"[ERROR] Unknown benchmark: {args.benchmark}")
            exit(1)
        install_single(args.benchmark)
    else:
        print("[INFO] No benchmark specified. Installing all benchmarks.")
        for benchmark in benchmark_url:
            install_single(benchmark)