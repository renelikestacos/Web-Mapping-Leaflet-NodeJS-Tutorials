# Leaflet-Nodejs-Tutorial

This is a series of tutorials how to use [Leaflet](http://leafletjs.com/) with [Nodejs](https://nodejs.org/en/). To install one of the tutorials just copy them into any folder and execute `npm start` within the folder in your console.
I am using [Browserify](http://browserify.org/) and [Beefy](http://didact.us/beefy/) to start the tutorials on the localhost. If you do so as well, you will find it on <http://localhost:9966/> in your machine.<br><br><br>

## From the scratch
In the following steps, I will explain you how to set up a Leaflet-Nodejs application from the scratch.

#### 1. Install dependencies
Make sure you have already installed Nodejs and NPM. If you haven't, here is a [link](https://nodejs.org/en/download/) to download and install it. As already mentioned above, you also need to install [Browserify](http://browserify.org/) and [Beefy](http://didact.us/beefy/).<br>

#### 2. Create directory & files
Create and navigate to a new project folder and create some files with 
```
mkdir myapp
cd myapp
mkdir style
touch index.html app.js style/style.css
```
These files are forming the base of our application. Then, we run `npm init` to create the package.json file. In the myapp folder you should find now a package.json file. Next step is to install leaflet with `npm install --save leaflet`. Using `--save` means basically it will be automatically saved as a dependency in the `package.json` file.<br>

#### 3. Write some code!
We already created files like `index.html`, `app.js` and `style.css`. Open file with `vi index.html` and paste the following code into it.
```html
<!doctype html>
<html>
<head>
<title>My first Leaflet example</title>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<link rel="stylesheet" href="node_modules/leaflet/dist/leaflet.css">
<link rel="stylesheet" href="style/style.css">
</head>
<body>
<div id="map"></div>
<script src="bundle.js"></script>
</body>
</html>
```
Now, lets do the same thing with the `app.js` file. Open the file with `vi app.js` and paste the following code into it.
```javascript
// Initialize leaflet.js
var L = require('leaflet');

// Initialize the map
var map = L.map('map', {
  scrollWheelZoom: false
});

// Set the position and zoom level of the map
map.setView([47.70, 13.35], 7);

// Initialize the base layer
var osm_mapnik = L.tileLayer('http://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
	maxZoom: 19,
	attribution: '&copy; OSM Mapnik <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>'
}).addTo(map);
```
The exact same thing with the `style.css` file. Open the file with `vi style/style.css` and paste the code into it.
```css
#map
{
    width: auto;
    height: 500px;
}
```
#### 4. Create the local development server
Eventually, you ask yourself why did you point to `bundle.js` in the `index.html`? It does not exist yet and actually we also won't create it at all. [Browserify](http://browserify.org/) and [Beefy](http://didact.us/beefy/) will serve the website on your computer. Those two modules will generate the `bundle.js` on the fly for us. So lets install them with `npm install --save-dev beefy browserify`.
Now lets change script section in our `package.json` to 
```json
"scripts": {
    "start": "beefy app.js:bundle.js --live",
    "bundle": "browserify app.js -o bundle.js"
  },
```
The part `browserify app.js -o bundle.js` will generate the `bundle.js` file.

#### 5. Run it!
Just do `npm start` in the `myapp` folder and that's it! You should be able to see your app running on [localhost:9966](http://localhost:9966). Your example should look like in the illustration below. Have fun!
![alt text](https://github.com/g07kore/leaflet-nodejs-tutorials/blob/master/001-Leaflet-BaseLayers/style/example.png "")
<br><br><br>
## Leaflet Nodejs Tutorials

#### 001-Leaflet-Base-Layers:
This first tutorial shows a few possible base layers you can use with Leaflet.

#### 002-Leaflet-Custom-Markers:
Shows how to integrate custom style markers. Markers are placed on the nine capitals of the Austrian states. By clicking on them, a popup appears and gives you information about the location.

#### 003-Leaflet-Interactive-Geojson:
This tutorial shows how to integrate an interactive Geojson layer. The example data is a geojson of the Austrian states. If you hover over a state, the name, population density (people/km2) appears in a small info control on the right bottom. Furthermore, the population density classified into groups including a legend panel.

Optional: In case you are familiar with MongoDB you can also find the Geojson in my MongoDB on mLAB. It can be find here `mongodb://guest:guest@ds159328.mlab.com:59328/mongo_project40`, the collection is called `austria`.

#### 004-Leaflet-WMS-GetFeatureInfo:
Demonstrates a custom-built function to retrieve information (attribute values) from a WMS layer for a pixel location on the map. OpenLayers has this function, Leaflet doesn't. For now it only works if one WMS layer is active.

#### 005-Leaflet-Custom-LayerGroup-Panel:
This tutorial shows how to include a customized layergroup panel to represent the displayed layers in a ordered way. I have included layers from different layer sources, such as a few base layers, WMS and Geojson. You can customize the styling of the layergroup panel like you want, you only need to change the [style.css](https://github.com/g07kore/leaflet-nodejs-tutorials/blob/master/005-Leaflet-Custom-LayerGroup-Panel/style/style.css) file.

#### 006-Leaflet-WFS-Request:
Shows the simple approach of making a WFS (Web Feature Service) Request on Leaflet. The result is a JSON structure of each object in the ESRI Shapefile which is stored in a Geoserver. From here, you could create a fancy looking table structure with JQuery or whatever and display them in a nice way.

#### 007-Leaflet-Custom-Opacity-Slider:
Simple example of an opacity slider for Leaflet.


#### 008-Leaflet-Search-Function-Geojson:
In this tutorial, you can find a search function example for Geojson data. The example data is a geojson of the Austrian states. If you type three or more letters into the search field, it will automatically give you state suggestions. By clicking on one of the suggestions, it will zoom into that state on the map.

#### 009-Leaflet-Custom-LayerGroup-Panel-2:
Like tutorial 005, this tutorial shows how to include a customized layergroup panel but in a more modern way. Layers from different data sources were included. You can customize the appearance, you only need to change the [layer_control.css](https://github.com/g07kore/leaflet-nodejs-tutorials/blob/master/009-Leaflet-Custom-LayerGroup-Panel-2/style/layer_control.css) file. This custom styled layer control is based on the flawless work of Davi Custodio's [Leaflet.StyledLayerControl](https://github.com/davicustodio/Leaflet.StyledLayerControl).

#### 010-Leaflet-WMS-Common-Query-Language:
This tutorial shows how to use CQL (Common Query Language) on WMS layers. In the example, below the map it will be shown a screenshot of the attributes of the WMS layer. The example shows how to use CQL on all three attribute categories.

#### 011-Leaflet-Synchronized-Maps:
This example shows two synchronized maps next two each other. The [Leaflet.Sync](https://github.com/turban/Leaflet.Sync) package is already installed, if you don't have it just do a simple `npm install leaflet.sync`. Eventually it also requires a ```require('leaflet.sync')``` like it is shown in [app.js](https://github.com/g07kore/leaflet-nodejs-tutorials/blob/master/011-Leaflet-Synchronized-Maps/app.js) line 3.

#### 012-Leaflet-Marker-Cluster:
This tutorial shows you how to create marker clusters. by zooming in or out it will decrease or increase the markers into marker clusters. If you only use my code, make sure to install [Leaflet.markercluster](https://github.com/Leaflet/Leaflet.markercluster) package with a simple `npm install leaflet.markercluster`. Eventually it also requires a ```require('leaflet.markercluster')``` like it is shown in [app.js](https://github.com/g07kore/leaflet-nodejs-tutorials/blob/master/012-Leaflet-Marker-Cluster/app.js) line 3.
