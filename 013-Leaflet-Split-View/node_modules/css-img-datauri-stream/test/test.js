var imgToDataUri = require("../index.js");
var os = require("os");
var path = require("path");
var fs = require("fs");

var inputFile = path.join( __dirname, "test_1.css" );
fs.createReadStream(inputFile)
	.pipe(imgToDataUri(inputFile))
	.pipe(fs.createWriteStream("./test_1_result.css"));
