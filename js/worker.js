const Module = {};

Module.onRuntimeInitialized = () => {
  self.onmessage = ({ data }) => {
    console.log(data);
    if (data.type === 'SETUP') {
      const { seed } = data;
      Module.setup(seed || 0);
      self.postMessage({ type: 'SETUP' });
    } else if (data.type === 'BOARD') {
      const { tiles } = data;

      // Convert to C++ vector
      const vec = new Module['VecInt']();
      vec['resize'](tiles.length, 0);
      for (let i = 0; i < tiles.length; i++) {
        vec['set'](i, tiles[i]);
      }

      Module.setBoard(vec);
      self.postMessage({ type: 'BOARD' });
    } else if (data.type === 'MOVE') {
      const { direction } = data;
      Module.move(direction);
      self.postMessage({ type: 'MOVE' });
    } else if (data.type === 'ADD') {
      const { position, value } = data;
      Module.addTile(position, value);
      self.postMessage({ type: 'ADD' });
    } else if (data.type === 'SOLVE') {
      const solution = Module.getMove();
      self.postMessage({ type: 'SOLVE', args: solution });
    } else if (data.type === 'TILE') {
      const tile = Module.getTile();
      self.postMessage({ type: 'TILE', args: tile });
    } else if (data.type === 'RESET') {
      const { seed } = data;
      Module.reset(seed || 0);
      self.postMessage({ type: 'RESET' });
    }
  };

  self.postMessage({ type: 'READY' });
};

self.importScripts('solve.js');
