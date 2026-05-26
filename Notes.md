


`PXT_COMPILE_SWITCHES=csv---mbcodal pxt deploy`



Standard Local Build:PXT_COMPILE_SWITCHES=csv---mbcodal PXT_FORCE_LOCAL=1 npx pxt build --localBuild & Run Local Editor:If you want to serve a local editor that also uses local CODAL compilation:PXT_COMPILE_SWITCHES=csv---mbcodal PXT_FORCE_LOCAL=1 PXT_RUNTIME_DEV=1 npx pxt serve --local



Via Claude:
The ADC path explains the exact discrepancy. CODAL converts to a period in whole microseconds, then scales for a 16 MHz timer (64 MHz ÷ 4 prescaler):


newPeriod  = 1,000,000 / 11,000  →  90 µs  (integer division, truncates 90.909…)
timerCount = 90 × 16            →  1,440 ticks at 16 MHz
actual rate = 16,000,000 / 1,440 →  11,111.1̄ Hz
So the closest it can get to 11,000 Hz is 11,111 Hz, and that predicts:


bin width = 11,111.1̄ / 256 ≈ 43.40 Hz


The Goertzel filter at each note is mathematically equivalent to a DFT bin of the same block size. Its frequency response is a sinc-shaped window:

Bin width = FS / N = 11136 / 1559 ≈ 7.14 Hz

More specifically:

−3 dB half-bandwidth: ≈ ±3.2 Hz (power drops to half at ~3.2 Hz off-center)
First null: ±7.14 Hz from the center frequency
The concern is whether adjacent notes are far enough apart to be distinguishable. Semitone spacing grows with frequency:

Octave	Example gap	vs. bin width
C3–C#3	7.78 Hz	≈ 1.09× — barely one bin apart
C4–C#4	15.56 Hz	≈ 2.2× — comfortable
C5–C#5	31.1 Hz	≈ 4.4× — clean
C6–C#6	62.2 Hz	≈ 8.7× — very clean
The C3 octave is the tight case. Adjacent notes are only ~1 bin apart, so the −3 dB points of neighboring notes nearly overlap. A tone played exactly on C3 will still register measurable power at C#3 (and vice versa). Detection in the C3–B3 range will work best when the played note is well in tune — a few Hz flat or sharp could tip the winner to the adjacent note.

C4 and above have comfortable separation and should be reliable.