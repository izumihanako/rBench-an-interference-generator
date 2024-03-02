# rBench

**rBench** is a toolkit containing several stress tests. It was originally designed to serve the thesis Eirene[^Eirene], which could stress a computer resource of a particular dimension as a way of interfering with other applications that might be occupying that resource.
**rBench** was designed to as focus as possible on the target resource, which means that interference with one dimension will put as little stress as possible on the other dimensions. It includes tests for CPU, cache, memory, disk, network and other resource dimensions, covering the computer resources required by common applications.



## Usage

Download the source code, go to the code directory and run the command to compile:

```bash
make
```

If you need to compile the AVX512 module, you need to change the value of AVX512SUPPORT in the Makefile.( Note that the AVX512 module has not been rigorously tested and verified ).



Then run the following command to run rbench:

```bash
./rbench.exe
```

Running rbench without any arguments outputs a help message. Full help information is at the end of README.

You can use `taskset` or `cgexec` to limit the cpu cores that rbench runs on and the resources it takes up.

Note that rbench itself has parameters to adjust CPU usage, e.g. the following command will run two cpu-int stressor, both of which are limited to 50% CPU usage:

```bash
./rbench.exe -r cpu-int -n 2 -s 50
```



## Possible output messages

- rbench checks the legitimacy of some key parameters. If you enter a parameter that is not legal (e.g., a negative memory size), it will output a prompt message
- If rbench fails to reach the run strength specified by the parameter for a period of time, it will output warning messages.
- If rbench gets an error while running (e.g. mmap failed or unable to create a socket) that still fails on retry, then it outputs the error message and exits the corresponding thread
- Other output information...



## Full help information

```bash
Usage :
         --addr ip:port        (stressor, network) For network related stressors, set the
                               ip:port for local program

         --addr-to ip:port     
         --to-addr ip:port     (stressor, network) For network related stressors, specify
                               the target machine's ip:port

         --cache-size N        (stressor) Specify the size of the cache buffer of the
                               cache stressor as N (bytes)

         --cache-size N        (stressor) Specify the size of the cache buffer of the
                               cache stressor as N (bytes)

         --check               (global) Run preset system check tasks. If this option is
                               present, all other options will be ignored

         --debug               (global) Output debug information
-d N,    --disk-file-size=N    (stressor, disk) Set the disk file size to N bytes
-l,      --limited=N           (stressor) If limited, the stressor will stop after N
                               rounds. Must have "=" !!!

-b W,    --mem-bandwidth W     
         --mb W                (stressor) For mem-bw, set mem-bandwidth to W MB/s for
                               every instance

-n N,    --ninstance N         
         --instance N          (stressor) Start N instances of stressors

         --no-warn             (global) Do not print warning information
         --page-tot N          (stressor) N (MB), MAKE SURE that this parameter is greater
                               than the total memory size that tlb can cache

         --pack-per-sec N      
         --pps N               (stressor, network) For network related stressors, set the
                               network pack sending rate to N pps

         --pack-size N         
         --psize N             (stressor, network) For network related stressors, set the
                               network pack size to N bytes

         --parallel            (global, not rigorously tested) Run in parallel mode
         --period N            (stressor) If specified, the time granularity is N
                               microseconds

-r NAME, --run NAME            (stressor) Run the specified stressor. Supported test items
                               are cacheL1, cacheL2, cacheL3, cache, cpu-int, cpu-float,
                               cpu-l1i, disk-write, tlb, mem-bw, simd-avx, simd-avx512

-s P,    --strength N          (stressor) Set stressors' load strength to P% for every
                               instance (run P% time per time granularity).Not valid for
                               mem-bw, network related stressors, ...

-t N,    --time N              (stressor) If specified, the stressor will stop after N
                               seconds

Note:
  PLEASE specify the benchmark project BEFORE specifying the runtime parameters!!

```



[^Eirene]: Eirene: Interference-aware and Isolation-on-demand Resource Scheduling

