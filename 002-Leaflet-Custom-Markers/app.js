// Initialize leaflet.js
var L = require('leaflet');

// Initialize the map
var map = L.map('map', {
  scrollWheelZoom: false
});

// Set the position and zoom level of the map
map.setView([47.70, 13.35], 7);

// Set base layers
var esri_NatGeoWorldMap = L.tileLayer('http://server.arcgisonline.com/ArcGIS/rest/services/NatGeo_World_Map/MapServer/tile/{z}/{y}/{x}', {
	attribution: 'Tiles &copy; Esri &mdash; National Geographic, Esri, DeLorme, NAVTEQ, UNEP-WCMC, USGS, NASA, ESA, METI, NRCAN, GEBCO, NOAA, iPC',
	maxZoom: 16
}).addTo(map);

var esri_WorldImagery = L.tileLayer('http://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}', {
	attribution: 'Tiles &copy; Esri &mdash; Source: Esri, i-cubed, USDA, USGS, AEX, GeoEye, Getmapping, Aerogrid, IGN, IGP, UPR-EGP, and the GIS User Community'
});

// Create base layers group object
var baseLayers = {
	"ESRI National Geographic": esri_NatGeoWorldMap,
	"ESRI World Imagery": esri_WorldImagery
};

var pinkIcon = L.icon({
    iconUrl: 'style/marker-pink.png',
    iconSize: [39, 39],
    iconAnchor: [18, 39],
    popupAnchor: [10, -35]
});
var greenIcon = L.icon({
    iconUrl: 'style/marker-green.png',
    iconSize: [39, 39],
    iconAnchor: [18, 39],
    popupAnchor: [10, -35]
});
var blueIcon = L.icon({
    iconUrl: 'style/marker-blue.png',
    iconSize: [39, 39],
    iconAnchor: [18, 39],
    popupAnchor: [10, -35]
});

var klagenfurt = L.marker([46.623997, 14.307812], {icon: pinkIcon}).bindPopup('<b>Klagenfurt, Kärnten</b>'),
graz = L.marker([47.070762, 15.438698], {icon: pinkIcon}).bindPopup('<b>Graz, Steiermark</b>'),
salzburg = L.marker([47.805109, 13.041151], {icon: pinkIcon}).bindPopup('<b>Salzburg, Salzburg</b>'),
eisenstadt = L.marker([47.845993, 16.527337], {icon: greenIcon}).bindPopup('<b>Eisenstadt, Burgenland</b>'),
wien = L.marker([48.208539, 16.372505], {icon: greenIcon}).bindPopup('<b>Wien, Wien</b>'),
stpoelten = L.marker([48.203828, 15.630877], {icon: greenIcon}).bindPopup('<b>St.Pölten, Niederösterreich</b>'),
linz = L.marker([48.307025, 14.284829], {icon: blueIcon}).bindPopup('<b>Linz, Oberösterreich</b>'),
innsbruck = L.marker([47.268896, 11.401791], {icon: blueIcon}).bindPopup('<b>Innsbruck, Tirol</b>'),
bregenz = L.marker([47.500929, 9.740660], {icon: blueIcon}).bindPopup('<b>Bregenz, Vorarlberg</b>');


var capitals = L.layerGroup([klagenfurt, 
	graz, eisenstadt, salzburg, wien, 
	stpoelten, linz, innsbruck, bregenz]).addTo(map);

var overlays = {
	'Capitals': capitals
};

// Add baseLayers and overlays to layer panel
L.control.layers(baseLayers, overlays).addTo(map);