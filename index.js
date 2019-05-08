var hidspyBinding = require("bindings")("hidspy.node");
var events = require("events");

require("es6-promise/auto");

var index = require("./package.json");

if (global[index.name] && global[index.name].version === index.version) {
  module.exports = global[index.name];
} else {
  var hidspy = new events.EventEmitter();
  var ready = false;

  hidspy.spyOn = function(callback) {
    var promise = new Promise(function(resolve, reject) {
      if (!ready) {
        hidspyBinding.spyOn(
          function(data) {
            hidspy.emit("change", data);
          },
          function(data) {
            hidspy.emit("end", data);
          },
          function() {
            resolve(true);
            callback(true);
          }
        );
        ready = true;
      } else {
        reject();
        callback(false);
      }
    });

    return promise;
  };

  hidspy.spyOff = function() {
    if (!ready) {
      hidspyBinding.spyOff();
      ready = false;
    }
  };

  hidspy.status = {
    plugin: 1,
    pullout: 0,
  }

  hidspy.version = index.version;
  global[index.name] = hidspy;

  module.exports = hidspy;
}
