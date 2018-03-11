const http = require('http')

const options = {
  hostname: '192.168.1.151',
  port: 80,
  path: '/listen',
  method: 'POST',
  headers: {
    'Connection': 'keep-alive',
    'Transfer-Encoding': 'chunked'
  }
};

const req = http.request(options, (res) => {
  console.log(`STATUS: ${res.statusCode}`);
  console.log(`HEADERS: ${JSON.stringify(res.headers)}`);
  res.setEncoding('utf8');
  res.on('data', (chunk) => {
    console.log(`BODY: ${chunk}`);
  });
  res.on('end', () => {
    console.log('No more data in response.');
  });
});

req.on('error', (e) => {
  console.error(`problem with request: ${e.message}`);
});

var writes = 0

const values = () => {
  var nums = []

  for (var i = 0; i < 1000; i++) {
    nums.push(Math.random() * 255)
  }

  return nums
}

function doAWrite() {
  req.write(values().toString())

  writes++

  setTimeout(doAWrite, 125)
}

doAWrite()
