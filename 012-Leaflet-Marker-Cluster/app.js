
// Initialize leaflet.js
var L = require('leaflet');
var MC = require('leaflet.markercluster')

// Initialize the map
var map = L.map('map').setView([47.70, 13.35], 7);

var esri_WorldImagery = L.tileLayer('http://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}', {
  attribution: 'Tiles &copy; Esri &mdash; Source: Esri, i-cubed, USDA, USGS, AEX, GeoEye, Getmapping, Aerogrid, IGN, IGP, UPR-EGP, and the GIS User Community'
}).addTo(map);

var esri_NatGeoWorldMap = L.tileLayer('http://server.arcgisonline.com/ArcGIS/rest/services/NatGeo_World_Map/MapServer/tile/{z}/{y}/{x}', {
  attribution: 'Tiles &copy; Esri &mdash; National Geographic, Esri, DeLorme, NAVTEQ, UNEP-WCMC, USGS, NASA, ESA, METI, NRCAN, GEBCO, NOAA, iPC',
  maxZoom: 16
});


$.getJSON("data/austria.geojson",function(states){
  L.geoJson(states).addTo(map);
});

$.getJSON("data/points_districts.geojson",function(data){
  var greenIcon = L.icon({
    iconUrl: 'style/images/leaf-green.png',
    shadowUrl: 'style/images/leaf-shadow.png',
    iconSize:     [38, 95],
    shadowSize:   [50, 64],
    iconAnchor:   [22, 94],
    shadowAnchor: [4, 62], 
    popupAnchor:  [-3, -76]
  });
  var districts = L.geoJson(data,{
    pointToLayer: function(feature,latlng){
      var marker = L.marker(latlng,{icon: greenIcon});
      marker.bindPopup('<b>'+feature.properties.name+'</b>');
      return marker;
    }
  });
  var clusters = L.markerClusterGroup();
  clusters.addLayer(districts);
  map.addLayer(clusters);
});



var baseLayers = {
  "ESRI World Imagery": esri_WorldImagery,
  "ESRI National Geographic": esri_NatGeoWorldMap
};

L.control.layers(baseLayers, null).addTo(map);



