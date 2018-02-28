function injectStyleTag(document, fileName, cb) {
  var style = document.getElementById(fileName);

  if (style) {
    cb(style);
  } else {
    var head = document.getElementsByTagName('head')[0];

    style = document.createElement('style');
    style.id = fileName;
    cb(style);
    head.appendChild(style);
  }

  return style;
}

module.exports = function (css, customDocument, fileName) {
  var doc = customDocument || document;
  if (doc.createStyleSheet) {
    var sheet = doc.createStyleSheet()
    sheet.cssText = css;
    return sheet.ownerNode;
  } else {
    return injectStyleTag(doc, fileName, function(style) {
      if (style.styleSheet) {
        style.styleSheet.cssText = css;
      } else {
        style.innerHTML = css;
      }
    });
  }
};

module.exports.byUrl = function(url) {
  if (document.createStyleSheet) {
    return document.createStyleSheet(url).ownerNode;
  } else {
    var head = document.getElementsByTagName('head')[0],
        link = document.createElement('link');

    link.rel = 'stylesheet';
    link.href = url;

    head.appendChild(link);
    return link;
  }
};
