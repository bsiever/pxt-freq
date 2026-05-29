```package
morse=github:bsiever/pxt-freq
```

# Frequency Detection

Detects musical notes in real time using the micro:bit v2's built-in microphone. Identifies notes
C3 through B5 (3 octaves, 36 notes) and reports when they start, stop, or are sustained.

> **micro:bit v2 only.** This extension requires the microphone and audio processing capabilities
> of the micro:bit v2 hardware. It will not compile or run on micro:bit v1.

## Blocks

### ``started note`` — run code when a note begins

```sig
frequencies.startNote(function (note, cents) {})
```

Runs the handler once each time a new note is detected as playing. Use this to react to the
beginning of a note.

**Parameters**

- **note** — the detected `Note` (e.g. `Note.A4`)
- **cents** — pitch deviation from the exact note frequency; divide by 40 to get cents (−50 to +50)

**Example** — display the note name when any note starts:

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

Runs the handler once each time a note is no longer detected. Pairs with `startNote` to track
note on/off events.

**Parameters**

- **note** — the `Note` that stopped
- **cents** — last known pitch deviation (divide by 40 for cents)

**Example** — clear the display when any note stops:

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

Runs the handler every detection cycle (~every 0.37 s) while a note remains active. Use this to
continuously track pitch or update a display while a note is sustained.

**Parameters**

- **note** — the `Note` currently playing
- **cents** — current pitch deviation (divide by 40 for cents)

**Example** — show whether the note is sharp or flat while playing:

```blocks
frequencies.playingNote(function (note, cents) {
    if (cents / 40 > 10) {
        basic.showString("+")
    } else if (cents / 40 < -10) {
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

Runs the handler every detection cycle for one specific note, reporting its raw power and pitch
deviation regardless of whether the note is considered "playing." Useful for building tuning
meters or monitoring the strength of a particular pitch at all times.

**Parameters**

- **note** — the specific `Note` to monitor (chosen from the dropdown)
- **theNote** — the `Note` being monitored (same as the input)
- **power** — normalized power for this note, 0 (silence) to 1000 (full scale)
- **cents** — pitch deviation (divide by 40 for cents, −50 to +50)

**Example** — LED tuning meter for A4:

```blocks
frequencies.watchNote(Note.A4, function (theNote, power, cents) {
    if (power > 500) {
        let deviation = cents / 40
        if (deviation < -20) {
            basic.showLeds(`
                . . . . .
                . . . . .
                . # . . .
                . . . . .
                . . . . .`)
        } else if (deviation > 20) {
            basic.showLeds(`
                . . . . .
                . . . . .
                . . . # .
                . . . . .
                . . . . .`)
        } else {
            basic.showLeds(`
                . . . . .
                . . . . .
                . . # . .
                . . . . .
                . . . . .`)
        }
    }
})
```

---

## Advanced blocks

These blocks expose the raw FFT results and are intended for custom detection logic or
debugging. They are available under the **Advanced** section in the block editor.

### ``getNotePower(noteIndex)``

```sig
frequencies.getNotePower(0)
```

Returns the normalized power for note index `noteIndex` (0 = C3, 35 = B5), scaled 0–1000.
After harmonic suppression has been applied. A value near 0 means silence at that pitch; 1000
means full-scale input.

### ``getNoteCents(noteIndex)``

```sig
frequencies.getNoteCents(0)
```

Returns the raw pitch deviation for note at `noteIndex`. Divide by 40 to convert to cents
(range: −50 to +50). Positive = sharp, negative = flat.

### ``getAvgNotePower()``

```sig
frequencies.getAvgNotePower()
```

Returns the mean normalized power across all 36 notes (0–1000 scale). Useful as a rough
indicator of overall sound level.

### ``getMaxNotePower()``

```sig
frequencies.getMaxNotePower()
```

Returns the highest normalized power among all 36 notes. The internal detection logic uses this
to set the threshold for "note on" decisions.

### ``getNumNotes()``

```sig
frequencies.getNumNotes()
```

Returns 36 — the number of notes tracked (C3 through B5).

### ``getBin(binIndex)``

```sig
frequencies.getBin(0)
```

Returns the raw squared magnitude of FFT bin `binIndex` (0–2047). Each bin represents a
frequency range of ~2.71 Hz. Bin 0 is DC; bin 2047 is ~5556 Hz. Use this to inspect the full
spectrum for debugging.

### ``on notes updated`` (advanced callback)

```sig
frequencies.doOnNotesUpdated()
```

Registers a handler that is called once per FFT frame (~every 0.37 s) with the results of the
latest detection. Inside this handler you can call `getNotePower(i)`, `getNoteCents(i)`,
`getAvgNotePower()`, and `getMaxNotePower()` to build completely custom detection logic.

### ``noteToString(note)``

```sig
frequencies.noteToString(Note.A4)
```

Converts a `Note` enum value to a human-readable string such as `"A4"`, `"C#3"`, or `"Bb5"`.

---

## Detection range and accuracy

| Property | Value |
|----------|-------|
| Notes detected | C3 – B5 (36 notes, 3 octaves) |
| Sample rate | ~11,111 Hz |
| FFT size | 4096 points |
| Frequency resolution | ~2.71 Hz per bin |
| Detection latency | ~0.37 s per frame |
| Pitch resolution | ±50 cents (reported in units of 1/40 cent) |

### Tips for best results

- **Hold the micro:bit close to the sound source.** The MEMS microphone is small; distance
  reduces signal level significantly.
- **Single sustained notes work best.** Detection is designed around one note at a time.
  Chord detection is approximate.
- **Lower octave notes (C3–B3) are harder to detect** because the microphone sensitivity
  drops at low frequencies and piano overtones are often stronger than the fundamental.
- **Minimize background noise.** The threshold is relative to the loudest frequency present,
  so broadband noise raises the floor for all notes.

### How detection works

Each frame, the extension:

1. Captures 4096 samples at ~11,111 Hz from the microphone.
2. Runs a 4096-point Q15 real FFT (CMSIS-DSP).
3. For each of the 36 notes, finds the power in the nearest FFT bin and applies
   parabolic interpolation to estimate the sub-bin pitch deviation.
4. Applies **harmonic suppression**: for each note, subtracts its estimated contribution
   to higher notes that fall near its overtones (harmonics 2–6). This helps the
   fundamental frequency win over its own overtones.
5. Reports notes whose power exceeds a threshold derived from the frame's peak power.
   A note must exceed the threshold in two consecutive frames before a "started" event
   fires, and must fall below it in two consecutive frames before a "stopped" event fires.

---

# Acknowledgements

Icon based on [Font Awesome icon 0xF001](https://www.iconfinder.com/search?q=f001) SVG.

CMSIS-DSP library (Q15 FFT implementation) copyright ARM Limited, licensed under
[Apache 2.0](cmsis-dsp/LICENSE).

---

# Support

I develop micro:bit extensions in my spare time to support activities I'm enthusiastic about,
like summer camps and science curricula. You are welcome to become a sponsor of my micro:bit
work (one time or recurring payments), which helps offset equipment costs:
[github.com/sponsors/bsiever](https://github.com/sponsors/bsiever). Any support at all is
greatly appreciated!

## Supported targets

for PXT/microbit (v2 only)

<script src="https://makecode.com/gh-pages-embed.js"></script>
<script>makeCodeRender("{{ site.makecode.home_url }}", "{{ site.github.owner_name }}/{{ site.github.repository_name }}");</script>
