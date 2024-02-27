This code monitors CPU and memory power utilization using Intel's Running Average Power Limiting ("RAPL") technology.

More details on Intel RAPL: https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-3b-part-2-manual.pdf

See section 14.9 "Platform Specific Power Management Support"

Prereq:

  ```sudo apt-get install msr-tools``` (or equivalent)
  
  ```sudo modprobe msr```

Compiling:
1. Set the number of CPUs in rapl.h
2. ```gcc rapl.c -o rapl.o```

---------------------------------------------------
Standalone usage: 

```sudo ./rapl.o```

Sample Output:

```
sudo ./rapl.o

Using 96 CPUs

MSR_RAPL_POWER_UNIT 658947
        Power Units 3 (dec) Watts
        Energy Status Units (dec) Joules 3584
        Time Units (dec) microseconds  655360
Sleeping...

CPU 0: (CPU, DRAM) energy: 3639590 4960840
CPU 1: (CPU, DRAM) energy: 3639590 4960840
CPU 2: (CPU, DRAM) energy: 3639590 4960840
CPU 3: (CPU, DRAM) energy: 3639590 4960840

...
CPU 95: (CPU, DRAM) energy: 3583898 5578793


CPU PWR 346758607. DRAM PWR 505949266

```
---------------------------------------------------
