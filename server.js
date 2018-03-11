const webSocket = require('ws')
const http = require('http')
const fs = require('fs')
var data = new Uint8Array(1000000)
var offset = 0

let server = http.createServer((request, response) => {
  if (request.method === 'GET') {
    if (request.url === '/') {
      fs.readFile("public/index.html", function(err, html) {
        response.writeHead(200, {'Content-Type': 'text/html'});
        response.end(html)
      })
    } else if (request.url === '/data') {
      data.set(new Uint8Array([1,2,3]), 0)
      offset = offset + 3

      var a1 = new Uint8Array(offset)
      var b1 = Buffer.from(a1)
      var b2 = Buffer.from(data)

      b2.copy(b1, 0, 0, offset)

      response.end(b1)

      data = new Uint8Array(1000000)
      offset = 0
    }
  }
})

// const wss = new webSocket.Server({
//   verifyClient: (info, done) => {
//     console.log('Parsing session from request...')
//
//     console.log(info.req)
//     console.log(
//     sessionParser(info.req, {}, () => {
//       console.log('Session is parsed!')
//       done(info.req.session.userId)
//     })
//   },
//   server
// })
//

const wss = new webSocket.Server({ server })

wss.on('connection', function connection(ws, request) {
  ws.on('message', function incoming(message) {
    let tmp = new Uint8Array(message.match(/.{1,2}/g).map(x => parseInt(x, 16)))

    data.set(tmp, offset)
    offset = offset + tmp.byteLength

    wss.clients.forEach(function each(client) {
      if (client !== ws && client.readyState === WebSocket.OPEN) {
        client.send(data);
      }
    })
  })
})

server.listen(8080)
