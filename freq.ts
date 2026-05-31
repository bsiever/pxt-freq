
    enum FrequencyChange {
        //% block="starting",
        Starting,
        //% block="start"
        Playing,
        //% block="stop"
        Stop, 
        //% block="stopping"
        Stopping
     }


//% color=#6a8694
//% icon="\uf001"
//% block="Frequencies"
//% groups="['Frequencies', 'Advanced']"
namespace frequencies {

    // Array of Frequency status for all 48 notes (C3–B6)
    let noteStatus: FrequencyChange[] = [
        FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop,
        FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop,
        FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop,
        FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop,
        FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop,
        FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop,
        FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop,
        FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop,
    ];

    // Array of note handlers that will be called when a note is detected as starting or stopping
    let noteStartHandlers: ((note: Note, cents: number) => void)[] = [];
    let noteStopHandlers: ((note: Note, cents: number) => void)[] = [];
    let notePlayingHandlers: ((note: Note, cents: number) => void)[] = [];

    // Note watch handlers are a combination of a note (to filter for) and a handler that is called with the power and cents for that note each time it is updated
    let noteWatchHandlers: [Note, (note: Note, power: number, cents: number) => void][] = [];
    let watchMaxPowerHandlers: ((lowFrequency: number, highFrequency: number, power: number) => void)[] = [];

    let initialized = false;

    const BIN_WIDTH = getBinWidth(); // Hz/bin

    // Array of notes mapping C++ index (0=C3 … B5) to Note enum values
    const notes = [
        Note.C3, Note.CSharp3, Note.D3, Note.Eb3, Note.E3, Note.F3,
        Note.FSharp3, Note.G3, Note.GSharp3, Note.A3, Note.Bb3, Note.B3,
        Note.C4, Note.CSharp4, Note.D4, Note.Eb4, Note.E4, Note.F4,
        Note.FSharp4, Note.G4, Note.GSharp4, Note.A4, Note.Bb4, Note.B4,
        Note.C5, Note.CSharp5, Note.D5, Note.Eb5, Note.E5, Note.F5,
        Note.FSharp5, Note.G5, Note.GSharp5, Note.A5, Note.Bb5, Note.B5
    ];

    const noteNames = [
        "C3", "C#3", "D3", "Eb3", "E3", "F3",
        "F#3", "G3", "G#3", "A3", "Bb3", "B3",
        "C4", "C#4", "D4", "Eb4", "E4", "F4",
        "F#4", "G4", "G#4", "A4", "Bb4", "B4",
        "C5", "C#5", "D5", "Eb5", "E5", "F5",
        "F#5", "G5", "G#5", "A5", "Bb5", "B5"
    ];

    const noteFrequencies = [
        130.81, 138.59, 146.83, 155.56, 164.81, 174.61,
        185.00, 196.00, 207.65, 220.00, 233.08, 246.94,
        261.63, 277.18, 293.66, 311.13, 329.63, 349.23,
        369.99, 392.00, 415.30, 440.00, 466.16, 493.88,
        523.25, 554.37, 587.33, 622.25, 659.25, 698.46,
        739.99, 783.99, 830.61, 880.00, 932.33, 987.77
    ];

    // ************* Primary User Blocks ******************
    /** Runs handler once when a new note begins. Receives the note and pitch deviation in cents (positive = sharp, negative = flat). */
    //% block="started $note ($cents)"
    //% draggableParameters="reporter"
    //% weight=4000
    export function startNote(handler: (note: Note, cents: number) => void) {
        // Add handler to collection of note handlers
        initialize() // ensure setup is called so that we have data for the listeners to process when they are added.    
        noteStartHandlers.push(handler)
    } 

    /** Runs handler once when a playing note stops. Receives the note that stopped and its last pitch deviation in cents. */
    //% block="stopped $note ($cents)"
    //% draggableParameters="reporter"
    //% weight=3900
    export function stopNote(handler: (note: Note, cents: number) => void) {
        // Add handler to collection of note handlers
        initialize() // ensure setup is called so that we have data for the listeners to process when they are added.    
        noteStopHandlers.push(handler)
    } 

    /** Runs handler each detection cycle (~0.37 s) while a note is sustained. Receives the note and current pitch deviation in cents. */
    //% block="playing $note ($cents)"
    //% draggableParameters="reporter"
    //% weight=3800
    export function playingNote(handler: (note: Note, cents: number) => void) {
        // Add handler to collection of note handlers
        initialize() // ensure setup is called so that we have data for the listeners to process when they are added.    
        notePlayingHandlers.push(handler)
    } 

