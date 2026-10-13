# Verification and Characterization Plan

This document defines the measurements needed to turn the current prototype into a quantitatively characterized optical link.

## 1. Questions to answer

1. Is the transmitter bit period actually close to the configured 50 µs?
2. Does the receiver output preserve those transitions at the selected rate?
3. How often are complete packets reconstructed correctly?
4. How does reliability change with distance, alignment, and ambient light?
5. What is the highest rate the current optical hardware can support reliably?

## 2. Required tools

Minimum:

- current transmitter and receiver hardware,
- two USB serial connections or separate power sources,
- ruler/tape measure,
- repeatable optical mounts,
- test payload list.

Strongly recommended:

- oscilloscope or logic analyzer,
- photodiode/TIA probe point for a future analog receiver,
- optical power meter if available.

## 3. Timing verification

Probe transmitter D8 and digital receiver OUT. Record start-bit width, data-bit width, stop-bit width, inter-byte gap, transmitter-to-receiver edge delay, and rise/fall behavior at receiver OUT.

Expected firmware target:

```text
bit period ≈ 50 µs
raw configured rate ≈ 20 kbit/s
```

Do not report 20 kbit/s as measured until this test is performed.

## 4. Packet reliability test

Use changing payloads so a stale OLED image cannot be mistaken for successful reception. For each condition, send at least 100 packets and record correct, wrong, and missing packets.

| Metric | Definition |
| --- | --- |
| Packets sent | Number emitted by transmitter |
| Correct packets | Exact payload match |
| Wrong packets | Payload received but incorrect |
| Missing packets | No valid packet reconstructed |
| Packet success rate | correct / sent |
| Packet error rate | 1 - success rate |

Receiver Serial output should be used as the primary count; OLED inspection alone is insufficient.

## 5. Distance sweep

At each chosen distance, align the beam, keep the geometry fixed, transmit the same packet set, and record success/error counts plus the ambient-light condition. Do not extrapolate beyond measured distances.

## 6. Alignment sensitivity

At a fixed distance, start from optimized alignment, offset the receiver laterally in controlled increments, and repeat the packet test at each position. Plot packet success rate versus offset.

## 7. Ambient-light test

Repeat one fixed-distance test under low ambient light and normal bright-room lighting. Record idle-level changes, false start edges, and packet-success changes.

## 8. Rate sweep

The current code uses `BIT_TICKS = 100`. With Timer1 at 2 MHz this is 50 µs per bit.

| BIT_TICKS | Bit period | Nominal raw rate |
| ---: | ---: | ---: |
| 400 | 200 µs | 5 kbit/s |
| 200 | 100 µs | 10 kbit/s |
| 100 | 50 µs | 20 kbit/s |

Change `BIT_TICKS` identically on transmitter and receiver. The aim is to find the measured reliability boundary, not to maximize the number written in the README.

## 9. Beam interruption / failure behavior

During repeated transmission, fully block the beam, partially block it, restore it, and deliberately misalign/re-align the receiver. Observe missing packets, malformed packets, false valid packets, and recovery behavior.

## 10. Recommended result plots

- packet success rate vs distance,
- packet success rate vs bit rate,
- packet success rate vs lateral misalignment,
- timing capture showing one complete byte,
- transmitter and receiver edge comparison,
- optional analog eye diagram after upgrading to a photodiode/TIA front end.

## 11. Acceptance criteria for the next repository release

Before replacing the current “nominal” wording with a measured performance claim, record oscilloscope/logic-analyzer timing evidence, exact test distance, ambient-light condition, number of transmitted packets, packet statistics, and the hardware configuration used.
