Strap Metrics for Pebble.js
===
This is a sample Pebble.js project that includes Strap Metrics support. Below is a sample app.js. Better docs will be here soon.


```
/**
 * Welcome to Pebble.js!
 *
 * This is where you write your app.
 */

var UI = require('ui');
var Vector2 = require('vector2');

// ------------------------------
// Start of Strap API
// ------------------------------
var strap_api_url="https://api.straphq.com/create/visit/with/";var strap_api_clone=function(obj){if(null==obj||"object"!=typeof obj)return obj;var copy={};for(var attr in obj){if(obj.hasOwnProperty(attr))copy[attr]=obj[attr]}return copy};function strap_api_init_accel(accel,params){accel.config({rate:10,samples:10});var p=strap_api_clone(params);p["action_url"]="STRAP_API_ACCL";strap_api_accl_on(accel,p)}function strap_api_accl_on(accel,params){console.log("turning accl on");strap_api_log(params);accel.on("data",function(e){var now=(new Date).getTime();var base=0;if(e.accels.length>0){base=e.accels[0].time}for(var i=0;i<e.accels.length;i++){e.accels[i].time=now+e.accels[i].time-base}var tmpstore=window.localStorage["strap_accl"];if(tmpstore){tmpstore=JSON.parse(tmpstore)}else{tmpstore=[]}tmpstore=tmpstore.concat(e.accels);window.localStorage["strap_accl"]=JSON.stringify(tmpstore)});setTimeout(function(){strap_api_log(params);strap_api_accl_off(accel,params)},1e3*60*1)}function strap_api_accl_off(accel,params){console.log("turning accl off");strap_api_log(params);accel.off();setTimeout(function(){strap_api_log(params);strap_api_accl_on(accel,params)},1e3*60*2)}function strap_api_init(params){var lp=strap_api_clone(params);lp["action_url"]="STRAP_START";strap_api_log(lp)}function strap_api_conv_accel(data,act){var da=[];for(var i=0;i<data.length;i++){var d={};d.x=data[i].x;d.y=data[i].y;d.z=data[i].z;d.ts=data[i].time;d.vib=data[i].vibe?true:false;d.act=act;da.push(d)}return da}function strap_api_log(params){var lp=params;var req=new XMLHttpRequest;req.open("POST",strap_api_url,true);var tz_offset=(new Date).getTimezoneOffset()/60*-1;var query="app_id="+lp["app_id"]+"&resolution="+(lp["resolution"]||"")+"&useragent="+(lp["useragent"]||"")+"&action_url="+(lp["action_url"]||"")+"&visitor_id="+(lp["visitor_id"]||Pebble.getAccountToken())+"&act="+(lp["act"]||"")+"&visitor_timeoffset="+tz_offset;if(lp["action_url"]==="STRAP_API_ACCL"){var tmpstore=window.localStorage["strap_accl"];if(tmpstore){tmpstore=JSON.parse(tmpstore)}else{tmpstore=[]}if(tmpstore.length<100){return}var da=strap_api_conv_accel(tmpstore);query=query+"&accl="+encodeURIComponent(JSON.stringify(da));window.localStorage.removeItem("strap_accl")}else{var p=strap_api_clone(params);p["action_url"]="STRAP_API_ACCL";setTimeout(function(){strap_api_log(p)},100)}req.setRequestHeader("Content-type","application/x-www-form-urlencoded");req.setRequestHeader("Content-length",query.length);req.setRequestHeader("Connection","close");req.onload=function(e){if(req.readyState==4&&req.status==200){if(req.status==200){}else{}}};req.send(query)}
// End of Strap API
// ------------------------------


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