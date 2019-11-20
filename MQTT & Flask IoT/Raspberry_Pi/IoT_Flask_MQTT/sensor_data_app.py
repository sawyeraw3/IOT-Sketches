#!/usr/bin/env python3
import eventlet
from flask import Flask, render_template, request, jsonify
from flask_mqtt import Mqtt
from flask_socketio import SocketIO
from flask_bootstrap import Bootstrap
import json
import sqlite3

eventlet.monkey_patch()

app = Flask(__name__)
app.config['TEMPLATES_AUTO_RELOAD'] = True
app.config['MQTT_BROKER_URL'] = 'MQTT_BROKER_URL'
app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_USERNAME'] = 'MQTT_USERNAME'
app.config['MQTT_PASSWORD'] = 'MQTT_PASSWORD'
app.config['MQTT_KEEPALIVE'] = 5
app.config['MQTT_TLS_ENABLED'] = False

mqtt = Mqtt(app)
socketio = SocketIO(app)
boostrap = Bootstrap(app)

@mqtt.on_connect()
def on_connect(client, userdata, flags, rc):
  print("Connected with result code " + str(rc))
  mqtt.subscribe("/esp8266/dhtdata")

def send_update_notification_to_client(message):
  data = dict(
    topic = message.topic,
    payload = "/most_recent"
  )
  socketio.emit('dht_data_message', data = data)
  return

@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
  with app.app_context():
    if message.topic == "/esp8266/dhtdata":
      dhtdata_json = json.loads(message.payload.decode())
      conn=sqlite3.connect('espsensordata.db')
      c=conn.cursor()
      #TODO could include timesent from client
      c.execute("""INSERT INTO dhtdata (temperature,
        humidity, currentdate, currenttime, device) VALUES((?), (?), date('now', 'localtime'),
        time('now', 'localtime'), (?))""", (dhtdata_json["temperature"],
        dhtdata_json["humidity"], dhtdata_json["device"]) )
      conn.commit()
      conn.close()
      send_update_notification_to_client(message)

@mqtt.on_log()
def handle_logging(client, userdata, level, buf):
  print(level, buf)
  return

def dict_factory(cursor, row):
  d = {}
  for idx, col in enumerate(cursor.description):
    d[col[0]] = row[idx]
  return d

def get_n_rows_from_db(db, table, nrows=0):
  conn=sqlite3.connect(db)
  conn.row_factory = dict_factory
  c=conn.cursor()
  cmd = "SELECT * FROM {} ORDER BY id".format(table)
  if (nrows > 0):
    cmd += " DESC LIMIT {}".format(nrows)
  c.execute(cmd)
  row_data = c.fetchall()
  return row_data

@app.route("/api/v1/resources/dhtdata/all", methods=['GET'])
def home():
  dht_readings = get_n_rows_from_db('espsensordata.db', 'dhtdata')
  return jsonify(dht_readings)

@app.route("/api/v1/resources/dhtdata/most_recent", methods=['GET'])
def api_most_recent_reading():
  dht_reading = get_n_rows_from_db('espsensordata.db', 'dhtdata', 1)
  return jsonify(dht_reading)

@app.route("/")
def api_20_readings():
  dht_readings = get_n_rows_from_db('espsensordata.db', 'dhtdata', 20)
  return render_template('index.html', dht_readings=dht_readings)

@app.errorhandler(404)
def page_not_found(e):
  return "<h1>404</h1><p>The resource could not be found.</p>", 404

@app.route('/api/v1/resources/dhtdata', methods=['GET'])
def api_filter():
    query_parameters = request.args
    device = query_parameters.get('device')
    date = query_parameters.get('date')
    query = "SELECT * FROM dhtdata WHERE"
    to_filter = []
    if device:
        query += ' device=? AND'
        to_filter.append(device)
    if date:
        query += ' currentdate=? AND'
        to_filter.append(date)
    if not (device or date):
        return page_not_found(404)
    query = query[:-len(' AND')] + ';'
    conn = sqlite3.connect('espsensordata.db')
    conn.row_factory = dict_factory
    cur = conn.cursor()
    results = cur.execute(query, to_filter).fetchall()
    return jsonify(results)

if __name__ == "__main__":
  socketio.run(app, host='0.0.0.0', port=8181, debug=False)