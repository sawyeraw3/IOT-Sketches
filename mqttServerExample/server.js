import { publishToFeed } from './aioManager'
const express = require('express')
const app = express()
const fs = require('fs');
const server = require('http').Server(app);
const sio = require('socket.io')(server);
const port = 8080;

server.listen(port, () => console.log(`Listening on port ${port}...`));

app.use(express.static(__dirname + '/public'));
app.get('/', function (req, res) {
  res.sendFile(__dirname + '/public/index.html');
});

// Socket.io commands
sio.on('connection', function(socket){
  console.log('a user connected');
  console.log();
  socket.on('disconnect', function(){
    console.log('user disconnected');
    console.log();
  });

  socket.on('aioEvent', (e) => {
  	console.log(e.name);
    console.log(e.message.split(','));
    console.log();
    aio.publishToFeed(message);
  });
});