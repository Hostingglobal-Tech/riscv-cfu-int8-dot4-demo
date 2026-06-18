#!/bin/env python
# Copyright 2020 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from amaranth import *
from amaranth_cfu import InstructionBase, InstructionTestBase, simple_cfu, CfuTestBase
import unittest


class Int8Dot4MacInstruction(InstructionBase):
    """INT8 DOT4/MAC behind cfu_op0.

    funct7=0: signed int8x4 dot product, accumulate, return accumulator
    funct7=1: reset accumulator to zero
    funct7=2: read accumulator
    """

    def elab(self, m):
        accumulator = Signal(signed(32))

        products = [
            self.in0.word_select(i, 8).as_signed() *
            self.in1.word_select(i, 8).as_signed()
            for i in range(4)
        ]
        dot4 = Signal(signed(18))
        next_accumulator = Signal(signed(32))
        m.d.comb += [
            dot4.eq(products[0] + products[1] + products[2] + products[3]),
            next_accumulator.eq(accumulator + dot4),
        ]

        with m.If(self.start):
            with m.Switch(self.funct7):
                with m.Case(0):
                    m.d.sync += [
                        accumulator.eq(next_accumulator),
                        self.output.eq(next_accumulator),
                    ]
                with m.Case(1):
                    m.d.sync += [
                        accumulator.eq(0),
                        self.output.eq(0),
                    ]
                with m.Case(2):
                    m.d.sync += self.output.eq(accumulator)
                with m.Default():
                    m.d.sync += self.output.eq(0xBAD00000 | self.funct7)
            m.d.sync += self.done.eq(1)
        with m.Else():
            m.d.sync += self.done.eq(0)


def pack_i8x4(values):
    result = 0
    for i, value in enumerate(values):
        result |= (value & 0xff) << (8 * i)
    return result


def u32(value):
    return value & 0xffff_ffff


class Int8Dot4MacInstructionTest(InstructionTestBase):
    def create_dut(self):
        return Int8Dot4MacInstruction()

    def test_reset_dot4_read(self):
        self.verify([
            (1, 0, 0, 0),
            (0, pack_i8x4([1, -2, 3, -4]), pack_i8x4([5, 6, -7, -8]), 4),
            (2, 0, 0, 4),
            (0, pack_i8x4([-1, -1, -1, -1]), pack_i8x4([1, 2, 3, 4]), u32(-6)),
            (2, 0, 0, u32(-6)),
        ])


def make_cfu():
    return simple_cfu({
        0: Int8Dot4MacInstruction(),
    })


class CfuTest(CfuTestBase):
    def create_dut(self):
        return make_cfu()

    def test(self):
        DATA = [
            ((0, 1, 0, 0), 0),
            ((0, 0, pack_i8x4([1, -2, 3, -4]),
              pack_i8x4([5, 6, -7, -8])), 4),
            ((0, 2, 0, 0), 4),
            ((0, 0, pack_i8x4([-1, -1, -1, -1]),
              pack_i8x4([1, 2, 3, 4])), -6),
            ((0, 2, 0, 0), -6),
        ]
        return self.run_ops(DATA)


if __name__ == '__main__':
    unittest.main()