    /** Runs handler each detection cycle for a specific note. power is 0–1000 (0 = silent, 1000 = loudest detected); cents is pitch deviation. */
    //% block="watch $note ($theNote, $power, $cents)"
    //% draggableParameters="reporter"
    //% weight=3700
    export function watchNote(note: Note, handler: (theNote: Note, power: number, cents: number) => void) {
        // Add handler to collection of note handlers
        initialize() // ensure setup is called so that we have data for the listeners to process when they are added.    
        noteWatchHandlers.push([note, handler])
    } 

    /** Converts a Note value to a readable string such as "A4" or "C#3". */
    //% block="note to string $note"
    //% note.shadow="device_note"
    //% weight=2000
    export function noteToString(note: number): string {
        // Switch statement to convert Note enum value to string
        for (let i = 0; i < notes.length; i++) {
            if (notes[i] == note) return noteNames[i];
        }
        return "";
    }

    //% shim=ENUM_GET
    //% blockId=note_enum_shim
    //% block="Note $arg"
    //% enumName="Notes"
    //% enumMemberName="note"
    //% enumPromptHint="e.g. C4, CSharp4, ..."
    //% enumInitialMembers="C4"
    //% weight=2000
    export function _noteEnumShim(arg: number) {
        // This function should do nothing, but must take in a single
        // argument of type number and return a number value.
        return arg;
    }

    // ************* Advanced User Blocks ******************

    /** Runs handler each detection cycle with the frequency range (Hz) and power of the single strongest FFT bin. */
    //% block="watch max power (low frequency $lowFrequency, high frequency $highFrequency, power $power)"
    //% draggableParameters="reporter"
    //% advanced=true
    //% weight=4000
    export function watchMaxPower(handler: (lowFrequency: number, highFrequency: number, power: number) => void) {
        initialize() // ensure setup is called so that we have data for the listeners to process when they are added.    
        watchMaxPowerHandlers.push(handler)
    }

    /** Returns the note whose FFT bin contains or falls at or below the given frequency (Hz). */
    //% block="note for frequency below or equal to $frequency"
    //% weight=3800
    //% advanced=true
    export function getNoteForBelowOrEqual(frequency: number): Note {
        // Iterate through note frequencies:
        // Move the frequency to the middle of it's bin
        let i = 0;
        while (i < noteFrequencies.length) {
            let binIndex = Math.floor(noteFrequencies[i] / BIN_WIDTH);
            let lowFrequency = binIndex * BIN_WIDTH;
            let highFrequency = lowFrequency + BIN_WIDTH;


            // If it's in this bucket
            if(frequency==lowFrequency)
                return getNote(i);
            else if (frequency < lowFrequency) {
                return getNote(i-1);
            }
            i++;
        }
        return getNote(noteFrequencies.length - 1);
    }

    /** Returns the note whose FFT bin contains or falls at or above the given frequency (Hz). */
    //% block="note for frequency above or equal to $frequency"
    //% weight=3700
    //% advanced=true
    export function getNoteForAboveOrEqual(frequency: number): Note {
        let i = noteFrequencies.length - 2;
        while (i >= 0) {
            let binIndex = Math.ceil(noteFrequencies[i] / BIN_WIDTH);
            let highFrequency = binIndex * BIN_WIDTH;
            if(highFrequency == frequency)
                return getNote(i);
            if (highFrequency < frequency) 
                return getNote(i+1);
            i--;
        }
        return getNote(0);  
    } 

    /** Convert a Note enum to an index */
    //% block="index of $note"
    //% weight=2000
    //% advanced=true
    //% note.shadow="device_note"
    export function getNoteIndex(note: Note): number {
        for (let i = 0; i < notes.length; i++) {
            if (notes[i] == note) return i;
        }
        return -1;
    }


    /** Convert an index to a Note enum */
    //% block="note at index $index"
    //% weight=1000
    //% advanced=true
    export function getNote(index: number): Note {
        index = Math.max(0, Math.min(index, notes.length - 1));
        return notes[index];
    }


    // ************* Private / Internal Functions ******************

    export function initialize() {
        if(initialized) return;
        initialized = true;
        onNotesUpdated(doOnNotesUpdated);
        setup();
    }

