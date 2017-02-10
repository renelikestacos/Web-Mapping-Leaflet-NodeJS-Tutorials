// Initialize leaflet.js
var L = require('leaflet');
var S = require('leaflet.sync')
var map_center = [47.807993, 13.056945];
//	Base layers
var esri_WorldImagery = L.tileLayer('http://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}', {
	attribution: 'Tiles &copy; Esri &mdash; Source: Esri, i-cubed, USDA, USGS, AEX, GeoEye, Getmapping, Aerogrid, IGN, IGP, UPR-EGP, and the GIS User Community'
});

var esri_NatGeoWorldMap = L.tileLayer('http://server.arcgisonline.com/ArcGIS/rest/services/NatGeo_World_Map/MapServer/tile/{z}/{y}/{x}', {
	attribution: 'Tiles &copy; Esri &mdash; National Geographic, Esri, DeLorme, NAVTEQ, UNEP-WCMC, USGS, NASA, ESA, METI, NRCAN, GEBCO, NOAA, iPC',
	maxZoom: 16
});

// Initialize the both maps
var map_1 = L.map('map_1', {
	layers:[esri_WorldImagery],
	scrollWheelZoom: false,
	center: map_center,
	zoom: 10
});

map_1.attributionControl.setPrefix('');

var map_2 = L.map('map_2', {
	layers:[esri_NatGeoWorldMap],
	scrollWheelZoom: false,
	center: map_center,
	zoom: 10
});

// Sync them
map_1.sync(map_2);
map_2.sync(map_1);