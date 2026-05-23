
//% color=#6a8694
//% icon="\uf001"
//% block="Frequencies"
//% groups="['Frequencies']"
namespace frequencies {

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
        FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange.Stop, FrequencyChange_Stop,
        FrequencyChange.Stop, FrequencyChange.Stop
    ];

    // Define "matchers", which are a note and tolerance pair as well as an action
    class Matcher {
        constructor(public note: Note, public tolerance: number, public action: Action) { }
    }

    // Create a list of matchers that includes a note and a tolerance to Action
    let matchers: Matcher[] = [];

    // /**
    //  * Select a note (frequency in Hz); Tones that trigger will match the nearest note
    //  */
    // //% block="on note $note"
    // //% note.fieldEditor="note" note.defl="262"
    // //% note.fieldOptions.decompileLiterals=true
    // //% tolerance.max=100 tolerance.min=1 tolerance.defl=5
    // export function onNote(note: Note, body: Action) {
    //     // Tolerance of -1 is 
    //     matchers.push(new Matcher(note, -1, body));
    // }

//    /**
//      * Select a note (frequency in Hz) and tolerance (also in Hz) to trigger the block.
//      */
//     //% block="on note $note with tolerance $tolerance"
//     //% note.fieldEditor="note" note.defl="262"
//     //% note.fieldOptions.decompileLiterals=true
//     //% tolerance.max=100 tolerance.min=1 tolerance.defl=5
//     export function onNoteWithTolerance(note: Note, tolerance: number, body: Action) {
//         matchers.push(new Matcher(note, tolerance, body));
//     }

    /**
     */
    //% block="detected $change in frequency between $lower Hz and $upper Hz"
    //% draggableParameters="reporter"
    //% lower.min=1 lower.max=5500 lower.defl=420
    //% upper.min=1 upper.max=5500 upper.defl=440
    //% weight=200
    export function detectedFrequencyChangeBetween(lower: number, upper: number, handler: (change: FrequencyChange) => void) {
        handler(FrequencyChange.Start)
    } 

    /**
     */
    //% block="detected $change in $note"
    //% draggableParameters="reporter"
    //% weight=500
    export function detectedChangeInNoteNote(handler: (note: Note, change: FrequencyChange) => void) {
        handler(Note.C, FrequencyChange.Start)
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


    /**
     */
    //% block="dominant $frequency $magnitude"
    //% draggableParameters="reporter"
    //% advanced=true
    //% weight=200
    export function dominantFrequencyAndMagnitude(handler: (frequency: number, magnitude: number) => void) {
        handler(0,0)
    } 
}
