
basic.showString("St")
serial.writeLine("Start")
// Show "ping" ever 5 s
// loops.everyInterval(5000, function () {
//     serial.writeLine("ping")
// })

// See if any notes have changed
frequencies.startNote(function (note: Note, cents: number) {
    serial.writeLine("Started " + frequencies.noteToString(note) + " with cents " + cents)
    // Convert note into a number from 0-24 and plot it on the LED display
    let noteNumber = frequencies.getNoteIndex(note);
    led.plot(noteNumber % 5, Math.floor(noteNumber / 5))

})

// See if any notes have changed
frequencies.stopNote(function (note: Note, cents: number) {
    serial.writeLine("Stopped " + frequencies.noteToString(note) + " with cents " + cents)
    let noteNumber = frequencies.getNoteIndex(note);
    led.unplot(noteNumber % 5, Math.floor(noteNumber / 5))
})

// // See if any notes have changed
// frequencies.playingNote(function (note: Note, cents: number) {
//     serial.writeLine("Playing " + frequencies.noteToString(note) + " with cents " + cents)
// })


// Add a handler to watch C4 and print it's power and cents value
// frequencies.watchNote(Note.C4, function (note: Note, power: number, cents: number) {
//         if(power > 0) {
//             serial.writeLine("C4 power " + power + " cents " + cents)
//         }
//     })
// frequencies.watchNote(Note.CSharp4, function (note: Note, power: number, cents: number) {
//         if(power > 0) {
//             serial.writeLine("C#4 power " + power + " cents " + cents)
//         }
//     })
// frequencies.watchNote(Note.B3, function (note: Note, power: number, cents: number) {
//         if(power > 0) {
//             serial.writeLine("B3 power " + power + " cents " + cents)
//         }
// })
    

// frequencies.watchMaxPower(function (lowFrequency: number, highFrequency: number, power: number) {
//     if (power > 10000) {
//         let noteBelow = frequencies.getNoteForBelowOrEqual(lowFrequency);
//         let noteAbove = frequencies.getNoteForAboveOrEqual(highFrequency);
//         let noteBelowString = frequencies.noteToString(noteBelow);
//         let noteAboveString = frequencies.noteToString(noteAbove);
//         serial.writeLine(noteBelowString + " - " + noteAboveString + "   Max power " + power + "  (" + lowFrequency + "-" + highFrequency + " Hz)" )
//         // Alternative way to get note strings without calling noteToString multiple times:
//         basic.clearScreen();
//         let noteNumber = frequencies.getNoteIndex(noteBelow);
//         led.plot(noteNumber % 5, Math.floor(noteNumber / 5))
//         noteNumber = frequencies.getNoteIndex(noteAbove);
//         led.plot(noteNumber % 5, Math.floor(noteNumber / 5))
        
//     }
// })


frequencies.watchNote(Note.C4, function (theNote, power, cents) {
    if (power > 0) {
        basic.showIcon(IconNames.Heart,0)
    } else {
        basic.clearScreen()
    }
})

input.onButtonPressed(Button.A, function () {
    // Enable microphone sampling 

    serial.writeLine("Sampling")

})
