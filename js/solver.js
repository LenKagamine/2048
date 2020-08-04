function Solver(onload) {
  this.resolves = {};

  this.worker = new Worker('js/worker.js');
  this.worker.onmessage = ({ data }) => {
    console.log(data);
    if (data.type === 'READY') {
      onload();
    } else if (this.resolves[data.type] != null) {
      const resolve = this.resolves[data.type].shift();
      resolve(data.args);
    }
  };
}

Solver.prototype.sendMessage = function (type, args = {}) {
  return new Promise(resolve => {
    if (this.resolves[type] == null) {
      this.resolves[type] = [];
    }

    this.resolves[type].push(resolve);
    this.worker.postMessage({ type, ...args });
  });
};

Solver.prototype.setup = function (seed) {
  return this.sendMessage('SETUP', { seed });
};

Solver.prototype.setBoard = function (tiles) {
  return this.sendMessage('BOARD', { tiles });
};

Solver.prototype.move = function (direction) {
  return this.sendMessage('MOVE', { direction });
};

Solver.prototype.addTile = function (position, value) {
  return this.sendMessage('ADD', { position, value });
};

Solver.prototype.getMove = function () {
  return this.sendMessage('SOLVE');
};

Solver.prototype.getTile = function () {
  return this.sendMessage('TILE');
};

Solver.prototype.reset = function (seed) {
  return this.sendMessage('RESET', { seed });
};
