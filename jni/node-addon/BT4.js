var BT = require('/data/phonetest/BT.node');

function Enum() {
    this._enums = [];
    this._lookups = {};
}

Enum.prototype.getEnums = function() {
    return _enums;
}

Enum.prototype.forEach = function(callback){
    var length = this._enums.length;
    for (var i = 0; i < length; ++i){
        callback(this._enums[i]);
    }
}

Enum.prototype.addEnum = function(e) {
    this._enums.push(e);
}

Enum.prototype.getByName = function(name) {
    return this[name];
}

Enum.prototype.getByValue = function(field, value) {
    var lookup = this._lookups[field];
    if(lookup) {
        return lookup[value];
    } else {
        this._lookups[field] = ( lookup = {});
        var k = this._enums.length - 1;
        for(; k >= 0; --k) {
            var m = this._enums[k];
            var j = m[field];
            lookup[j] = m;
            if(j == value) {
                return m;
            }
        }
    }
    return null;
}

function defineEnum(definition) {
    var k;
    var e = new Enum();
    for(k in definition) {
        var j = definition[k];
        e[k] = j;
        e.addEnum(j)
    }
    return e;
}

var EVENTS = defineEnum({
    CONNECT_BACK : {
        value : 1,
        string : 'CONNECT_BACK'
    },
    DISCONNECT_BACK : {
        value : 3,
        string : 'DISCONNECT_BACK'
    },
    HANDLE_NOTIFY_BACK : {
        value : 4,
        string : 'HANDLE_NOTIFY_BACK'
    },
    HANDLE_INDICATOR_BACK : {
        value : 5,
        string : 'HANDLE_INDICATOR_BACK'
    },
    CHAR_READ_HND_BACK : {
        value : 10,
        string : 'CHAR_READ_HND_BACK'
    },
    CHAR_READ_DESC_BACK : {
        value : 12,
        string : 'CHAR_READ_DESC_BACK'
    }
});

function scan_cb(arg1)
{
    console.log("scan_cb:"+arg1);
}
function device_cb(e)
{
    //for (var i = 0; i < arguments.length; i++) {
    //console.log("device_cb arg["+i+"] = "+arguments[i]);
    //}
    console.log("DEVICE JS event = "+e.event);
    switch(e.event){
    case EVENTS.CONNECT_BACK.value:
	console.log("DEVICE JS event name = "+EVENTS.CONNECT_BACK.string);
	if (e.retval !=0){
	    console.log("    bad retval = "+e.retval);
	}
	D.CharWriteCommand(0x0039,"20");
	break;
    case EVENTS.DISCONNECT_BACK.value:
	console.log("DEVICE JS event name = "+EVENTS.DISCONNECT_BACK.string);	
	break;
    case EVENTS.CHAR_READ_HND_BACK.value:
	console.log("DEVICE JS event name = "+EVENTS.CHAR_READ_HND_BACK.string);
	console.log("\thandle = 0x"+e.handle.toString(16));
	var buffer = new Buffer(e.handle_data.length+1)
	for (var i = 0; i < e.handle_data.length; i++) {
	    console.log("\tdata["+i+"] = 0x"+e.handle_data[i].toString(16));
	    buffer[i] = e.handle_data[i]
	}
	if ((e.handle_data.length > 4) && (e.handle_data.length < 20)){
	    console.log("string interp = "+buffer.toString("utf-8"));
	}
	else{
	    console.log("string length = "+e.handle_data.length);
	}
	
	break;
    case EVENTS.CHAR_READ_DESC_BACK.value:
	console.log("DEVICE JS event name = "+EVENTS.CHAR_READ_DESC_BACK.string);
	console.log("\thandle = 0x"+e.handle.toString(16)+"uuid = 0x",e.uuid.toString(16));
	break;
    case EVENTS.HANDLE_NOTIFY_BACK.value:
	console.log("DEVICE JS event name = "+EVENTS.HANDLE_NOTIFY_BACK.string);
	console.log("\thandle = 0x"+e.handle.toString(16));
	var buffer = new Buffer(e.handle_data.length+1)
	for (var i = 0; i < e.handle_data.length; i++) {
	    console.log("\tdata["+i+"] = 0x"+e.handle_data[i].toString(16));
	    buffer[i] = e.handle_data[i]
	}
	
	break;
    case EVENTS.HANDLE_INDICATOR_BACK.value:
	console.log("DEVICE JS event name = "+EVENTS.HANDLE_INDICATOR_BACK.string);
	console.log("\thandle = 0x"+e.handle.toString(16));
	var buffer = new Buffer(e.handle_data.length+1)
	for (var i = 0; i < e.handle_data.length; i++) {
	    console.log("\tdata["+i+"] = 0x"+e.handle_data[i].toString(16));
	    buffer[i] = e.handle_data[i]
	}
	break;

    default:
	console.log("Unknown Event:"+e.event);
	break;
    }
}


GLOBAL.S = new BT.Scanner(scan_cb);
GLOBAL.D = new BT.Device("BC:6A:29:AB:B2:8D",device_cb);

