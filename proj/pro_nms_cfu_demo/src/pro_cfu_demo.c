/*
 * Copyright 2026 The CFU-Playground Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 */

#include "pro_cfu_demo.h"

#include <stdint.h>
#include <stdio.h>

#include "cfu.h"
#include "perf.h"

#define DOT_LEN 256

static int8_t input_data[DOT_LEN] __attribute__((aligned(4)));
static int8_t weight_data[DOT_LEN] __attribute__((aligned(4)));

static void init_vectors(void) {
  for (int i = 0; i < DOT_LEN; ++i) {
    input_data[i] = (int8_t)(((i * 17 + 3) % 31) - 15);
    weight_data[i] = (int8_t)(((i * 11 + 5) % 29) - 14);
  }
}

static int32_t dot_cpu(const volatile int8_t* input,
                       const volatile int8_t* weight, int len) {
  int32_t accumulator = 0;
  for (int i = 0; i < len; ++i) {
    accumulator += (int32_t)input[i] * (int32_t)weight[i];
  }
  return accumulator;
}

static int32_t dot_cfu(const volatile int8_t* input,
                       const volatile int8_t* weight, int len) {
  cfu_op0(1, 0, 0);

  const volatile uint32_t* in32 = (const volatile uint32_t*)(const volatile void*)input;
  const volatile uint32_t* wt32 = (const volatile uint32_t*)(const volatile void*)weight;
  for (int i = 0; i < len / 4; ++i) {
    cfu_op0(0, in32[i], wt32[i]);
  }

  return (int32_t)cfu_op0(2, 0, 0);
}

static uint64_t measure_cpu(int32_t* result) {
  uint64_t start = perf_get_mcycle64();
  *result = dot_cpu(input_data, weight_data, DOT_LEN);
  return perf_get_mcycle64() - start;
}

static uint64_t measure_cfu(int32_t* result) {
  uint64_t start = perf_get_mcycle64();
  *result = dot_cfu(input_data, weight_data, DOT_LEN);
  return perf_get_mcycle64() - start;
}

void run_pro_cfu_demo(void) {
  int32_t cpu_result = 0;
  int32_t cfu_result = 0;
  uint64_t cpu_cycles = 0;
  uint64_t cfu_cycles = 0;

  init_vectors();

  (void)dot_cpu(input_data, weight_data, DOT_LEN);
  (void)dot_cfu(input_data, weight_data, DOT_LEN);

  cpu_cycles = measure_cpu(&cpu_result);
  cfu_cycles = measure_cfu(&cfu_result);

  puts("");
  puts("PRO CFU DEMO - RISC-V + CFU INT8 DOT4");
  puts("");
  puts("CPU-only dot product");
  printf("result = %ld\n", (long)cpu_result);
  printf("cycles = %llu\n", (unsigned long long)cpu_cycles);
  puts("");
  puts("CFU custom instruction dot product");
  printf("result = %ld\n", (long)cfu_result);
  printf("cycles = %llu\n", (unsigned long long)cfu_cycles);
  puts("");
  printf("VERIFY = %s\n", cpu_result == cfu_result ? "OK" : "FAIL");

  if (cfu_cycles != 0) {
    uint64_t speedup_x100 = (cpu_cycles * 100) / cfu_cycles;
    printf("SPEEDUP = %llu.%02llu x\n",
           (unsigned long long)(speedup_x100 / 100),
           (unsigned long long)(speedup_x100 % 100));
  } else {
    puts("SPEEDUP = unavailable");
  }

  puts("");
  puts("CFU op mapping:");
  puts("funct7=1 reset accumulator");
  puts("funct7=0 dot4 accumulate: int8x4 MAC");
  puts("funct7=2 read accumulator");
  puts("");
  puts("Running on:");
  puts("CFU Playground / LiteX / VexRiscV / Renode");
}
