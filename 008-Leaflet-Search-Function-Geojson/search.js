function sort_geojson(a, b) {
  var _a = a.feature.properties.name;
  var _b = b.feature.properties.name;
  if (_a < _b) {
    return -1;
  }
  if (_a > _b) {
    return 1;
  }
  return 0;
}

L.Control.Search = L.Control.extend({
  options: {
    position: 'bottomright',
    placeholder: 'Search...'
  },
  initialize: function (options) {
    L.Util.setOptions(this, options);
  },
  onAdd: function (map) {
    var container = L.DomUtil.create('div', 'search-container');
    this.form = L.DomUtil.create('form', 'form', container);
    var group = L.DomUtil.create('div', 'form-group', this.form);
    this.input = L.DomUtil.create('input', 'form-control input-sm', group);
    this.input.type = 'text';
    this.input.placeholder = this.options.placeholder;
    this.results = L.DomUtil.create('div', 'list-group', group);
    L.DomEvent.addListener(this.input, 'keyup', _.debounce(this.keyup, 300), this);
    L.DomEvent.addListener(this.form, 'submit', this.submit, this);
    L.DomEvent.disableClickPropagation(container);
    return container;
  },
  onRemove: function (map) {
    L.DomEvent.removeListener(this._input, 'keyup', this.keyup, this);
    L.DomEvent.removeListener(form, 'submit', this.submit, this);
  },
  keyup: function(e) {
    if (e.keyCode === 38 || e.keyCode === 40) {
    } else {
      this.results.innerHTML = '';
      if (this.input.value.length > 2) {
        var value = this.input.value;
        var results = _.take(_.filter(this.options.data, function(x) {
          return x.feature.properties.name.toUpperCase().indexOf(value.toUpperCase()) > -1;
        }).sort(sort_geojson), 10);
        _.map(results, function(x) {
          var a = L.DomUtil.create('a', 'list-group-item');
          a.href = '';
          a.setAttribute('data-result-name', x.feature.properties.name);
          a.innerHTML = x.feature.properties.name;
          this.results.appendChild(a);
          L.DomEvent.addListener(a, 'click', this.itemSelected, this);
          return a;
        }, this);
      }
    }
  },
  itemSelected: function(e) {
    L.DomEvent.preventDefault(e);
    var elem = e.target;
    var value = elem.innerHTML;
    this.input.value = elem.getAttribute('data-result-name');
    var feature = _.find(this.options.data, function(x) {
      return x.feature.properties.name === this.input.value;
    }, this);
    if (feature) {
      this._map.fitBounds(feature.getBounds());
    }
    this.results.innerHTML = '';
  },
  submit: function(e) {
    L.DomEvent.preventDefault(e);
  }
});
L.control.search = function(id, options) {
  return new L.Control.Search(id, options);
}