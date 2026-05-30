
basic.showString("St")
serial.writeLine("Start")
frequencies.inititialize()
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
frequencies.watchNote(Note.C4, function (note: Note, power: number, cents: number) {
        if(power > 0) {
            serial.writeLine("C4 power " + power + " cents " + cents)
        }
    })
frequencies.watchNote(Note.CSharp4, function (note: Note, power: number, cents: number) {
        if(power > 0) {
            serial.writeLine("C#4 power " + power + " cents " + cents)
        }
    })
frequencies.watchNote(Note.B3, function (note: Note, power: number, cents: number) {
        if(power > 0) {
            serial.writeLine("B3 power " + power + " cents " + cents)
        }
    })

input.onButtonPressed(Button.A, function () {
    // Enable microphone sampling 

    serial.writeLine("Sampling")

})
