function Solver(onload) {
  this.resolves = {
    setup: [],
    board: [],
    solve: [],
    add: [],
    reset: []
  };

  this.worker = new Worker('js/worker.js');
  this.worker.onmessage = ({ data }) => {
    if (data.type === 'READY') {
      onload();
    } else if (data.type === 'SETUP_DONE') {
      const resolve = this.resolves.setup.shift();
      resolve();
    } else if (data.type === 'BOARD_DONE') {
      const resolve = this.resolves.board.shift();
      resolve();
    } else if (data.type === 'ADD_DONE') {
      const resolve = this.resolves.add.shift();
      resolve();
    } else if (data.type === 'SOLVE_DONE') {
      const resolve = this.resolves.solve.shift();
      resolve(data.solution);
    } else if (data.type === 'RESET_DONE') {
      const resolve = this.resolves.reset.shift();
      resolve();
    }
  };
}

Solver.prototype.setup = function (seed) {
  return new Promise(resolve => {
    this.resolves.setup.push(resolve);
    this.worker.postMessage({ type: 'SETUP', seed });
  });
};

Solver.prototype.setBoard = function (tiles) {
  return new Promise(resolve => {
    this.resolves.board.push(resolve);
    this.worker.postMessage({ type: 'BOARD', tiles });
  });
};

Solver.prototype.addTile = function (position, value) {
  return new Promise(resolve => {
    this.resolves.add.push(resolve);
    this.worker.postMessage({ type: 'ADD', position, value });
  });
};

Solver.prototype.getMove = function () {
  return new Promise(resolve => {
    this.resolves.solve.push(resolve);
    this.worker.postMessage({ type: 'SOLVE' });
  });
};

Solver.prototype.reset = function (seed) {
  return new Promise(resolve => {
    this.resolves.reset.push(resolve);
    this.worker.postMessage({ type: 'RESET', seed });
  });
};
