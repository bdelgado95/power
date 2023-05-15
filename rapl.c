#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "rapl.h"

int main(int argc, char **argv)
{
  struct power_t power_array_before[NUM_CPUS] = {0};
  struct power_t power_array_after[NUM_CPUS]  = {0};
  struct power_t power_diff[NUM_CPUS] = {0};
  struct power_t powerSummary = {0};
  uint32_t cpuPowerBucket = 0;
  uint32_t dramPowerBucket = 0;
  uint32_t i = 0;
  uint32_t detected_cpus = sysconf(_SC_NPROCESSORS_ONLN);
  uint32_t duration=3;
  
  if (detected_cpus != NUM_CPUS) {
    printf("\nCurrently set to use %u CPUs, configure NUM_CPUS in rapl.h for your system.\n", NUM_CPUS);
  } else {
    printf("\nUsing %u CPUs\n", NUM_CPUS);
  }
  
  /*
    Optional: Dump out info on units from MSR_RAPL_POWER_UNIT
   */
  getUnits();
  
  /*
    Take two RAPL samples 
  */
  getPower(NUM_CPUS, power_array_before);
  sleep(duration);
  getPower(NUM_CPUS, power_array_after);

  computePowerDiff((uint32_t)NUM_CPUS, power_array_before, power_array_after, power_diff, &powerSummary, NOPRINT);
  cpuPowerBucket = assignCPUPowerBucket(powerSummary.cpu_energy);
  dramPowerBucket = assignDRAMowerBucket(powerSummary.dram_energy);

  for (i=0; i < NUM_CPUS; i++) {
    printf("\nCPU %u: (POWER, DRAM) energy: %lu %lu", i, power_diff[i].cpu_energy, power_diff[i].dram_energy);
  }
  printf("\n\n\nCPU PWR %lu. DRAM PWR %lu\n\n", powerSummary.cpu_energy, powerSummary.dram_energy);
  return 0;
}

uint32_t getUnits()
{
  int fd=0;
  uint64_t data = 0;
  uint32_t power_units=0;
  uint32_t energy_status_units=0;
  uint32_t time_units = 0;
  const unsigned char full_msr_string[32] = "/dev/cpu/0/msr";

  fd = open(full_msr_string, O_RDONLY);
  if (fd < 0) {
    printf("\nCan't open MSR module\n");
    return 0;
  }
  
  if (pread(fd, &data, sizeof(data), MSR_RAPL_POWER_UNIT) != sizeof(data)) {
    printf("\nError reading msr\n");
    return 0;
  }
  
  printf("\nMSR_RAPL_POWER_UNIT %lu", data);
  power_units = data & 0xf;
  energy_status_units = data & 0x1f00;
  time_units = data & 0xf0000;
  
  printf("\n\tPower Units %d (dec) Watts ", power_units);
  printf("\n\tEnergy Status Units (dec) Joules %d ", energy_status_units);
  printf("\n\tTime Units (dec) microseconds  %d", time_units);

  return 0;
}

uint32_t getPower(uint32_t cpus, struct power_t power_array[])
{
  const unsigned char msr_string[32] = "/dev/cpu/"; // 0/msr";
  unsigned char full_msr_string[32];
  unsigned char cpu_string[10];
  int fd=0;
  uint32_t i = 0;
  unsigned long long data = 0;
  unsigned int reg = 0;
  unsigned int power_units=0;
  unsigned int energy_status_units=0;
  unsigned int time_units = 0;

  for (i=0; i < cpus; i++) {
    strcpy(full_msr_string, msr_string); // prepare "/dev/cpu/[0,1,2,3]/msr", etc
    sprintf(cpu_string, "%d", i); //
    strcat(full_msr_string, cpu_string);
    strcat(full_msr_string, "/msr");

    fd = open(full_msr_string, O_RDONLY);
    if (fd < 0) {
      printf("\nCan't open MSR module\n");
      return 0;
    }
    
    /*
      Query Package Energy Status
    */
    if (pread(fd, &data, sizeof data, MSR_PKG_ENERGY_STATUS) != sizeof data) {
      printf("\nError reading msr\n");
      return 0;
    }
    
    //printf("\nMSR_PKG_ENERGY_STATUS");
    power_array[i].cpu_energy = data;

    /*
      Query DRAM energy usage
    */
    
    if (pread(fd, &data, sizeof data, MSR_DRAM_ENERGY_STATUS) != sizeof data) {
      printf("\nError reading msr\n");
      return 0;
    }

    power_array[i].dram_energy = data;
    close(fd);
  }

  return 0;
}


