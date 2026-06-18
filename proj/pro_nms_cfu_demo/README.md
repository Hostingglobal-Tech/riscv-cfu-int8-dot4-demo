# PRO NMS CFU INT8 DOT4 Demo

This project is a small CFU Playground demo that proves the CPU-to-CFU custom-instruction path with a deterministic signed `int8` dot product.

## Operation

The firmware runs two implementations over the same test vectors:

- `dot_cpu()`: normal RISC-V C loop.
- `dot_cfu()`: packs four signed `int8` values per operand and calls `cfu_op0()`.

CFU instruction mapping:

```text
cfu_op0(funct7=1, in0=0, in1=0)       reset accumulator
cfu_op0(funct7=0, in0=packed_a, in1=packed_b)
                                             signed int8x4 MAC accumulate
cfu_op0(funct7=2, in0=0, in1=0)       read accumulator
```

## Verified Output

Captured from Renode/Robot on WSL Ubuntu:

```text
PRO CFU DEMO - RISC-V + CFU INT8 DOT4

CPU-only dot product
result = -595
cycles = 2824

CFU custom instruction dot product
result = -595
cycles = 1554

VERIFY = OK
SPEEDUP = 1.81 x

CFU op mapping:
funct7=1 reset accumulator
funct7=0 dot4 accumulate: int8x4 MAC
funct7=2 read accumulator

Running on:
CFU Playground / LiteX / VexRiscV / Renode
```

## Reproduce

From the repository root:

```sh
python3 -m venv .venv-renode
.venv-renode/bin/pip install -r requirements-renode.txt
cd proj/pro_nms_cfu_demo
PATH=../../.venv-renode/bin:/usr/bin:/bin:/usr/sbin:/sbin make renode-test
```

CFU Python unit test:

```sh
cd proj/pro_nms_cfu_demo
PATH=/usr/bin:/bin:/usr/sbin:/sbin ../../scripts/pyrun cfu.py
```

## Files

- `cfu.py`: Amaranth CFU implementation for signed `int8x4` MAC.
- `src/pro_cfu_demo.c`: CPU-only and CFU-backed C demo.
- `src/software_cfu.cc`: software fallback model for the same `funct7` mapping.
- `src/proj_menu.cc`: project menu entry at `Project Menu -> 0`.
- `pro_nms_cfu_demo.robot`: Renode Robot test that verifies the UART output.
- `docs/renode-output.txt`: captured output from the verified Renode run.

## Note

Renode proves firmware behavior and the custom-instruction call path in simulation. It does not prove physical acceleration on the host PC. For real hardware speed, build and measure on an FPGA target.
