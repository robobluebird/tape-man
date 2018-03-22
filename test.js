const WebSocket = require('ws')
const ws = new WebSocket('ws://localhost:8080')

let tones = [generateTone(440), generateTone(440), generateTone(440)]
let readyToSend = []

ws.on('open', function open() {
  for (var frequencyIndex = 0; frequencyIndex < tones.length; frequencyIndex) {
    for(var toneIndex = 0; toneIndex < tones[frequencyIndex].length; toneIndex += 500) {
      let hexString = ''

      for (var counter = 0; counter < 500; counter++) {
        let asHex = tones[frequencyIndex][toneIndex + counter].toString(16).toUpperCase()

        hexString = hexString.concat('00'.substring(asHex.length) + asHex)
      }

      // var stop = new Date().getTime();
      //
      // while(new Date().getTime() < stop + 100) {
      //   ;
      // }

      ws.send(hexString)
    }
  }
})

ws.on('message', function incoming(data) {
  console.log(data)
})

function generateTone(frequency) {
  let ary = new Uint8Array(16000)
  let angularFrequency = frequency * 2 * Math.PI

  for (let sampleNumber = 0 ; sampleNumber < ary.length ; sampleNumber++) {
    ary[sampleNumber] = generateSample(sampleNumber, angularFrequency);
  }

  return ary
}

function generateSample(sampleNumber, angularFrequency) {
  let sampleTime = sampleNumber / 8000;
  let sampleAngle = sampleTime * angularFrequency;

  return stretch(Math.sin(sampleAngle));
}

// convert value from range (-1..1) to (0..255)
function stretch(sinWaveComponentValue) {
  return (((sinWaveComponentValue - -1) * 255) / 2)
}
