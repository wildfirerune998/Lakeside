// // Import the Clay package
// var Clay = require('pebble-clay');
// // Load our Clay configuration file
// var clayConfig = require('./config.json');
// // Initialize Clay
// var clay = new Clay(clayConfig);

var weatherCode = 100;
var isDay = 5;

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
    // if (api) {
        var url = "http://api.open-meteo.com/v1/forecast?latitude=" + 
            pos.coords.latitude + "&longitude=" + pos.coords.longitude + "&current=weather_code,is_day";

      // Send request to OpenWeatherMap
      xhrRequest(url, 'GET', function(responseText) {
        // responseText contains a JSON object with weather info
        var json = JSON.parse(responseText);

        console.log(JSON.stringify(json));
        if (json){

          weatherCode = 100;
          isDay = 5;

          // Conditions
          var lcl_weatherCode = json.current.weather_code;      
          weatherCode = lcl_weatherCode;
        
          // isDay
          var lcl_isDay = json.current.is_day;      
          if (lcl_isDay == 0 || lcl_isDay == 1){
            isDay = lcl_isDay;
          }
        
          // Assemble dictionary using our keys
          var dictionary = {
            "WEATHERCODE": weatherCode,
            "ISDAY": isDay
          };
          
            console.log(weatherCode); 
            console.log(isDay); 
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
  // }; //weather check

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
    
  if (!e.payload){
    console.log('no payload');
    return;
  }
  
  var weatherCode_string;
  weatherCode_string = JSON.stringify(e.payload.WEATHERCODE);
  if (weatherCode_string) {
    weatherCode = weatherCode_string.replace(/"/g,"");
  }

  var isDay_string;
  isDay_string = JSON.stringify(e.payload.isDay);
  if (isDay_string) {
    isDay = isDay_string.replace(/"/g,"");
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

  weatherCode = claySettings[messageKeys.WEATHERCODE];
  isDay = claySettings[messageKeys.ISDAY];

  getWeather(); 

}
);