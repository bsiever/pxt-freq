
//% color=#6a8694
//% icon="\uf001"
//% block="Frequencies"
//% groups="['Frequencies']"
namespace frequencies {


    // TODO: Setup timer / interrupt to do frequency analysis every X ms and call the appropriate handlers when we detect changes in frequency or magnitude.  We can use the CMSIS DSP library for the FFT and frequency analysis.  We should also have a function that allows users to set the detection threshold for detecting changes in frequency, so that they can adjust the sensitivity of the frequency change detection.
    // Then update FrequencyChange records and call all three types of handlers if needed. 


    enum FrequencyChange {
        //% block="start"
        Start,
        //% block="stop"
        Stop
    }

    // Array of Frequency status for all 36 notes (from low C to high C, with flats and sharps)
    let noteStatus: FrequencyChange[] = [
        FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop,
        FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop,
        FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop,
        FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop,
        FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop,
        FrequencyChange.Stop, FrequencyChange.Stop
    ];

    // Array of frequencies for the 36 notes from C3 to B6
    let noteFrequencies: number[] = [
        130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185.00, 196.00, 207.65, 220.00, 233.08, 246.94,
        261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.00, 415.30, 440.00, 466.16, 493.88,
        523.25, 554.37, 587.33, 622.25, 659.25, 698.46, 739.99, 783.99
    ];
        

    // Convert a frequency to a note index (0-35, corresponding to the 36 notes from low C to high C, with flats and sharps)
    // Find the "closest" note based on eucledean distance between the frequency and the note frequencies, and return that note's index.  If the frequency is exactly in between two notes, return the higher note.
    // Do binary search to find the closest note and then check the neighbors and use euclidean distance to determine which is closest.
    function frequencyToNoteIndex(frequency: number): number {
        let low = 0;
        let high = noteFrequencies.length - 1;
        while (low <= high) {
            let mid = Math.floor((low + high) / 2);
            if (noteFrequencies[mid] === frequency) {
                return mid;
            } else if (noteFrequencies[mid] < frequency) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        // low is now the index of the smallest note that is higher than the frequency, and high is the index of the largest note that is lower than the frequency.  Check which of these two notes is closer to the frequency and return that index.  If low is out of bounds, return high.  If high is out of bounds, return low.
        if (low >= noteFrequencies.length) {
            return high;
        }
        if (high < 0) {
            return low;
        }
        let lowDiff = noteFrequencies[low] - frequency;
        let highDiff = frequency - noteFrequencies[high];
        if (lowDiff < highDiff) {
            return low;
        } else {
            return high;
        }
    }


    // Convert a Note to a string for display in the block editor
    // Include the number as well, e.g., A4, C#3, etc.
    function noteToString(noteIndex: number): string {
        let noteNames = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];
        let octave = Math.floor(noteIndex / 12) - 1; // Calculate octave number based on index
        let noteName = noteNames[noteIndex % 12]; // Get note name based on index
        return noteName + octave.toString(); // Combine note name and octave for display
    }   

    // // Convert a raw frequency to the closest Note object (closest in euclidean distance between the frequency and the note frequencies).  If the frequency is exactly in between two notes, return the higher note.
    // function frequencyToNote(frequency: number): Note {
    //     let noteIndex = frequencyToNoteIndex(frequency);
    //     return Note.create(noteIndex, noteToString(noteIndex), noteFrequencies[noteIndex]);
    // }


    // Array of note handlers that will be called when a note is detected as starting or stopping
    let noteHandlers: ((note: Note, change: FrequencyChange) => void)[] = [];
    // Array of frequency handlers that will be called when a frequency change is detected within a certain range
    let frequencyHandlers: ((change: FrequencyChange, frequency: number) => void)[] = [];
    // Array of dominant frequency handlers that will be called when a dominant frequency and magnitude is detected
    let dominantFrequencyHandlers: ((frequency: number, magnitude: number) => void)[] = [];

    /**
     */
    //% block="detected $change in frequency between $lower Hz and $upper Hz"
    //% draggableParameters="reporter"
    //% lower.min=1 lower.max=5500 lower.defl=420
    //% upper.min=1 upper.max=5500 upper.defl=440
    //% weight=200
    export function detectedFrequencyChangeBetween(lower: number, upper: number, handler: (change: FrequencyChange) => void) {
        // Add handler to collection of frequency handlers, along with the specified frequency range
        setup() // ensure setup is called so that we have data for the listeners to process when they are added.    
        frequencyHandlers.push((change: FrequencyChange, frequency: number) => {
            if (frequency >= lower && frequency <= upper) {
                handler(change)
            }
        })  
    } 
    /**
     */
    //% block="detected $change in $note"
    //% draggableParameters="reporter"
    //% weight=500
    export function detectedChangeInNote(handler: (note: Note, change: FrequencyChange) => void) {
        // Add handler to collection of note handlers
        setup() // ensure setup is called so that we have data for the listeners to process when they are added.    
        noteHandlers.push(handler)
    } 

    //% block="set detection threshold to $threshold"
    //% advanced=true
    //% weight=500
    export function setDetectionThreshold(threshold: number) {

    }

    //% block
    //% shim=frequencies::dumpSamples
    //% advanced=true
    //% weight=50
    export function dumpSamples() {

    }

    // Shim for init
    //% shim=frequencies::setup
    export function setup() {

    }

}
