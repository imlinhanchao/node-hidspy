var hidspy = require("../index");

hidspy.spyOn().then(function() {
  hidspy.on("change", function(data) {
    console.log(data);
  });

  hidspy.on("end", function() {
    console.log('Stop to listen');
  });
});

setTimeout(() => {
  hidspy.spyOff();
}, 5000);