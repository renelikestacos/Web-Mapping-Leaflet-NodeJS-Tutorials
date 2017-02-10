// Initialize leaflet.js
var L = require('leaflet');

// Initialize the map
var map = L.map('map', {
  scrollWheelZoom: false
});

// Set the position and zoom level of the map
map.setView([47.20, 13.35], 8);

/* Base Layers */
var esri_WorldImagery = L.tileLayer('http://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}', {
	attribution: 'Tiles &copy; Esri &mdash; Source: Esri, i-cubed, USDA, USGS, AEX, GeoEye, Getmapping, Aerogrid, IGN, IGP, UPR-EGP, and the GIS User Community'
}).addTo(map);

var spittal = L.tileLayer.wms("http://webportal.conabio.gob.mx:8085/geoserver/wms?",
	{layers:"leafletTutorial:at_districts", format:'image/png', transparent:true, CQL_FILTER: 'iso=206'}).addTo(map)

var salzburg_umgebung = L.tileLayer.wms("http://webportal.conabio.gob.mx:8085/geoserver/wms?",
	{layers:"leafletTutorial:at_districts", format:'image/png', transparent:true, CQL_FILTER: "IN('at_districts.70')"}).addTo(map)

var villach = L.tileLayer.wms("http://webportal.conabio.gob.mx:8085/geoserver/wms?",
	{layers:"leafletTutorial:at_districts", format:'image/png', transparent:true, CQL_FILTER: "name IN('Villach Stadt')"}).addTo(map)


// Create overlay object
var overlay = {
	"Spittal an der Drau": spittal,
	"Salzburg Umgebung": salzburg_umgebung,
	"Villach Stadt": villach,
};

// Add control to the map
L.control.layers(null, overlay).addTo(map)