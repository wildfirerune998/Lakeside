// Import the Clay package
var Clay = require('pebble-clay');
// Load our Clay configuration file
var clayConfig = require('./config.json');
// Initialize Clay
var clay = new Clay(clayConfig);

var api = "";
var conditions;
var sunset;
var sunrise;

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
//   var conditions;
//   var sunset;
//   var sunrise;
  // We will request the weather here
  // Construct URL

  // Don't bother with doing ANYTHING if they don't actually want the weather
    // Don't bother with anything unless you have an API key
    if (api) {
        var url = "http://api.openweathermap.org/data/2.5/weather?lat=" + 
            pos.coords.latitude + "&lon=" + pos.coords.longitude + "&appid=" + api + "&units=METRIC";
      // Send request to OpenWeatherMap
      xhrRequest(url, 'GET', function(responseText) {
        // responseText contains a JSON object with weather info
        var json = JSON.parse(responseText);

        if (json.cod != 200){
          //We have an error, so nothing
        } else {

          // Conditions
          conditions = json.weather[0].main;      
        
          // sunset
          sunset = json.sys.sunset;      
        
          // sunrise
          sunrise = json.sys.sunrise;  
   
        
          // Assemble dictionary using our keys
          var dictionary = {
            "CONDITIONS": conditions,
            "SUNRISE": sunrise,
            "SUNSET": sunset,
            "API": api
          };
          
            // console.log(conditions); 
            console.log(sunrise); 
            console.log(sunset); 
            console.log(conditions); 
            // console.log( pos.coords.latitude); 
            // console.log( pos.coords.longitude); 
          // Send to Pebble
          Pebble.sendAppMessage(dictionary, function(e) {
            console.log('Weather info sent to Pebble successfully!');
            },
            function(e) {
              console.log('Error sending weather info to Pebble!');
            }
          );//sendAppMessge

        } // end json status check

      });//xhr request
      //console.log('END xhr request'); 
    }; // API check
    //console.log('END API check'); 
  }; //weather check

function locationError(err) {
  console.log('Error requesting location!');
}
function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', function(e) {

    //console.log('PebbleKit JS ready!');

    var dictionary = {
      "READY": 1
    };

    // Send to Pebble
    Pebble.sendAppMessage(dictionary,
      function(e) {
        console.log('READY');
      },
      function(e) {
        console.log('OH NO');
      }
     
    );  }
);

var messageKeys = require('message_keys');
// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage', function(e) {
    
    console.log('appmessage');
  if (!e.payload){
    console.log('no payload');
    return;
  }

  var api_string;
  api_string = JSON.stringify(e.payload.API);
  if (api_string) {
    api = api_string.replace(/"/g,"");
  }
  
  var conditions_string;
  conditions_string = JSON.stringify(e.payload.CONDITIONS);
  if (conditions_string) {
    conditions = conditions_string.replace(/"/g,"");
  }

  var sunrise_string;
  sunrise_string = JSON.stringify(e.payload.SUNRISE);
  if (sunrise_string) {
    sunrise = sunrise_string.replace(/"/g,"");
  }

  var sunset_string;
  sunset_string = JSON.stringify(e.payload.SUNSET);
  if (sunset_string) {
    sunset = sunset_string.replace(/"/g,"");
  }
  
  getWeather();

  }                     
);

Pebble.addEventListener('webviewclosed', function(e) {
  
  //console.log('e.response ' + e.response);
  if (e && !e.response) {
    return;
  };
  
  // Get the keys and values from each config item
  var claySettings = clay.getSettings(e.response);

  api = claySettings[messageKeys.API];
  conditions = claySettings[messageKeys.CONDITIONS];
  sunrise = claySettings[messageKeys.SUNRISE];
  sunset = claySettings[messageKeys.SUNSET];

  getWeather(); 

}
);