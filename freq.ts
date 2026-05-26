
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



    // Array of notes mapping C++ index (0=C3 … B5) to Note enum values
    const note = [
        Note.C3, Note.CSharp3, Note.D3, Note.Eb3, Note.E3, Note.F3,
        Note.FSharp3, Note.G3, Note.GSharp3, Note.A3, Note.Bb3, Note.B3,
        Note.C4, Note.CSharp4, Note.D4, Note.Eb4, Note.E4, Note.F4,
        Note.FSharp4, Note.G4, Note.GSharp4, Note.A4, Note.Bb4, Note.B4,
        Note.C5, Note.CSharp5, Note.D5, Note.Eb5, Note.E5, Note.F5,
        Note.FSharp5, Note.G5, Note.GSharp5, Note.A5, Note.Bb5, Note.B5
    ];

    // Array to convert Note enum values to note name strings for printing to the console
    const noteNames = [
        "C3", "C#3", "D3", "Eb3", "E3", "F3",
        "F#3", "G3", "G#3", "A3", "Bb3", "B3",
        "C4", "C#4", "D4", "Eb4", "E4", "F4",
        "F#4", "G4", "G#4", "A4", "Bb4", "B4",
        "C5", "C#5", "D5", "Eb5", "E5", "F5",
        "F#5", "G5", "G#5", "A5", "Bb5", "B5"
    ];

    // Function to convert Note enum value to string
    export function noteToString(note: Note): string {
        // Switch statement to convert Note enum value to string
        switch (note) {
            case Note.C3: return "C3";
            case Note.CSharp3: return "C#3";
            case Note.D3: return "D3";
            case Note.Eb3: return "Eb3";
            case Note.E3: return "E3";
            case Note.F3: return "F3";
            case Note.FSharp3: return "F#3";
            case Note.G3: return "G3";
            case Note.GSharp3: return "G#3";
            case Note.A3: return "A3";
            case Note.Bb3: return "Bb3";
            case Note.B3: return "B3";
            case Note.C4: return "C4";
            case Note.CSharp4: return "C#4";
            case Note.D4: return "D4";
            case Note.Eb4: return "Eb4";
            case Note.E4: return "E4";
            case Note.F4: return "F4";
            case Note.FSharp4: return "F#4";
            case Note.G4: return "G4";
            case Note.GSharp4: return "G#4";
            case Note.A4: return "A4";
            case Note.Bb4: return "Bb4";
            case Note.B4: return "B4";
            case Note.C5: return "C5";
            case Note.CSharp5: return "C#5";
            case Note.D5: return "D5";
            case Note.Eb5: return "Eb5";
            case Note.E5: return "E5";
            case Note.F5: return "F5";
            case Note.FSharp5: return "F#5";
            case Note.G5: return "G5";
            case Note.GSharp5: return "G#5";
            case Note.A5: return "A5";
            case Note.Bb5: return "Bb5";
            case Note.B5: return "B5";
            default: return "";
        }
    }

    /** Convert an index to a Note enum */
    function getNote(index: number): Note {
        return note[index];
    }

    // Shim for onNotesUpdated - called by C++ when new results are available
    //% shim=frequencies::onNotesUpdated
    export function onNotesUpdated(handler: Action) {
    }

    /**
     * Register a handler called each time note detection completes.
     * avgPower is the mean normalized power across all notes, scaled by 1000.
     * Use getNotePower(i) and getNoteCents(i) inside the handler to read per-note results.
     */
    //% block="on notes updated (avgPower $avgPower)"
    //% draggableParameters="reporter"
    //% weight=100
    //% advanced=true
    export function doOnNotesUpdated() {
        let avgPower = getAvgNotePower();
        let maxPower = getMaxNotePower();
        let medianPower = (avgPower - maxPower) / 2;
        let threshold = 10 * avgPower;
        // Iterate through each "Note" and determine if it has started or stopped since the last time we checked
        for (let i = 0; i < note.length; i++) {
            let power = Math.abs(getNotePower(i));
            let cents = getNoteCents(i);
            // Show the average power 
            if (power > threshold) {
                switch (noteStatus[i]) {
                    case FrequencyChange.Stop:
                    case FrequencyChange.Stopping:
                        // Note has started since last time we checked
                        noteStatus[i] = FrequencyChange.Starting;
                        break;
                    case FrequencyChange.Starting:
                        // Note has started since last time we checked
                        noteStatus[i] = FrequencyChange.Playing;
                        noteStartHandlers.forEach(h => h(note[i], cents));
                        notePlayingHandlers.forEach(h => h(note[i], cents));
                        break;
                    case FrequencyChange.Playing:
                        // Note is still playing since last time we checked
                        noteStatus[i] = FrequencyChange.Playing;
                        notePlayingHandlers.forEach(h => h(note[i], cents));
                        break;
                }
            } else {
                switch (noteStatus[i]) {
                    case FrequencyChange.Playing:
                        // Note has stopped since last time we checked
                        noteStatus[i] = FrequencyChange.Stopping;
                        break;
                    case FrequencyChange.Stopping:
                        // Note has stopped since last time we checked
                        noteStatus[i] = FrequencyChange.Stop;
                        noteStopHandlers.forEach(h => h(note[i], cents));
                        break;
                }
            }
        }
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

}
