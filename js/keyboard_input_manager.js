function KeyboardInputManager() {
  this.events = {};

  if (window.navigator.msPointerEnabled) {
    //Internet Explorer 10 style
    this.eventTouchstart = 'MSPointerDown';
    this.eventTouchmove = 'MSPointerMove';
    this.eventTouchend = 'MSPointerUp';
  } else {
    this.eventTouchstart = 'touchstart';
    this.eventTouchmove = 'touchmove';
    this.eventTouchend = 'touchend';
  }

  this.listen();
}

KeyboardInputManager.prototype.on = function (event, callback) {
  if (!this.events[event]) {
    this.events[event] = [];
  }
  this.events[event].push(callback);
};

KeyboardInputManager.prototype.emit = async function (event, data) {
  var callbacks = this.events[event];
  if (callbacks) {
    for (const callback of callbacks) {
      await callback(data);
    }
  }
};

KeyboardInputManager.prototype.listen = function () {
  var self = this;

  var map = {
    38: 0, // Up
    37: 1, // Left
    40: 2, // Down
    39: 3, // Right
    75: 0, // Vim up
    72: 1, // Vim left
    74: 2, // Vim down
    76: 3, // Vim right
    87: 0, // W
    65: 1, // A
    83: 2, // S
    68: 3 // D
  };

  // Respond to direction keys
  document.addEventListener('keydown', function (event) {
    var modifiers =
      event.altKey || event.ctrlKey || event.metaKey || event.shiftKey;
    var mapped = map[event.which];

    if (!modifiers) {
      if (mapped !== undefined) {
        event.preventDefault();
        self.emit('move', mapped);
      }
    }

    // R key restarts the game
    if (!modifiers && event.which === 82) {
      self.restart.call(self, event);
    }
  });

  // Respond to button presses
  this.bindButtonPress('.retry-button', this.restart);
  this.bindButtonPress('.restart-button', this.restart);
  this.bindButtonPress('#run-button', this.autorun);
  this.bindButtonPress('#evil-button', this.evilTile);
  this.bindButtonPress('.keep-playing-button', this.keepPlaying);

  // Respond to swipe events
  var touchStartClientX, touchStartClientY;
  var gameContainer = document.getElementsByClassName('game-container')[0];

  gameContainer.addEventListener(this.eventTouchstart, function (event) {
    if (
      (!window.navigator.msPointerEnabled && event.touches.length > 1) ||
      event.targetTouches.length > 1
    ) {
      return; // Ignore if touching with more than 1 finger
    }

    if (window.navigator.msPointerEnabled) {
      touchStartClientX = event.pageX;
      touchStartClientY = event.pageY;
    } else {
      touchStartClientX = event.touches[0].clientX;
      touchStartClientY = event.touches[0].clientY;
    }

    event.preventDefault();
  });

  gameContainer.addEventListener(this.eventTouchmove, function (event) {
    event.preventDefault();
  });

  gameContainer.addEventListener(this.eventTouchend, function (event) {
    if (
      (!window.navigator.msPointerEnabled && event.touches.length > 0) ||
      event.targetTouches.length > 0
    ) {
      return; // Ignore if still touching with one or more fingers
    }

    var touchEndClientX, touchEndClientY;

    if (window.navigator.msPointerEnabled) {
      touchEndClientX = event.pageX;
      touchEndClientY = event.pageY;
    } else {
      touchEndClientX = event.changedTouches[0].clientX;
      touchEndClientY = event.changedTouches[0].clientY;
    }

    var dx = touchEndClientX - touchStartClientX;
    var absDx = Math.abs(dx);

    var dy = touchEndClientY - touchStartClientY;
    var absDy = Math.abs(dy);

    if (Math.max(absDx, absDy) > 10) {
      // (right : left) : (down : up)
      self.emit('move', absDx > absDy ? (dx > 0 ? 1 : 3) : dy > 0 ? 2 : 0);
    }
  });
};

KeyboardInputManager.prototype.restart = function (event) {
  event.preventDefault();
  this.emit('restart');
};

KeyboardInputManager.prototype.autorun = function (event) {
  event.preventDefault();
  this.emit('autorun');
};

KeyboardInputManager.prototype.evilTile = function (event) {
  event.preventDefault();
  this.emit('evilTile');
};

KeyboardInputManager.prototype.keepPlaying = function (event) {
  event.preventDefault();
  this.emit('keepPlaying');
};

KeyboardInputManager.prototype.bindButtonPress = function (selector, fn) {
  var button = document.querySelector(selector);
  button.addEventListener('click', fn.bind(this));
  button.addEventListener(this.eventTouchend, fn.bind(this));
};
