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

    this.HEX_DIV = 256;
    this.HEX_PAD = 2;
  }

  getHexColor() {
    return '#' + this.toHexValue(this.r) + this.toHexValue(this.g) + this.toHexValue(this.b);
  }

  toHexValue(val) {
    return leftPad(Math.floor(val / this.HEX_DIV).toString(16), this.HEX_PAD);
  }

  addSwatch(parentElement) {
    let elem = createElement(parentElement, 'td')
    elem.style = 'width: 5em; text-align: center; text-shadow: 1px 1px 0 white; background-color: ' + this.getHexColor();
    return elem;
  }

  addColorCells(parentElement) {
    this.addSwatch(parentElement);
    createElement(parentElement, 'td', leftPad(this.r.toString(16), 4));
    createElement(parentElement, 'td', leftPad(this.g.toString(16), 4));
    createElement(parentElement, 'td', leftPad(this.b.toString(16), 4));
  }
}

class LutExperiment {
  constructor(container) {
    this.container = container;

    this.SIZE = 5;
    this.NUM_COLOR = 2 ** 16;
    this.PER_BLOCK = Math.floor(this.NUM_COLOR / (this.SIZE - 1));
    
    this.generatePoints();

    this.colorChecker = [
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

    this.showColorChecker();
    this.showResults();
  }

  generatePoints() {
    this.points = new Array();
    for (let x = 0; x < this.SIZE; ++x) {
      this.points[x] = new Array();

      for (let y = 0; y < this.SIZE; ++y) {
        this.points[x][y] = new Array();
        
        for (let z = 0; z < this.SIZE; ++z) {
          this.points[x][y][z] = new Point(this.getColorValue(x), this.getColorValue(y), this.getColorValue(z));
        }
      }
    }
  }

  getColorValue(index) {
    return Math.min(this.NUM_COLOR - 1, this.PER_BLOCK * index);
  }

  showColorChecker() {
    let table = createElement(this.container, 'table');

    let headers = createElement(table, 'tr');
    createElement(headers, 'th', 'Swatch');
    createElement(headers, 'th', 'R');
    createElement(headers, 'th', 'G');
    createElement(headers, 'th', 'B');
    createElement(headers, 'th', 'Root');
    createElement(headers, 'th', 'x+1');
    createElement(headers, 'th', 'y+1');
    createElement(headers, 'th', 'z+1');
    createElement(headers, 'th', 'x+1,y+1');
    createElement(headers, 'th', 'x+1,z+1');
    createElement(headers, 'th', 'y+1,z+1');

    for (let point of this.colorChecker) {
      let tr = createElement(table, 'tr');
      point.addColorCells(tr);

      let rootX = Math.min(Math.floor(point.r / this.PER_BLOCK), this.SIZE - 2);
      let rootY = Math.min(Math.floor(point.g / this.PER_BLOCK), this.SIZE - 2);
      let rootZ = Math.min(Math.floor(point.b / this.PER_BLOCK), this.SIZE - 2);

      this.points[rootX][rootY][rootZ].addSwatch(tr).textContent = rootX + ',' + rootY + ',' + rootZ;
      this.points[rootX + 1][rootY][rootZ].addSwatch(tr);
      this.points[rootX][rootY + 1][rootZ].addSwatch(tr);
      this.points[rootX][rootY][rootZ + 1].addSwatch(tr);
      this.points[rootX + 1][rootY + 1][rootZ].addSwatch(tr);
      this.points[rootX + 1][rootY][rootZ + 1].addSwatch(tr);
      this.points[rootX][rootY + 1][rootZ + 1].addSwatch(tr);
    }
  }

  showResults() {
    let table = createElement(this.container, 'table');

    let headers = createElement(table, 'tr');
    createElement(headers, 'th', 'X');
    createElement(headers, 'th', 'Y');
    createElement(headers, 'th', 'Z');
    createElement(headers, 'th', 'Swatch');
    createElement(headers, 'th', 'R');
    createElement(headers, 'th', 'G');
    createElement(headers, 'th', 'B');

    for (let x = 0; x < this.SIZE; ++x) {
      let square = this.points[x];

      for (let y = 0; y < this.SIZE; ++y) {
        let row = square[y];

        for (let z = 0; z < this.SIZE; ++z) {
          let point = row[z];

          let tr = createElement(table, 'tr');
          createElement(tr, 'td', x);
          createElement(tr, 'td', y);
          createElement(tr, 'td', z);
          point.addColorCells(tr);
        }
      }
    }
  }
}
