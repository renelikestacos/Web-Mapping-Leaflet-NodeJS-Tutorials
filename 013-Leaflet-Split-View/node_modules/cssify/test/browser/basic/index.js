'use strict'

var tapeCatch = require('tape-catch')
var css = require('./style.css')
var injectCss = require('../../..')

var styleId = 'injection-testing'

function test (desc, fn) {
  tapeCatch(desc, function (t) {
    try {
      setTimeout(function () { fn(t) }, 0)
    } catch (err) {
      t.fail(err)
    }
  })
}

test('browser: basic usage', function (t) {
  t.equal(css, 'body { position: absolute; }\n', 'styles exported')

  injectCss(css, undefined, styleId)
  t.equal(
    window.getComputedStyle(document.body).position,
    'absolute',
    'styles injected'
  )

  injectCss('body { width: 300px; }')
  injectCss('body { font-size: 5px; }')

  const computedStyle = window.getComputedStyle(document.body)

  const actualStyles = {
    position: computedStyle.position,
    width: computedStyle.width,
    fontSize: computedStyle.fontSize
  }

  const expectedStyles = {
    position: 'absolute',
    width: '300px',
    fontSize: '5px'
  }

  t.deepEqual(actualStyles, expectedStyles, 'inject styles without an id')
  t.end()
})

test('browser: hot module replacement', function (t) {
  injectCss('body { position: absolute; }\n', undefined, styleId)

  t.equal(
    window.getComputedStyle(document.body).position,
    'absolute',
    'first injection'
  )

  injectCss('', undefined, styleId)

  t.equal(
    window.getComputedStyle(document.body).position,
    'static',
    'second injection'
  )

  t.end()
})
