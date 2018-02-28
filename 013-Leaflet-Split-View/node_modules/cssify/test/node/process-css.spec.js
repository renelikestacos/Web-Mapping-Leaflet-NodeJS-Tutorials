'use strict'

var test = require('tape-catch')
var assign = require('lodash.assign')
var processCss = require('../../lib/process-css')

var defaultOptions = { _flags: {} }

var fileName = 'test.css'
var hashedFilename = '_nu4uke'

var css = '.test-class { font-family: "Times New Roman", sans-serif; }'
var escapedCss = '.test-class { font-family: \\"Times New Roman\\", sans-serif; }'

// var devMap = '{\n\t\'test-class\': \'_test___test-class\'\n}'
// var devCss = '._test___test-class { font-family: \\"Times New Roman\\", sans-serif; }'

var prodMap = '{\n\t\'test-class\': \'_1j9q0wu\'\n}'
var prodCss = '._1j9q0wu { font-family: \\"Times New Roman\\", sans-serif; }'

test('processCss', function (t) {
  processCss(fileName, css, assign({}, defaultOptions))
    .then(function (moduleSource) {
      t.equal(
        moduleSource,
        'module.exports = "' + escapedCss + '";\n',
        'without injection'
      )

      return processCss(fileName, css, assign({}, defaultOptions, {
        'auto-inject': true
      }))
    })
    .then(function (moduleSource) {
      t.equal(
        moduleSource, [
          'var inject = require(\'./\');',
          'var css = "' + escapedCss + '";',
          'inject(css, undefined, \'' + hashedFilename + '\');',
          'module.exports = css;'
        ].join('\n') + '\n',
        'with injection'
      )

      return processCss(fileName, css, assign({}, defaultOptions, {
        modules: true
      }))
    })
    .then(function (moduleSource) {
      t.equal(
        moduleSource,
        'module.exports = { css: "' + prodCss + '", map: ' + prodMap + ' };\n',
        'with modules'
      )

      return processCss(fileName, css, assign({}, defaultOptions, {
        'auto-inject': true,
        modules: true
      }))
    })
    .then(function (moduleSource) {
      t.equal(
        moduleSource, [
          'var inject = require(\'./\');',
          'var css = "' + prodCss + '";',
          'inject(css, undefined, \'' + hashedFilename + '\');',
          'module.exports = ' + prodMap + ';'
        ].join('\n') + '\n',
        'with modules and injection'
      )

      return processCss(fileName, css, assign({}, defaultOptions, {
        modules: true,
        _flags: assign({}, defaultOptions._flags, { debug: true })
      }))
    })
    .then(function (moduleSource) {
      // t.equal(
      //   moduleSource,
      //   'module.exports = { css: "' + devCss + '", map: ' + devMap + ' };\n',
      //   'with debug'
      // )
    })
    .then(t.end)
    .catch(t.fail)
})
