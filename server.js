const webSocket = require('ws')
const http = require('http')
const fs = require('fs')

let server = http.createServer((request, response) => {
  if (request.method === 'GET') {
    if (request.url === '/') {
      fs.readFile("index.html", function(err, html) {
        response.writeHead(200, {'Content-Type': 'text/html'});
        response.end(html)
      })
    } else if (request.url == '/favicon.ico') {
      response.setHeader('Content-Type', 'image/x-icon');
      fs.createReadStream('favicon.ico').pipe(response);
    }
  }
})

const wss = new webSocket.Server({ server })

wss.on('connection', function connection(ws, request) {
  ws.on('message', function incoming(message) {
    let data = new Uint8Array(message.match(/.{1,2}/g).map(x => parseInt(x, 16)))

    wss.clients.forEach(function each(client) {
      if (client !== ws && client.readyState === webSocket.OPEN) {
        client.send(data)
      }
    })
  })
})

server.listen(process.env.PORT || 8080)
