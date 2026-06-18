/*
 * Copyright 2021 The CFU-Playground Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include "software_cfu.h"

namespace {

int32_t dot4_i8(uint32_t a, uint32_t b) {
  int32_t sum = 0;
  for (int i = 0; i < 4; ++i) {
    int8_t av = static_cast<int8_t>((a >> (8 * i)) & 0xff);
    int8_t bv = static_cast<int8_t>((b >> (8 * i)) & 0xff);
    sum += static_cast<int32_t>(av) * static_cast<int32_t>(bv);
  }
  return sum;
}

}  // namespace

// Software fallback for the same cfu_op0 funct7 mapping used by cfu.py.
uint32_t software_cfu(int funct3, int funct7, uint32_t rs1, uint32_t rs2) {
  static int32_t accumulator = 0;

  if (funct3 != 0) {
    return rs1;
  }

  switch (funct7) {
    case 0:
      accumulator += dot4_i8(rs1, rs2);
      return static_cast<uint32_t>(accumulator);
    case 1:
      accumulator = 0;
      return 0;
    case 2:
      return static_cast<uint32_t>(accumulator);
    default:
      return static_cast<uint32_t>(0xBAD00000 | funct7);
  }
}
