function append_incoming_dht_data_row(data) {
  let text = `<tr><td>${data['id']}</td><td> ${data['currentdate']} @ ${data['currenttime']} </td><td>${data['temperature']} | ${data['humidity']} </td><td> ${data['device']}</td></tr>`;
  $('#incomingdatatable tr').eq(1).after(text);
}

function handle_resource_data(resource, data) {
  switch (resource) {
    case '/most_recent':
      data = data[0];
      append_incoming_dht_data_row(data);
      break;
    default:
      break;
  }
}

function get_and_handle_resource_jquery(resource) {
  let query_url = `/api/v1/resources/dhtdata${resource}`;
  $.ajax({
    type: "GET",
    url: query_url,
    success: function(data) {
      handle_resource_data(resource, data);
    }
  });
}

$(document).ready(function() {
  let socket = io.connect('http://' + document.domain + ':' + location.port);
  socket.on('dht_data_message', function(mqtt_message) {
    mqtt_payload = mqtt_message['payload'];
    get_and_handle_resource_jquery(mqtt_payload);
  });
});

/*Auto-updating functionality, currently will append previously appended data*/
/*$(function loadDatabase(){
  window.setInterval(function(){
    get_and_handle_resource_jquery('/most_recent');
  }, 5000);
});*/