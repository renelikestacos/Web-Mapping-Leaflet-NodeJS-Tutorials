// Initialize leaflet.js
var L = require('leaflet');
var sideBySide = require('leaflet-side-by-side')

// Initialize the map
var map = L.map('map').setView([19.436332, -99.158875], 11);

var esri_WorldImagery = L.tileLayer('http://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}', {
  attribution: 'Tiles &copy; Esri &mdash; Source: Esri, i-cubed, USDA, USGS, AEX, GeoEye, Getmapping, Aerogrid, IGN, IGP, UPR-EGP, and the GIS User Community'
}).addTo(map);

var esri_NatGeoWorldMap = L.tileLayer('http://server.arcgisonline.com/ArcGIS/rest/services/NatGeo_World_Map/MapServer/tile/{z}/{y}/{x}', {
  attribution: 'Tiles &copy; Esri &mdash; National Geographic, Esri, DeLorme, NAVTEQ, UNEP-WCMC, USGS, NASA, ESA, METI, NRCAN, GEBCO, NOAA, iPC',
  maxZoom: 16
}).addTo(map);


L.control.sideBySide(esri_WorldImagery ,esri_NatGeoWorldMap).addTo(map);

