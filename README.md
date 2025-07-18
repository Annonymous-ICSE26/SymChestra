# SymChestra
SymChestra is the first orchestration framework to integrate symbolic execution techniques that constructs the set of entry states for the next technique and finds the best-performing parameterized components, thereby maximizing the performance of symbolic execution.

# Installation
For ease of use, we would like to introduce Docker image for fast installation. You can just install SymChestra by following instructions.
```bash
$ docker build -t symchestra/icse2026 .
$ docker run -it symchestra/icse2026
```
In this dockerfile, we provide the best-performing scenario evaluated in our paper (Symtuner + Featmaker + KLEE-RAM) and 16 benchmark programs installation.

# How to execute SymChestra 
To run SymChestra, you can execute the following command in the '/root/symchestra/' directory in three integration modes.
```bash
$ # naive Mode - Union-based integration
$ python3 bin_sequence(SFR).py sqlite sequence_sqlite
$
$ # naive Mode - Seqeunce-based integration
$ python3 bin_union(SFR).py sqlite union_sqlite
$
$ # SymChestra Mode
$ python3 bin_symchestra(SFR).py sqlite symchestra_sqlite
```
Each argument of the command represents:
* sqlite : the program to be tested under our configuration ('sqlite' can be replaced by any programs installed in /root/benchmarks directory.)
* sequence_sqlite : the name of output directory (also can be replaced)

Furthermore, the following arguments can be specified as you like:
* --total_budget : the total time budget for your experiment (default setting in our evaluation is 24-hours, 86,400s)

If you want to evaluate the standalone techniques without any orchestration, you can execute the following commands.
```bash
$ python3 bin_standalone_{technique name}.py configs/sqlite.json standalone_sqlite
```
In our dockerfile, we declare three techniques used in the best scenarios: SymTuner, FeatMaker, KLEE-RAM

# Check the effectiveness of SymChestra in terms of code coverage and bug-finding ability
After all time budget is exahusted, the program displays the number of branches the technique achieved as follows:
```bash
$ Standalone {technique name} achieved X,XXX coverage.
$ Union-based integration of SymTuner+FeatMaker+RAM achieved X,XXX coverage.
$ Sequence-based integration of SymTuner+FeatMaker+RAM achieved X,XXX coverage.
$ SymChestra-based integration of SymTuner+FeatMaker+RAM achieved X,XXX coverage.
```
