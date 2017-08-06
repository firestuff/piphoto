'use strict';

function leftPad(str, len) {
  while (str.length < len) {
    str = '0' + str;
  }
  return str;
}

function createElement(parentNode, tagName, opt_text) {
  let element = document.createElement(tagName);
  if (opt_text !== undefined) {
    element.textContent = opt_text;
  }
  parentNode.appendChild(element);
  return element;
}


class Point {
  constructor(r, g, b) {
    this.r = r;
    this.g = g;
    this.b = b;

    this.ref = 0;

    this.HEX_DIV = 256;
    this.HEX_PAD = 2;
  }

  getHexColor() {
    return '#' + this.toHexValue(this.r) + this.toHexValue(this.g) + this.toHexValue(this.b);
  }

  toHexValue(val) {
    return leftPad(Math.floor(val / this.HEX_DIV).toString(16), this.HEX_PAD);
  }

  incRef() {
    this.ref++;
  }

  addSwatch(parentElement) {
    let elem = createElement(parentElement, 'td')
    elem.classList.add('swatch');
    elem.style = 'background-color: ' + this.getHexColor();
    return elem;
  }

  addRgb(parentElement) {
    createElement(parentElement, 'td', leftPad(this.r.toString(16), 4));
    createElement(parentElement, 'td', leftPad(this.g.toString(16), 4));
    createElement(parentElement, 'td', leftPad(this.b.toString(16), 4));
  }

  addRef(parentElement) {
    createElement(parentElement, 'td', this.ref);
  }
}

class LutExperiment {
  constructor(container) {
    this.container = container;

    this.SIZE_X = 4;  // R
    this.SIZE_Y = 3;  // G
    this.SIZE_Z = 3;  // B
    this.NUM_COLOR = 2 ** 16;
    
    this.generatePoints();

    this.COLOR_CHECKER = [
      new Point(0x7300, 0x5200, 0x4400),
      new Point(0xc200, 0x9600, 0x8200),
      new Point(0x6200, 0x7a00, 0x9d00),
      new Point(0x5700, 0x6c00, 0x4300),
      new Point(0x8500, 0x8000, 0xb100),
      new Point(0x6700, 0xbd00, 0xaa00),
      new Point(0xd600, 0x7e00, 0x2c00),
      new Point(0x5000, 0x5b00, 0xa600),
      new Point(0xc100, 0x5a00, 0x6300),
      new Point(0x5e00, 0x3c00, 0x6c00),
      new Point(0x9d00, 0xbc00, 0x4000),
      new Point(0xe000, 0xa300, 0x2e00),
      new Point(0x3800, 0x3d00, 0x9600),
      new Point(0x4600, 0x9400, 0x4900),
      new Point(0xaf00, 0x3600, 0x3c00),
      new Point(0xe700, 0xc700, 0x1f00),
      new Point(0xbb00, 0x5600, 0x9500),
      new Point(0x0800, 0x8500, 0xa100),
      new Point(0xf300, 0xf300, 0xf200),
      new Point(0xc800, 0xc800, 0xc800),
      new Point(0xa000, 0xa000, 0xa000),
      new Point(0x7a00, 0x7a00, 0x7900),
      new Point(0x5500, 0x5500, 0x5500),
      new Point(0x3400, 0x3400, 0x3400),
    ];

    this.CUBE_OFFSET = [
      [0, 0, 0],
      [1, 0, 0],
      [1, 1, 0],
      [0, 1, 0],
      [0, 0, 1],
      [1, 0, 1],
      [1, 1, 1],
      [0, 1, 1],
    ];

    this.showColorChecker();
    this.showResults();
  }

  generatePoints() {
    this.points = new Array();
    for (let x = 0; x < this.SIZE_X; ++x) {
      this.points[x] = new Array();

      for (let y = 0; y < this.SIZE_Y; ++y) {
        this.points[x][y] = new Array();
        
        for (let z = 0; z < this.SIZE_Z; ++z) {
          this.points[x][y][z] = new Point(
            this.getColorValue(x, this.SIZE_X),
            this.getColorValue(y, this.SIZE_Y),
            this.getColorValue(z, this.SIZE_Z),
          );
        }
      }
    }
  }

  getColorValue(index, size) {
    let per_block = Math.floor(this.NUM_COLOR / (size - 1));
    return Math.min(this.NUM_COLOR - 1, per_block * index);
  }

  getIndex(val, size) {
    let per_block = Math.floor(this.NUM_COLOR / (size - 1));
    return Math.min(Math.floor(val / per_block), size - 2);
  }

  showColorChecker() {
    let table = createElement(this.container, 'table');

    let headers = createElement(table, 'tr');
    createElement(headers, 'th', 'Swatch');
    createElement(headers, 'th', 'R');
    createElement(headers, 'th', 'G');
    createElement(headers, 'th', 'B');
    for (let offset of this.CUBE_OFFSET) {
      createElement(headers, 'th', offset.join(','));
    }

    for (let point of this.COLOR_CHECKER) {
      let tr = createElement(table, 'tr');
      point.addSwatch(tr);
      point.addRgb(tr);

      let x = this.getIndex(point.r, this.SIZE_X);
      let y = this.getIndex(point.g, this.SIZE_Y);
      let z = this.getIndex(point.b, this.SIZE_Z);

      for (let [ox, oy, oz] of this.CUBE_OFFSET) {
        this.addSwatch(tr, x + ox, y + oy, z + oz);
        this.points[x + ox][y + oy][z + oz].incRef();
      }
    }
  }

  showResults() {
    let table = createElement(this.container, 'table');

    let headers = createElement(table, 'tr');
    createElement(headers, 'th', 'Swatch');
    createElement(headers, 'th', 'R');
    createElement(headers, 'th', 'G');
    createElement(headers, 'th', 'B');
    createElement(headers, 'th', 'Ref');

    for (let x = 0; x < this.SIZE_X; ++x) {
      for (let y = 0; y < this.SIZE_Y; ++y) {
        for (let z = 0; z < this.SIZE_Z; ++z) {
          let point = this.points[x][y][z];
          let tr = createElement(table, 'tr');
          this.addSwatch(tr, x, y, z);
          point.addRgb(tr);
          point.addRef(tr);
        }
      }
    }
  }

  addSwatch(parentElement, x, y, z) {
    this.points[x][y][z].addSwatch(parentElement).textContent = x + ',' + y + ',' + z;
  }
}
