const WebSocket = require('ws')

const ws = new WebSocket('ws://localhost:8080')

ws.on('open', function open() {
  for (var i = 0; i < 500; i++) {
    ws.send("6275168BA33F24CE29069C9A393238996ECD11D8FC7E5B6391AA588CC0F60F7D")
    ws.send("6275168BA33F24CE29069C9A393238996ECD11D8FC7E5B6391AA588CC0F60F7D")
    // ws.send('4A8ED0FE6A5BDFAA6D349137CE94336BCEE05C3E2FDDA90B2E8C12CE9FE7AF82')
  }

  process.exit()
})

ws.on('message', function incoming(data) {
  console.log(data)
})
