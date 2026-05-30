
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

    /**
     */
    //% block="started $note ($cents)"
    //% draggableParameters="reporter"
    //% weight=500
    export function startNote(handler: (note: Note, cents: number) => void) {
        // Add handler to collection of note handlers
        setup() // ensure setup is called so that we have data for the listeners to process when they are added.    
        noteStartHandlers.push(handler)
    } 

    /**
     */
    //% block="stopped $note ($cents)"
    //% draggableParameters="reporter"
    //% weight=500
    export function stopNote(handler: (note: Note, cents: number) => void) {
        // Add handler to collection of note handlers
        setup() // ensure setup is called so that we have data for the listeners to process when they are added.    
        noteStopHandlers.push(handler)
    } 


    /**
     */
    //% block="playing $note ($cents)"
    //% draggableParameters="reporter"
    //% weight=500
    export function playingNote(handler: (note: Note, cents: number) => void) {
        // Add handler to collection of note handlers
        setup() // ensure setup is called so that we have data for the listeners to process when they are added.    
        notePlayingHandlers.push(handler)
    } 

    /**
     */
    //% block="watch $note ($theNote, $power, $cents)"
    //% draggableParameters="reporter"
    //% weight=700
    export function watchNote(note: Note, handler: (theNote: Note, power: number, cents: number) => void) {
        // Add handler to collection of note handlers
        setup() // ensure setup is called so that we have data for the listeners to process when they are added.    
        noteWatchHandlers.push([note, handler])
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

        // if (maxPower > 5 * avgPower && maxPower > 10000) {
        //     for (let i = 0; i < getNumNotes(); i++) {
        //         let power = getNotePower(i);
        //         if (power > maxPower / 2) {
        //             serial.writeLine("Note " + noteToString(getNote(i)) + " is loud with power " + power + " and cents " + getNoteCents(i))
        //         }
        //     }
        // }   

        // serial.writeLine("Max Power " + getMaxNotePower() + ", Avg Power  " + getAvgNotePower())
        // // Print out note name, not power, and cents for each note that has changed since the last update
        // for (let i = 0; i < getNumNotes(); i++) {
        //     // Print out the note name, note power, note cents
        //     serial.writeLine("Note " + noteToString(getNote(i)) + " = " + getNotePower(i) + " power, " + getNoteCents(i) + " cents")
        //     pause(100)
        // }   
        // // Print all bin values too
        // for(let i = 0; i < 2048 / 2; i++) {
        //     serial.writeLine("Bin " + i + " = " + getBin(i))
        // }

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

    export function inititialize() {
        onNotesUpdated(doOnNotesUpdated);
        setup();
    }

    export function getNoteIndex(note: Note): number {
        for (let i = 0; i < notes.length; i++) {
            if (notes[i] == note) return i;
        }
        return -1;
    }

}
