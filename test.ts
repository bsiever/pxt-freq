
basic.showString("St")
serial.writeLine("Start")
frequencies.inititialize()
// Show "ping" ever 5 s
// loops.everyInterval(5000, function () {
//     serial.writeLine("ping")
// })

// See if any notes have changed
frequencies.detectedChangeInNote(function (note: Note, change: FrequencyChange, cents: number) {
    serial.writeLine("Average power " + frequencies.getAvgNotePower());
    serial.writeLine("Note " + frequencies.noteToString(note) + " " + (change == FrequencyChange.Start ? "started" : "stopped") + " with cents " + cents)
})

input.onButtonPressed(Button.A, function () {
    // Enable microphone sampling 

    serial.writeLine("Sampling")

    frequencies.dumpSamples()
})
