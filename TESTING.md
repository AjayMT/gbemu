
# gbemu manual tests

To compile `gbemu`, install [CSFML](https://www.sfml-dev.org/download/csfml/) and run:
```
make
```

This will produce a `gbemu` executable.

## 1. Command-line arguments
When running `gbemu` without command-line arguments, it should print an error message as follows and exit:
```
$ gbemu
Usage: gbemu <filename>
```

When a GameBoy ROM file is provided as the first command-line argument, `gbemu` should create a window and fill it with a light green color:
![](https://gitlab.engr.illinois.edu/ajaymt2/fa21-cs242-project/-/raw/week-3/images/empty.png)

## 2. Boot up
When booting up, the window should display a dark green Nintendo logo that scrolls down from the top of the screen:
![](https://gitlab.engr.illinois.edu/ajaymt2/fa21-cs242-project/-/raw/week-3/images/nintendo.png)

This Nintendo logo should disappear after scrolling to the middle of the screen.

## 3. Displaying CPU tests
`test/cpu_instrs.gb` is a CPU instruction testing ROM. To run it:
```
gbemu test/cpu_instrs.gb
```

After booting up, `gbemu` should run the CPU instruction tests and the screen should look as follows, indicating that all tests have passed:
![](https://gitlab.engr.illinois.edu/ajaymt2/fa21-cs242-project/-/raw/week-3/images/cpu.png)

## 4. Keyboard input
To test keyboard input, run the `test/tetris.gb` ROM:
```
gbemu test/tetris.gb
```

This should start tetris:
![](https://gitlab.engr.illinois.edu/ajaymt2/fa21-cs242-project/-/raw/week-3/images/tetris.png)

The game should respond to keyboard input and should be playable with the following keybindings:
- <kbd>A</kbd>: "A" button
- <kbd>S</kbd>: "B" button
- <kbd>Up</kbd>: "Up" button
- <kbd>Down</kbd>: "Down" button
- <kbd>Left</kbd>: "Left" button
- <kbd>Right</kbd>: "Right" button
- <kbd>Enter</kbd>: "Start" button
- <kbd>'</kbd>: "Select" button

![](https://gitlab.engr.illinois.edu/ajaymt2/fa21-cs242-project/-/raw/week-3/images/tetris2.png)
