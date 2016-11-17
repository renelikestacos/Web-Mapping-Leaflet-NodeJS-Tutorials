// Initialize leaflet.js
var L = require('leaflet');

//Set Root URL for your Geoserver
var rootUrl = 'http://demo.opengeo.org/geoserver/ows';

//Set Default Parameter for the Layer you want
var defaultParameters = {
    service: 'WFS',
    version: '1.0.0',
    request: 'GetFeature',
    typeName: 'og:archsites',
    maxFeatures: 200,
    outputFormat: 'application/json',
    srsName:'EPSG:4326'
};
var parameters = L.Util.extend(defaultParameters);

function readBody(xhr) {
    var data;
    if (!xhr.responseType || xhr.responseType === "text") {
        data = xhr.responseText;
    } else if (xhr.responseType === "document") {
        data = xhr.responseXML;
    } else {
        data = xhr.response;
    }
    return data;
}

var xhr = new XMLHttpRequest();
xhr.onreadystatechange = function() {
    if (xhr.readyState == 4) {
        document.getElementById("json_data").innerHTML = (readBody(xhr));
    }
}
//Set up the final URL for WFS Request
var wfs_url = rootUrl + L.Util.getParamString(parameters);

//Fire WFS request
xhr.open('GET',wfs_url, true);
xhr.send(null);

