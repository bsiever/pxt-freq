
//% color=#6a8694
//% icon="\uf001"
//% block="Frequencies"
//% groups="['Frequencies']"
namespace frequencies {

    enum FrequenceyChange {
        //% block="start"
        Start,
        //% block="stop"
        Stop
    }

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
    //% block="on frequence between $lower Hz and $upper Hz with status $status"
    //% draggableParameters="reporter"
    //% lower.min=1 lower.max=5500 lower.defl=420
    //% upper.min=1 upper.max=5500 upper.defl=440
    export function onFrequencyBetween(lower: number, upper: number, handler: (change: FrequenceyChange) => void) {
        handler(FrequenceyChange.Start)
    } 

    /**
     */
    //% block="on note $note with $change"
    //% draggableParameters="reporter"
    export function onNote(handler: (note: Note, change: FrequenceyChange) => void) {
        handler(Note.C, FrequenceyChange.Start)
    } 

    //% block="set detection threshold to $threshold Hz"
    export function setDetectionThreshold(threshold: number) {

    }
}
