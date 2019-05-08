##### Note: Designed for Windows

# node-hidspy
An event based node.js c++ addon/binding to detect the hid device insertion/removal and notify the subscribed apps.

## Prerequisties for installation
Before installing this package, make sure you have Node.js addon build tool [node-gyp](https://github.com/nodejs/node-gyp) installed in your machine.

As `node-gyp` uses `Python 2.x`, make sure it is installed in your machine and it is on your EVN path.

If you dont have the python installed, then it is recommanded to install the `windows-build-tools`. To installl windows build tool, open CLI as adminstrator and execute the below command as per the issue discussed [here](https://github.com/felixrieseberg/windows-build-tools/issues/56).

```
npm --add-python-to-path='true' --debug install --global windows-build-tools
```

The above command would install pythin 2.x and build tools needed for windows. It would take few mins to complete the installation.


## Installation
To install `node-hidspy` execute the below command

if you prefer `npm`
```
npm install node-hidspy
```

for `yarn`,

```
yarn add node-hidspy
```
Upon installation, `node-gyp` would start generating the c++ addon project as per the configuration we set in `binding.gyp` and compile the same. If the compilation is successfull, it would have generated the executable lib files in the `build`folder with `Release` configuration.

## How to use it

`require('node-hidspy')` would populate the hidspy object which is of event type.

### Activating the detection(spying on the usb controller)

```
var hidspy = require('node-hidspy');

hidspy.spyOn().then(() => {
    ...
})
```

### Deactivating the detection

```
hidspy.spyOff() // would stop listening for the usb detection.
```

### Listening for change
There is an event `change` which would be emitted when a usb device is inserted or removed. You can subscribe for the event and do necessory action upon the event.

```
hidspy.on('change', (device) => {
    console.log(device);
    /* { 
        name: '\\\\?\\HID#VID_1044&PID_7A13&MI_03#8&2346c4b6&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}',
        vid: 4164,
        pid: 31251,
        hid: 0,
        guid: '4d1e55b2-f16f-11cf-001111000030',
        status: 1 
    } */
});

hidspy.on('end', () => {
    // would be triggered when you call `spyOff` function.
})
```

## API

### Properties
When an usb device is inserted, an `Device` object would be generated and emitted.

`Device` object has,

* `device_status` - It is an integer type property. The possible values for this property is 0 or 1 when 0 indicates the device is been removed from the system and the 1 indicates the device is been added.
* `guid` - It is an string type property. The unique numeric assigned to the usb storage device by the windows.
* `vid` - It is an integer type property. The vendor id is assigned by the USB Implementers Forum to a specific company.
* `pid` - It is an integer type property. The product id is assigned by the company for the individual product.
* `interface` - It is an integer type property. It is the interface index of the hid device.
* `name` - It is an string type property. It is the device name in device manager of windows.


### Methods
There are four methods available in `hidspy`.

#### spyOn([callback])
`spyOn` method takes a callback as parameter and returns a promise object. Here `callback` is optional. Since spyOn returns promise, you can use `then` to kick start the detection. 

When the addon is ready, `callback` would be called with `true` which indicates everything is OK if passed otherwise the promise would be resolve/rejected.

#### spyOff()
`spyOff` should be called when you wanted to stop listening for the usb device change.

### Events
There are two events emitted from the `hidspy` module. 

#### change - hidspy.on('change' callback(device))
When any usb storage device is been inserted/removed into/from the machine, `change` event would be triggered with the `Device` object.

#### end - hidspy.on('end', callback)
When the `spyOff` method is called, the `end` event would be triggered.

## Example

You can have a look into `example/test.js` for usage and example.

```
var hidspy = require('../index');

hidspy.spyOn().then(function() {

    hidspy.on('change', function(data) {
        console.log(data);
    });
    
    hidspy.on("end", function() {
        console.log('Stop to listen');
    });
});

setTimeout(() => {
    hidspy.spyOff();
}, 5000); // after 5 secs, would stop wathcing for device change.
```

##### Note:
When you DEBUG the c++ code, you have to comment the line#16 in the `usbspy.h`
