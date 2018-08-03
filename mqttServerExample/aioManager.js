const mqtt = require('mqtt')
const username = 'username'
const key = 'key'
// Using Adafruit IO
const url = `mqtts://${username}:${key}@io.adafruit.com:8883`
const aioClient = mqtt.connect(url);
const exampleFeed = `${username}/feeds/examplefeed`

aioClient.on('connect', () => {
  aioClient.subscribe(exampleFeed);
  aioClient.publish(exampleFeed, 'Client Connected!');
});

aioClient.on('message', (topic, message) => {
  // convert message from Buffer to string
  console.log(`${exampleFeed} : ${message.toString()}`);
  console.log();
  //aioClient.end();
});

export function publishToFeed(message) {
	aioClient.publish(`${exampleFeed}`, message);
};