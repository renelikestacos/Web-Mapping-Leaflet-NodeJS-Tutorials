// Initialize leaflet.js
var L = require('leaflet');

/*	Opacity Slider 	*/
L.Control.opacitySlider = L.Control.extend({
    options: {
        position: 'bottomright'
    },
    setOpacityLayer: function (layer) {
            opacity_layer = layer;
    },
    onAdd: function (map) {
        var opacity_slider_div = L.DomUtil.create('div', 'opacity_slider_control');
        $(opacity_slider_div).slider({
          orientation: "vertical",
          range: "min",
          min: 0,
          max: 100,
          value: 100,
          step: 5,
          start: function ( event, ui) {
            map.dragging.disable();
            map.once('mousedown', function (e) { 
              map.dragging.enable();
            });
          },
          slide: function ( event, ui ) {
            var slider_value = ui.value / 100;
            opacity_layer.setOpacity(slider_value);
          }
        });
        return opacity_slider_div;
    }
});

// Set map
var map = L.map('map', {
  scrollWheelZoom: false
});

// Set the position and zoom level of the map
map.setView([24, -103], 5);

/* Base Layers */
var esri_WorldImagery = L.tileLayer('http://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}', {
	attribution: 'Tiles &copy; Esri &mdash; Source: Esri, i-cubed, USDA, USGS, AEX, GeoEye, Getmapping, Aerogrid, IGN, IGP, UPR-EGP, and the GIS User Community'
}).addTo(map);

/* WMS Layer */
var wms_layer = L.tileLayer.wms("http://webportal.conabio.gob.mx:8085/geoserver/wms?",
	{layers:"MEX_EcosystemIntegrity:EcosystemIntegrity_2014", format:'image/png',transparent:true}).addTo(map)

// Set opacity slider
var opacitySlider = new L.Control.opacitySlider();
map.addControl(opacitySlider);

// Add layer to opacity slider
opacitySlider.setOpacityLayer(wms_layer);