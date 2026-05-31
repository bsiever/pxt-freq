
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
//% groups="['Frequencies']"
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

    /**
     */
    //% block="started $note ($cents)"
    //% draggableParameters="reporter"
    //% weight=500
    export function startNote(handler: (note: Note, cents: number) => void) {
        // Add handler to collection of note handlers
        initialize() // ensure setup is called so that we have data for the listeners to process when they are added.    
        noteStartHandlers.push(handler)
    } 


    /**
     */
    //% block="stopped $note ($cents)"
    //% draggableParameters="reporter"
    //% weight=500
    export function stopNote(handler: (note: Note, cents: number) => void) {
        // Add handler to collection of note handlers
        initialize() // ensure setup is called so that we have data for the listeners to process when they are added.    
        noteStopHandlers.push(handler)
    } 

    //% block="watch max power (low frequency $lowFrequency, high frequency $highFrequency, power $power)"
    //% draggableParameters="reporter"
    //% weight=700
    export function watchMaxPower(handler: (lowFrequency: number, highFrequency: number, power: number) => void) {
        initialize() // ensure setup is called so that we have data for the listeners to process when they are added.    
        watchMaxPowerHandlers.push(handler)
    }

    /**
     */
    //% block="playing $note ($cents)"
    //% draggableParameters="reporter"
    //% weight=500
    export function playingNote(handler: (note: Note, cents: number) => void) {
        // Add handler to collection of note handlers
        initialize() // ensure setup is called so that we have data for the listeners to process when they are added.    
        notePlayingHandlers.push(handler)
    } 

    /**
     */
    //% block="watch $note ($theNote, $power, $cents)"
    //% draggableParameters="reporter"
    //% weight=700
    export function watchNote(note: Note, handler: (theNote: Note, power: number, cents: number) => void) {
        // Add handler to collection of note handlers
        initialize() // ensure setup is called so that we have data for the listeners to process when they are added.    
        noteWatchHandlers.push([note, handler])
    } 

    const BIN_WIDTH=getBinWidth(); // Hz/bin

    // Given a specific frequency, if it's in a bin for a specific note, return that note.  Otherwise return the closest note below the given frequency
    //% block="note for frequency below or equal to $frequency"
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

    // Given a specific frequency, if it's in a bin for a specific note, return that note.  Otherwise return the closest note above the given frequency
    //% block="note for frequency above or equal to $frequency"
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

    //% shim=ENUM_GET
    //% blockId=note_enum_shim
    //% block="Note $arg"
    //% enumName="Notes"
    //% enumMemberName="note"
    //% enumPromptHint="e.g. C4, CSharp4, ..."
    //% enumInitialMembers="C4"
    export function _noteEnumShim(arg: number) {
        // This function should do nothing, but must take in a single
        // argument of type number and return a number value.
        return arg;
    }

    // Function to convert Note enum value to string
    //% block="note to string $note"
    //% note.shadow="device_note"
    //% weight=900    
    export function noteToString(note: number): string {
        // Switch statement to convert Note enum value to string
        for (let i = 0; i < notes.length; i++) {
            if (notes[i] == note) return noteNames[i];
        }
        return "";
    }

    /** Convert an index to a Note enum */
    function getNote(index: number): Note {
        index = Math.max(0, Math.min(index, notes.length - 1));
        return notes[index];
    }

    // Shim for onNotesUpdated - called by C++ when new results are available
    //% shim=frequencies::onNotesUpdated
    export function onNotesUpdated(handler: Action) {
        0;
    }

    /**
     * Register a handler called each time note detection completes.
     * avgPower is the mean normalized power across all notes, scaled by 1000.
     * Use getNotePower(i) and getNoteCents(i) inside the handler to read per-note results.
     */
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


    // Shim for fft bin access 
    //% shim=frequencies::getBin
    export function getBin(_binIndex: number): number {
        return 0;
    }

    /** Returns the number of notes tracked (C3–B6 = 48). */
    //% shim=frequencies::getNumNotes
    //% advanced=true
    export function getNumNotes(): number { return 48; }

    /** Returns the mean normalized power across all notes, scaled by 1000. */
    //% shim=frequencies::getAvgNotePower
    //% advanced=true
    export function getAvgNotePower(): number { return 0; }

    /**
     * Returns normalized power for note index i, scaled by 1000.
     * 0 = silence, 1000 = full-scale.
     */
    //% shim=frequencies::getNotePower
    //% advanced=true
    export function getNotePower(_noteIndex: number): number { return 0; }

    /**
     * Returns the pitch error for note index i.
     * Divide by 40 to get cents (range: −50 to +50).
     */
    //% shim=frequencies::getNoteCents
    //% advanced=true
    export function getNoteCents(_noteIndex: number): number { return 0; }

    // Shim for init
    //% shim=frequencies::setup
    export function setup() {
        0;
    }

    // Shim for getMaxNotePower
    //% shim=frequencies::getMaxNotePower
    export function getMaxNotePower(): number {
        return 0;
    }

    export function initialize() {
        if(initialized) return;
        initialized = true;
        onNotesUpdated(doOnNotesUpdated);
        setup();
    }

    export function getNoteIndex(note: Note): number {
        for (let i = 0; i < notes.length; i++) {
            if (notes[i] == note) return i;
        }
        return -1;
    }

    //% shim=frequencies::getMaxBinPower
    export function getMaxBinPower(): number {
        return 0;
    }

    //% shim=frequencies::getMaxBinIndex
    export function getMaxBinIndex(): number {
        return 0;
    }
    
    //% shim=frequencies::getBinWidth
    export function getBinWidth(): number {
        return 0;
    }

}

