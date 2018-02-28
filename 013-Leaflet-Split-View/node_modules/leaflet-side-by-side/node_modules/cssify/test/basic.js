"use strict";

var fs = require("fs");
var path = require("path")
var assert = require("assert");
var concatStream = require("concat-stream");
var browserify = require("browserify");
var jsdom = require("jsdom").jsdom;
var cssify = require("..");

var pageHtml = fs.readFileSync(fixturePath("index.html"), "utf8");

specify("It gives the desired output", function (done) {

  var bundleStream = browserify()
    .transform(cssify)
    .add(fixturePath("entry.js"))
    .require('..', {expose: 'cssify'})
    .bundle();

  bundleStream.pipe(concatStream(function (bundleJs) {

    var window = jsdom(pageHtml).createWindow();

    var c1 = colors(window);
    assert.equal(c1.bg, '')
    assert.equal(c1.fg, '')

    var scriptEl = window.document.createElement("script");
    scriptEl.textContent = bundleJs;
    window.document.head.appendChild(scriptEl);

    var c2 = colors(window);
    assert.equal(c2.bg, 'purple')
    assert.equal(c2.fg, 'yellow')

    done();

  }));
});

specify("It does not automatically inject styles", function (done) {

  var bundleStream = browserify()
    .transform({"auto-inject": false}, cssify)
    .add(fixturePath("entry.js"))
    .require('..', {expose: 'cssify'})
    .require(fixturePath('style.css'), {expose: 'mycss'})
    .bundle();

  bundleStream.pipe(concatStream(function (bundleJs) {
    var window = jsdom(pageHtml).createWindow();

    var colorsBeforeIncludingBundle = colors(window);
    assert.equal(colorsBeforeIncludingBundle.bg, '')
    assert.equal(colorsBeforeIncludingBundle.fg, '')

    var scriptEl = window.document.createElement("script");
    scriptEl.textContent = bundleJs;
    window.document.head.appendChild(scriptEl);

    // no auto inject
    var colorsAfterIncludingBundle = colors(window);
    assert.equal(colorsAfterIncludingBundle.bg, '')
    assert.equal(colorsAfterIncludingBundle.fg, '')

    // inject manually
    window.require('cssify')(window.require('mycss'))
    var colorsAfterAddingStyles = colors(window);
    assert.equal(colorsAfterAddingStyles.bg, 'purple')
    assert.equal(colorsAfterAddingStyles.fg, 'yellow')

    done();

  }));
});

specify("It adds styles into an iframe", function (done) {

  var bundleStream = browserify()
    .transform({"auto-inject": false}, cssify)
    .add(fixturePath("entry.js"))
    .require('..', {expose: 'cssify'})
    .require(fixturePath('style.css'), {expose: 'mycss'})
    .bundle();

  bundleStream.pipe(concatStream(function (bundleJs) {
    // boooooy this is complicated - I think once https://github.com/tmpvar/jsdom/issues/749 is fixed, we shouldn't need this
    var window = jsdom(pageHtml, null, {
      file: fixturePath("index.html"),
      features: {
        FetchExternalResources: ["iframe"]
      }
    }).createWindow();

    var scriptEl = window.document.createElement("script");
    scriptEl.textContent = bundleJs;
    window.document.head.appendChild(scriptEl);

    var iframe = window.document.createElement('iframe');
    iframe.src = fixturePath('index.html'); // workaround for https://github.com/tmpvar/jsdom/issues/749
    iframe.onload = function () {

      var defaultIframeColors = colors(iframe.contentWindow);
      assert.equal(defaultIframeColors.bg, '')
      assert.equal(defaultIframeColors.fg, '')

      // inject into an iframe
      window.require('cssify')(window.require('mycss'), iframe.contentWindow.document)

      // original window should not be modified
      var originalBodyColors = colors(window);
      assert.equal(originalBodyColors.bg, '')
      assert.equal(originalBodyColors.fg, '')

      // only the iframe should be modified
      var iframeModifiedColors = colors(iframe.contentWindow);
      assert.equal(iframeModifiedColors.bg, 'purple')
      assert.equal(iframeModifiedColors.fg, 'yellow')

      done();

    }
    window.document.body.appendChild(iframe);

  }));
});

specify("It returns the stylesheet element", function (done) {

  var bundleStream = browserify()
    .transform(cssify)
    .add(fixturePath("entry.js"))
    .require('..', {expose: 'cssify'})
    .require(fixturePath('style.css'), {expose: 'mycss'})
    .bundle();

  bundleStream.pipe(concatStream(function (bundleJs) {

    var window = jsdom(pageHtml).createWindow();

    var scriptEl = window.document.createElement("script");
    scriptEl.textContent = bundleJs;
    window.document.head.appendChild(scriptEl);

    var el = window.require('cssify')(window.require('mycss'));
    assert.equal(el.nodeName, 'STYLE', 'style')

    done();

  }));
});

function fixturePath(fileName) {
  return path.resolve(__dirname, "fixtures/basic", fileName);
}

function colors(win) {
  var style = win.getComputedStyle(win.document.body);
  return {
    bg: style.backgroundColor.replace(/\s+/g, ''),
    fg: style.color.replace(/\s+/g, '')
  };
}
