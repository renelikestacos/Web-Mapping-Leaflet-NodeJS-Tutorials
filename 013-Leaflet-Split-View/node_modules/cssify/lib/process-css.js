'use strict'

var path = require('path')
var Core = require('css-modules-loader-core')
var stringHash = require('string-hash')
var stringifyObject = require('stringify-object')

function escapeCss (css) {
  return JSON.stringify(css)
}

function hash (str) {
  return '_' + stringHash(str).toString(36)
}

function generateHashName (styleName, fileName) {
  return hash(fileName + ':' + styleName)
}

function generateDebugName (styleName, fileName) {
  var sanitisedPath = fileName
    .replace(/\.[^\.\/\\]+$/, '')
    .replace(/[\W_]+/g, '_')
    .replace(/^_|_$/g, '')

  return '_' + sanitisedPath + '__' + styleName
}

function wrapCss (fileName, css, options, map) {
  var escapedCss = escapeCss(css)
  var stringifiedMap = stringifyObject(map)
  var packagePath = path.join(__dirname, '..')
  var dirName = path.dirname(fileName)
  var requirePath = path.relative(dirName, packagePath)

  // On Windows, path.relative returns unescaped backslashes and
  // that causes cssify to not be findable.
  requirePath = requirePath.replace(/\\/g, '/')

  var moduleSource = options['auto-inject']
    ? [
      'var inject = require(\'./' + requirePath + '\');',
      'var css = ' + escapedCss + ';',
      'inject(css, undefined, \'' + hash(fileName) + '\');',
      options.modules
        ? 'module.exports = ' + stringifiedMap + ';'
        : 'module.exports = css;'
    ].join('\n') + '\n'
    : options.modules
      ? 'module.exports = { css: ' + escapedCss + ', map: ' + stringifiedMap + ' };\n'
      : 'module.exports = ' + escapedCss + ';\n'

  return moduleSource
}

function processCss (fileName, source, options) {
  if (options.modules) {
    Core.scope.generateScopedName = options.debug
      ? generateDebugName
      : generateHashName

    var core = new Core()

    return core.load(source, path.relative(process.cwd(), fileName))
      .then(function (result) {
        return wrapCss(
          fileName,
          result.injectableSource,
          options,
          result.exportTokens
        )
      })
  }

  return Promise.resolve(wrapCss(fileName, source, options))
}

module.exports = processCss