    function doOnNotesUpdated() {
        // Print max power , average power
        
         // If any handlers for frequency watching
         let maxBinPower = getMaxBinPower();
         let maxBinIndex = getMaxBinIndex();
         let binWidth = getBinWidth();
         let lowFrequency = maxBinIndex * binWidth;
         let highFrequency = (maxBinIndex + 1) * binWidth;
         watchMaxPowerHandlers.forEach(h => h(lowFrequency, highFrequency, maxBinPower))
                  
        // if maxPower > 5x average power, print out all notes that are >50% of max power
        let maxPower = getMaxNotePower();
        let avgPower = getAvgNotePower();

        let threshold = maxPower;
        if (maxPower > 8 * avgPower && maxPower > 8000) {
            threshold = maxPower / 2;
        } else {
            // No notes playing
            threshold = maxPower * 2; // minimum threshold to consider a note "on"
        }

        for (let item of noteWatchHandlers) {
            let [note, handler] = item;
            let noteIndex = getNoteIndex(note);
            let power = getNotePower(noteIndex);
            let cents = getNoteCents(noteIndex)/40.0;
            if (power > threshold) {
                power = power / maxPower * 1000; // rescale power to be out of 1000 for the handler
            } else {
                power = 0;
                cents = 0;
            }
            handler(note, power, cents);
        }

        // State machine to iterate through all notes and update the state and handlers)
        for(let i = 0; i < getNumNotes(); i++) {
            let power = getNotePower(i);
            let cents = getNoteCents(i)/40.0;
            switch (noteStatus[i]) {
                
                case FrequencyChange.Stop:
                    if (power > threshold) {
                        noteStatus[i] = FrequencyChange.Starting;
                    }
                    break;

                case FrequencyChange.Starting:
                    if (power > threshold) {
                        noteStatus[i] = FrequencyChange.Playing;
                        noteStartHandlers.forEach(h => h(getNote(i), cents))
                        notePlayingHandlers.forEach(h => h(getNote(i), cents))
                    } else {
                        noteStatus[i] = FrequencyChange.Stop;
                    }
                    break;

                case FrequencyChange.Playing:
                    if (power < threshold) {
                        noteStatus[i] = FrequencyChange.Stopping;
                    } else {
                        notePlayingHandlers.forEach(h => h(getNote(i), cents))
                    }
                    break;

                case FrequencyChange.Stopping:
                    if (power < threshold) {
                        noteStatus[i] = FrequencyChange.Stop;
                        noteStopHandlers.forEach(h => h(getNote(i), 0))
                    } else {
                        noteStatus[i] = FrequencyChange.Playing;
                        notePlayingHandlers.forEach(h => h(getNote(i), cents))
                    }
                    break;
            }
        }   
     }
    
    // ************* Private Shims to C++ ******************

    // TODO: These probably don't need to be exported 
    
    // Shim for onNotesUpdated - called by C++ when new results are available
    //% shim=frequencies::onNotesUpdated
    export function onNotesUpdated(handler: Action) { 0; }

    /** Returns the raw squared magnitude of FFT bin binIndex (0–2047). Each bin spans ~2.71 Hz. Bin 0 is DC; bin 2047 is ~5556 Hz. */
    //% shim=frequencies::getBin
    export function getBin(_binIndex: number): number { return 0; }

    /** Returns 36 — the number of tracked notes (C3 through B5). */
    //% shim=frequencies::getNumNotes
    export function getNumNotes(): number { return 36; }

    /** Returns the mean normalized power across all 36 notes (0–1000 scale). */
    //% shim=frequencies::getAvgNotePower
    export function getAvgNotePower(): number { return 0; }

    /** Returns normalized power for note at index (0=C3, 35=B5), scaled 0–1000 after harmonic suppression. */
    //% shim=frequencies::getNotePower
    export function getNotePower(_noteIndex: number): number { return 0; }

    /** Returns raw pitch deviation for note at index. Divide by 40 to get cents; positive = sharp, negative = flat. */
    //% shim=frequencies::getNoteCents
    export function getNoteCents(_noteIndex: number): number { return 0; }

    //% shim=frequencies::setup
    export function setup() { 0; }

    /** Returns the highest normalized power among all 36 notes (0–1000 scale). */
    //% shim=frequencies::getMaxNotePower
    export function getMaxNotePower(): number { return 0; }

    /** Returns the squared magnitude of the highest-power FFT bin across the full spectrum. */
    //% shim=frequencies::getMaxBinPower
    export function getMaxBinPower(): number { return 0; }

    /** Returns the index (0–2047) of the highest-power FFT bin. Multiply by getBinWidth() to get Hz. */
    //% shim=frequencies::getMaxBinIndex
    export function getMaxBinIndex(): number { return 0; }

    /** Returns the width of each FFT bin in Hz (~2.71 Hz). */
    //% shim=frequencies::getBinWidth
    export function getBinWidth(): number { return 0; }

}

