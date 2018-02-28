"use strict";

var path = require('path')
var through = require("through");

module.exports = function (fileName, options) {

    options = options || {};
    if (typeof(options['auto-inject']) == 'undefined') {
        options['auto-inject'] = true;
    }

    if (!/\.css$/i.test(fileName)) {
        return through();
    }

    var inputString = "";

    return through(
        function (chunk) {
            inputString += chunk;
        },
        function () {
            var stringifiedCss = JSON.stringify(inputString);
            var requirePath = path.relative(path.dirname(fileName), __dirname)
            var moduleBody = options['auto-inject']
              ? "var css = " + stringifiedCss + "; (require("+JSON.stringify('./'+requirePath)+"))(css, undefined, '" + fileName + "'); module.exports = css;"
              : "module.exports = " + stringifiedCss;

            this.queue(moduleBody);
            this.queue(null);
        }
    );
};
