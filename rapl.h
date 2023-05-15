#define NUM_CPUS 96

#define NOPRINT 0
#define PRINT 1

// RAPL power
#define MSR_RAPL_POWER_UNIT            0x606
#define MSR_PKG_ENERGY_STATUS          0x611
#define MSR_DRAM_ENERGY_STATUS         0x619

// CPU energy buckets (customize as needed)
#define CPU_POWER_T1  1000
#define CPU_POWER_T2  3000
#define CPU_POWER_T3  6000
#define CPU_POWER_T4  9000
#define CPU_POWER_T5  12000
#define CPU_POWER_T6  15000
#define CPU_POWER_T7  20000
#define CPU_POWER_T8  25000
#define CPU_POWER_T9  30000
#define CPU_POWER_T10 35000

// DRAM energy buckets (customize as needed)
#define DRAM_POWER_T1 100
#define DRAM_POWER_T2 500
#define DRAM_POWER_T3 1000
#define DRAM_POWER_T4 1500
#define DRAM_POWER_T5 2000
#define DRAM_POWER_T6 2500
#define DRAM_POWER_T7 3000
#define DRAM_POWER_T8 3500
#define DRAM_POWER_T9 4000
#define DRAM_POWER_T10 5000

struct power_t {
  uint64_t cpu_energy;
  uint64_t dram_energy;
};

uint32_t getUnits();
uint32_t getPower(uint32_t cpus, struct power_t power_array[]);
uint32_t computePowerDiff(uint32_t cpus, struct power_t power_array_before[], struct power_t power_array_after[], struct power_t powerDiff[], struct power_t *powerSummary, int print);
uint32_t assignCPUPowerBucket(uint64_t power);
uint32_t assignDRAMowerBucket(uint64_t power);
