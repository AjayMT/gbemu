
# gbemu
gbemu is a simple GameBoy emulator. It is currently a work in progress; see the TODO section.

![](https://raw.githubusercontent.com/AjayMT/gbemu/master/images/cpu.png)
![](https://raw.githubusercontent.com/AjayMT/gbemu/master/images/tetris.png)

## Build
Requirements:
- [CSFML](https://www.sfml-dev.org/download/csfml/)

Run the following in the project root directory:
```
make
```

This will produce a `gbemu` executable.

To build the tests:
```
make gbemu_tests
```

## Usage
```
gbemu <filename>
```

The `<filename>` argument is the path to a GameBoy ROM file.

gbemu uses the following keybindings:
- <kbd>A</kbd>: "A" button
- <kbd>S</kbd>: "B" button
- <kbd>Up</kbd>: "Up" button
- <kbd>Down</kbd>: "Down" button
- <kbd>Left</kbd>: "Left" button
- <kbd>Right</kbd>: "Right" button
- <kbd>Enter</kbd>: "Start" button
- <kbd>'</kbd>: "Select" button

## TODO
- Audio
- Saving data persistently
- Save states
