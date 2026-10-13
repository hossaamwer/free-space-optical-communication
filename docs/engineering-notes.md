# Engineering Notes

## 1. Design objective

The project implements a small free-space optical data link with enough structure to study communications engineering rather than only demonstrate that a laser can blink.

The functional path is:

```text
keypad input
→ message buffer
→ STX/payload/ETX framing
→ asynchronous byte framing
→ OOK laser modulation
→ free-space propagation
→ optical receiver thresholding
→ start-edge detection
→ center-bit sampling
→ byte validation
→ message reconstruction
→ OLED display
```

## 2. Why OOK?

On-off keying is the simplest form of intensity modulation/direct detection. A logical one is represented by received light and a logical zero by the absence of that light. It is attractive for a first prototype because the physical-layer behavior is visible and the transmitter can be driven directly by a digital signal.

The limitation is equally important: OOK does not by itself provide robustness against ambient illumination, channel fading, threshold drift, or timing errors. Those effects become part of the measurement problem.

## 3. Why timer-based bit timing?

At a nominal 50 µs bit period, software overhead becomes a meaningful fraction of a bit. Repeated high-level GPIO calls and relative delays can accumulate timing error.

The transmitter therefore:

- runs Timer1 at 2 MHz,
- uses 100 timer ticks per bit,
- drives D8 through the AVR port register,
- advances an absolute deadline for each next bit.

The receiver uses the same timer scale and samples near bit centers after detecting a start edge.

This is still a software receiver. Start-edge polling latency, clock tolerance, GPIO/receiver delay, and module bandwidth all contribute to timing uncertainty.

## 4. Optical/electrical polarity

The protocol defines:

- optical `1` = laser ON,
- optical `0` = laser OFF.

The digital receiver module used in the prototype has an electrically inverted output: its output is LOW when illuminated. The receiver firmware converts that electrical level back into the logical optical bit.

Keeping optical polarity and electrical polarity conceptually separate makes debugging much easier.

## 5. Framing and integrity

The packet uses:

```text
STX (0x02) + printable ASCII payload + ETX (0x03)
```

The receiver also checks:

- start bit,
- stop bit,
- payload length,
- printable-character range,
- partial-message timeout.

These checks detect some invalid frames but they are **not an integrity code**. A corrupted printable character can still be accepted. A CRC is therefore a high-priority next step.

## 6. Raw line rate vs useful throughput

A 50 µs bit period corresponds to a nominal raw line rate of 20 kbit/s. This number describes the configured bit timing only.

Each byte also contains a start bit, stop bit, and inter-byte idle time, while each message adds STX/ETX framing and a deliberate post-packet pause for OLED refresh. Payload throughput is therefore lower than the raw line rate.

For a research-style report, these quantities should be kept separate:

- configured raw bit rate,
- measured physical bit timing,
- payload goodput,
- packet-error rate,
- bit-error rate if a suitable automated test sequence is added.

## 7. Why the digital receiver is the main hardware limitation

The digital laser receiver module contains its own photosensor, amplification, and thresholding behavior. That is convenient for a first link but hides the analog information needed for serious characterization.

A photodiode plus transimpedance amplifier would allow measurement of:

- received signal amplitude,
- noise floor,
- bandwidth,
- rise/fall time,
- threshold margin,
- eye opening,
- response versus distance and alignment.

That upgrade would make the optical front end much more relevant to detector and receiver engineering.

## 8. What would make this closer to a research instrument?

A stronger second iteration would add:

1. CRC-8 and packet sequence numbers.
2. Automated transmission of thousands of known packets.
3. Logged PER/BER versus bit rate and distance.
4. Analog photodiode/TIA receiver measurements.
5. Oscilloscope traces and an eye diagram.
6. Optical power measurement and link-budget comparison.
7. Controlled ambient-light and misalignment experiments.
8. Hardware-timed sampling or oversampling receiver logic.

The goal is not to make the prototype look more complicated; it is to replace qualitative claims with measurable communication performance.
