
basic.showString("Start")
serial.writeLine("Start")

// Show "ping" ever 5 s
// loops.everyInterval(5000, function () {
//     serial.writeLine("ping")
// })


input.onButtonPressed(Button.A, function () {
    // Enable microphone sampling 

    serial.writeLine("Sampling")

    frequencies.dumpSamples()
})