uint32_t computePowerDiff(uint32_t cpus, struct power_t power_array_before[], struct power_t power_array_after[], struct power_t powerDiff[], struct power_t *powerSummary, int print)
{
  uint32_t i=0;
  uint64_t totalDiff = 0;
  uint64_t cpuPowerSummary = 0;
  uint64_t dramPowerSummary = 0;

  for (i=0; i < cpus; i++) {
    if (power_array_after[i].cpu_energy >= power_array_before[i].cpu_energy) {
      powerDiff[i].cpu_energy = power_array_after[i].cpu_energy  - power_array_before[i].cpu_energy;
    } else {
      //printf("\nWraparound detected, before = %u. after = %u\n", power_array_before[i].cpu_energy,  power_array_after[i].cpu_energy);
      powerDiff[i].cpu_energy = (0xffffffff + power_array_after[i].cpu_energy - power_array_before[i].cpu_energy);
      //printf("\nSpecial calculated diff as %u", powerDiff[i].cpu_energy);
    }

    powerSummary->cpu_energy += powerDiff[i].cpu_energy;
    
    if (power_array_after[i].dram_energy >= power_array_before[i].dram_energy) {
      powerDiff[i].dram_energy  = power_array_after[i].dram_energy - power_array_before[i].dram_energy;
    } else { // Counter wraparound
      //printf("\nWraparound detected, before = %u. after = %u\n", power_array_before[i].dram_energy,  power_array_after[i].dram_energy);
      powerDiff[i].dram_energy = (0xffffffff + power_array_after[i].dram_energy - power_array_before[i].dram_energy);
      //printf("\nSpecial calculated diff as %u", powerDiff[i].dram_energy);
    }

    powerSummary->dram_energy += powerDiff[i].dram_energy;

  }

  if (print==1) {
    printf("\nPowerSummary %lu %lu", powerSummary->cpu_energy, powerSummary->dram_energy);
    for (i=0; i < cpus; i++) {
      printf("\nFULL: %lu %lu ", powerDiff[i].cpu_energy, powerDiff[i].dram_energy);
    }
  }
  return 0;
}

// Fixme: This could be made more elegant
uint32_t assignCPUPowerBucket(uint64_t power)
{
  if (power >= CPU_POWER_T10) {
    return 10;
  } else if (power >= CPU_POWER_T9) {
    return 9;
  } else if (power >= CPU_POWER_T8) {
    return 8;
  } else if (power >= CPU_POWER_T7) {
    return 7;
  } else if (power >= CPU_POWER_T6) {
    return 6;
  } else if (power >= CPU_POWER_T5) {
    return 5;
  } else if (power >= CPU_POWER_T4) {
    return 4;
  } else if (power >= CPU_POWER_T3) {
    return 3;
  } else if (power >= CPU_POWER_T2) {
    return 2; 
  } else if (power >= CPU_POWER_T1) {
    return 1;
  } else {
    return 0;
  }
}

// Fixme: This could be made more elegant
uint32_t assignDRAMowerBucket(uint64_t power)
{
  if (power >= DRAM_POWER_T10) {
    return 10;
  } else if (power >= DRAM_POWER_T9) {
    return 9;
  } else if (power >= DRAM_POWER_T8) {
    return 8;
  } else if (power >= DRAM_POWER_T7) {
    return 7;
  } else if (power >= DRAM_POWER_T6) {
    return 6;
  } else if (power >= DRAM_POWER_T5) {
    return 5;
  } else if (power >= DRAM_POWER_T4) {
    return 4;
  } else if (power >= DRAM_POWER_T3) {
    return 3;
  } else if (power >= DRAM_POWER_T2) {
    return 2;
  } else if (power >= DRAM_POWER_T1) {
    return 1;
  } else {
    return 0;
  }
}
