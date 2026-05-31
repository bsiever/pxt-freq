# Frequency Detection

```package
morse=github:bsiever/pxt-freq
```

Detects musical notes in real time using the micro:bit v2's built-in microphone. Identifies notes C3 through B5 (3 octaves, 36 notes) and reports when they start, stop, or are sustained.

> **micro:bit v2 only.** This extension requires the microphoneand audio processing capabilities of the micro:bit v2 hardware. It will not compile or run on micro:bit v1.

## Blocks

### ``started note`` — run code when a note begins

```sig
frequencies.startNote(function (note, cents) {})
```

Runs the handler once each time a new note is detected as playing. The handler receives **note** (the detected `Note`, e.g. `Note.A4`) and **cents** (raw pitch deviation, where positive = sharp, negative = flat).

```blocks
frequencies.startNote(function (note, cents) {
    basic.showString(frequencies.noteToString(note))
})
```

---

### ``stopped note`` — run code when a note ends

```sig
frequencies.stopNote(function (note, cents) {})
```

Runs the handler once each time a note stops being detected. Pairs with `startNote` to track note on/off events. The handler receives **note** (the `Note` that stopped) and **cents** (last known pitch deviation).

```blocks
frequencies.stopNote(function (note, cents) {
    basic.clearScreen()
})
```

---

### ``playing note`` — run code while a note is held

```sig
frequencies.playingNote(function (note, cents) {})
```

Runs the handler every detection cycle (~every 0.37 s) while a note remains active. Use this to continuously track pitch or update a display while a note is sustained. The handler receives
**note** (the `Note` currently playing) and **cents** (current pitch deviation in cents).

```blocks
frequencies.playingNote(function (note, cents) {
    if (cents > 10) {
        basic.showString("+")
    } else if (cents < -10) {
        basic.showString("-")
    } else {
        basic.showString("=")
    }
})
```

---

### ``watch note`` — monitor one specific note continuously

```sig
frequencies.watchNote(Note.A4, function (theNote, power, cents) {})
```

Runs the handler every detection cycle for one specific note, reporting its raw power and pitch deviation regardless of whether the note crosses the "playing" threshold. Useful for tuning meters or monitoring the strength of a particular pitch at all times.

The handler receives **theNote** (the `Note` being monitored), **power** (relative power, 0 = silence to 1000 = this note is the loudest thing currently detected), and **cents** (current pitch deviation in cents).

```blocks
frequencies.watchNote(Note.A4, function (theNote, power, cents) {
    if (power > 500) {
        let deviation = cents
        if (deviation < -20) {
            basic.showArrow(ArrowNames.West)
        } else if (deviation > 20) {
            basic.showArrow(ArrowNames.East)
        } else {
            basic.showIcon(IconNames.Yes)
        }
    }
})
```

---

### ``note for frequency below or equal to`` — find the note at or below a frequency

```sig
frequencies.getNoteForBelowOrEqual(440)
```

Returns the `Note` whose FFT bin contains the given frequency, or the nearest note whose bin falls below it. Useful for mapping a raw Hz value (e.g. from `getBinWidth() * binIndex`) back to a note.

```blocks
frequencies.startNote(function (note, cents) {
    let hz = frequencies.getBinWidth() * frequencies.getMaxBinIndex()
    let closest = frequencies.getNoteForBelowOrEqual(hz)
    basic.showString(frequencies.noteToString(closest))
})
```

---

### ``note for frequency above or equal to`` — find the note at or above a frequency

```sig
frequencies.getNoteForAboveOrEqual(440)
```

Returns the `Note` whose FFT bin contains the given frequency, or the nearest note whose bin falls above it. Pairs with `getNoteForBelowOrEqual` to bracket a frequency range by note.

---

## Advanced blocks

These expose the raw FFT results for custom detection logic or debugging.

### ``getNotePower(noteIndex)``

```sig
frequencies.getNotePower(0)
```

Returns the normalized power for note index `noteIndex` (0 = C3, 35 = B5), scaled 0–1000, after harmonic suppression. Near 0 means silence; 1000 means full-scale input.

### ``getNoteCents(noteIndex)``

```sig
frequencies.getNoteCents(0)
```

Returns the raw pitch deviation for the note at `noteIndex`. Divide by 40 to convert to cents. Positive = sharp, negative = flat.

### ``getAvgNotePower()``

```sig
frequencies.getAvgNotePower()
```

Returns the mean normalized power across all 36 notes (0–1000 scale). Useful as a rough indicator of overall sound level.

### ``getMaxNotePower()``

```sig
frequencies.getMaxNotePower()
```

Returns the highest normalized power among all 36 notes. The internal detection logic uses this to set the threshold for note-on decisions.

### ``getNumNotes()``

```sig
frequencies.getNumNotes()
```

Returns 36 — the number of notes tracked (C3 through B5).

### ``getBin(binIndex)``

```sig
frequencies.getBin(0)
```

Returns the raw squared magnitude of FFT bin `binIndex` (0–2047). Each bin spans ~2.71 Hz. Bin 0 is DC; bin 2047 is ~5556 Hz. Use this to inspect the full spectrum for debugging.

### ``getMaxBinPower()``

