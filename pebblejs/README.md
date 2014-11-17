Strap Metrics for Pebble.js
===
This is a sample Pebble.js project that includes Strap Metrics support. Below is a sample app.js. Better docs will be here soon.

1. Login to the <a href="http://www.straphq.com/login">Strap Dashboard</a> and create an app. You'll need your App ID handy for the next step.

2. Integrate Strap with your code as shown below.


```
/**
 * Welcome to Pebble.js!
 *
 * This is where you write your app.
 */

var UI = require('ui');
var Vector2 = require('vector2');
var SM = require('strap-metrics');


var strap_params = {
  // CHANGE ME!
    app_id: "abc123abc123xyz",
    resolution: "144x168",
    useragent: "PEBBLE/2.0"
};

// turn on accelerometer if desired
var Accel = require('ui/accel');
Accel.init();
strap_api_init_accel(Accel, strap_params);

// init the strap api
strap_api_init(strap_params);

// -------------------------

var main = new UI.Card({
  title: 'Pebble.js',
  icon: 'images/menu_icon.png',
  subtitle: 'Hello World!',
  body: 'Press any button.'
});

main.show();

params = strap_api_clone(strap_params);
params['action_url'] = '/main/show';
strap_api_log(params);

main.on('click', 'up', function(e) {
  var menu = new UI.Menu({
    sections: [{
      items: [{
        title: 'Pebble.js',
        icon: 'images/menu_icon.png',
        subtitle: 'Can do Menus'
      }, {
        title: 'Second Item',
        subtitle: 'Subtitle Text'
      }]
    }]
  });
  menu.on('select', function(e) {
    console.log('Selected item #' + e.itemIndex + ' of section #' + e.sectionIndex);
    console.log('The item is titled "' + e.item.title + '"');
    params['action_url'] = '/selected/'+e.item.title;
    strap_api_log(params);
  });
  menu.show();
});

main.on('click', 'select', function(e) {
  var wind = new UI.Window();
  var textfield = new UI.Text({
    position: new Vector2(0, 50),
    size: new Vector2(144, 30),
    font: 'gothic-24-bold',
    text: 'Text Anywhere!',
    textAlign: 'center'
  });
  wind.add(textfield);
  wind.show();
  params['action_url'] = '/main/select';
  strap_api_log(params);
});

main.on('click', 'down', function(e) {
  var card = new UI.Card();
  card.title('A Card');
  card.subtitle('Is a Window');
  card.body('The simplest window type in Pebble.js.');
  card.show();
  params['action_url'] = '/main/down';
  strap_api_log(params);
});

```
