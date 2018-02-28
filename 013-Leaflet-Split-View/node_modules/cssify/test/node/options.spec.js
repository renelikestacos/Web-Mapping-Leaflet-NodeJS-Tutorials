'use strict'

var test = require('tape-catch')
var options = require('../../lib/options')
var normalize = options.normalize
var skipIt = options.skipIt
var stringToRegExp = options.stringToRegExp

test('options.normalize', function (t) {
  t.deepEqual(
    normalize(),
    { 'auto-inject': true, test: /\.css$/i },
    'falls back to defaults'
  )

  t.equal(
    normalize({ 'auto-inject': null })['auto-inject'],
    null,
    'falsy value for auto-inject is returned as is'
  )

  t.equal(
    normalize({ test: '/str/' }).test.toString(),
    '/str/',
    'regular expression string is parsed for test option'
  )

  var testObj = {}

  t.equal(
    normalize({ test: testObj }).test,
    testObj,
    'non-string value for test option is passed as is'
  )

  t.deepEqual(
    normalize({ 'no-auto-inject': true }),
    { 'auto-inject': false, test: /\.css$/i },
    'no-auto-inject option'
  )

  t.end()
})

test('options.skipIt', function (t) {
  t.equal(skipIt('', {}), false, 'nothing to match')

  t.equal(
    skipIt('./style\.css', { test: function () { return false } }),
    true,
    'function returning false'
  )

  t.equal(
    skipIt('./style\.css', { test: function () { return true } }),
    false,
    'function returning true'
  )

  t.equal(
    skipIt('./style\.styl', { test: /\.styl$/ }),
    false,
    'matching RegExp'
  )

  t.equal(
    skipIt('./style\.styl', { test: /\.css$/ }),
    true,
    'not-matching RegExp'
  )

  t.end()
})

test('options.stringToRegExp', function (t) {
  var reSource = '/str/'
  var re = stringToRegExp(reSource)

  t.equal(re instanceof RegExp, true, 'RegExp is instantiated')

  t.equal(
    re.toString(), reSource,
    'regular expression converted to string matches source'
  )

  t.equal(
    stringToRegExp(), undefined,
    'returns undefined if arg is falsy'
  )

  t.end()
})