```sig
frequencies.getMaxBinPower()
```

Returns the raw squared magnitude of the highest-power FFT bin across the entire spectrum (not just the 36 tracked notes). Useful for detecting any strong frequency component.

### ``getMaxBinIndex()``

```sig
frequencies.getMaxBinIndex()
```

Returns the bin index (0–2047) of the FFT bin with the highest power. Multiply by `getBinWidth()` to get the frequency in Hz.

### ``getBinWidth()``

```sig
frequencies.getBinWidth()
```

Returns the width of each FFT bin in Hz (~2.71 Hz). Use this to convert between bin indices and frequencies.

### ``noteToString(note)``

```sig
frequencies.noteToString(Note.A4)
```

Converts a `Note` enum value to a human-readable string such as `"A4"`, `"C#3"`, or `"Bb5"`.

---

## Detection range and accuracy

| Property              | Value                         |
| --------------------- | ----------------------------- |
| Notes detected        | C3 – B5 (36 notes, 3 octaves) |
| Sample rate           | ~11,111 Hz                    |
| FFT size              | 4096 points                   |
| Frequency resolution  | ~2.71 Hz per bin              |
| Detection latency     | ~0.37 s per frame             |
| Pitch resolution      | ±50 cents      |

### Tips for best results

- **Hold the micro:bit close to the sound source.** The MEMS microphone is small; distance reduces signal level significantly.
- **Single sustained notes work best.** Detection is designed around one note at a time. Chord detection is approximate.
- **Lower octave notes (C3–B3) are harder to detect** because microphone sensitivity drops at low frequencies and piano overtones are often stronger than the fundamental.
- **Minimize background noise.** The detection threshold is relative to the loudest frequency present, so broadband noise raises the floor for all notes.

### How detection works

Each frame, the extension:

1. Captures 4096 samples at ~11,111 Hz from the microphone.
2. Runs a 4096-point Q15 real FFT (CMSIS-DSP).
3. For each of the 36 notes, finds the power in the nearest FFT bin and applies parabolic interpolation to estimate the sub-bin pitch deviation.
4. Applies **harmonic suppression**: for each note, subtracts its estimated contribution to higher notes that fall near its overtones (harmonics 2–6). This helps the fundamental frequency win over its own overtones.
5. Reports notes whose power exceeds a threshold derived from the frame's peak power. A note must exceed the threshold in two consecutive frames before a "started" event fires, and must fall below it in two consecutive frames before a "stopped" event fires.

# Examples

## Acting on a Note

The micro:bit can "react" to notes being played and could even detect or record a tune!  

```blocks
frequencies.watchNote(Note.C4, function (theNote, power, cents) {
    if (power > 0) {
        basic.showIcon(IconNames.Heart)
    } else {
        basic.clearScreen()
    }
})
```

## "Tuning"

The example below can be used to tune instruments.  Each dot on the micro:bit's LED grid corresponds to a note (C3-B5), with C3 in the top left, C4 (middle C) in the center, and B5 in the bottom right.  One or two LEDs will light to indicate dominate frequencies.  If two dots are list, the frequency is between two notes and should be adjusted up or down until a single dot is lit.  A single dot indicates the dominate frequency is within 2.71 Hz of the respective note.

```blocks
frequencies.watchMaxPower(function (lowFrequency: number, highFrequency: number, power: number) {
    if (power > 10000) {
        let noteBelow = frequencies.getNoteForBelowOrEqual(lowFrequency);
        let noteAbove = frequencies.getNoteForAboveOrEqual(highFrequency);
        let noteBelowString = frequencies.noteToString(noteBelow);
        let noteAboveString = frequencies.noteToString(noteAbove);
        serial.writeLine(noteBelowString + " - " + noteAboveString + "   Max power " + power + "  (" + lowFrequency + "-" + highFrequency + " Hz)" )
        // Alternative way to get note strings without calling noteToString multiple times:
        basic.clearScreen();
        let noteNumber = frequencies.getNoteIndex(noteBelow);
        led.plot(noteNumber % 5, Math.floor(noteNumber / 5))
        noteNumber = frequencies.getNoteIndex(noteAbove);
        led.plot(noteNumber % 5, Math.floor(noteNumber / 5))
        
    }
})
```

---

## Acknowledgements

Icon based on [Font Awesome icon 0xF001](https://www.iconfinder.com/search?q=f001) SVG.

CMSIS-DSP library (Q15 FFT implementation) copyright ARM Limited, licensed under
[Apache 2.0](cmsis-dsp/LICENSE).

Significant components of this were constructed using Claude Code.

---

## Support

I develop micro:bit extensions in my spare time to support activities I'm enthusiastic about, like summer camps and science curricula. You are welcome to become a sponsor of my micro:bit work (one time or recurring payments), which helps offset equipment costs: [github.com/sponsors/bsiever](https://github.com/sponsors/bsiever). Any support at all is greatly appreciated!

### Supported targets

for PXT/microbit (v2 only)

<script src="https://makecode.com/gh-pages-embed.js"></script>
<script>makeCodeRender("{{ site.makecode.home_url }}", "{{ site.github.owner_name }}/{{ site.github.repository_name }}");</script>
