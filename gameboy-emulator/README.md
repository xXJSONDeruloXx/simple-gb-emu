# Game Boy Emulator

This is a simple and readable Game Boy emulator designed to run on multiple platforms with minimal dependencies. The emulator aims to provide a clear understanding of how the Game Boy hardware operates through straightforward code.

## Project Structure

```
gameboy-emulator
├── src
│   ├── main.c          # Entry point of the emulator
│   ├── cpu.c           # CPU emulation
│   ├── memory.c        # Memory management
│   ├── ppu.c           # Graphics rendering
│   ├── timer.c         # Timer functionality
│   ├── input.c         # User input handling
│   ├── cartridge.c      # ROM cartridge management
│   └── gameboy.h       # Common interface header
├── roms
│   └── .gitkeep        # Keeps the roms directory in version control
├── Makefile            # Build instructions
└── README.md           # Project documentation
```

## Requirements

- A C compiler (e.g., GCC)
- Make utility for building the project

## Building the Emulator

To build the emulator, navigate to the project directory and run:

```
make
```

This will compile the source files and create the emulator executable.

## Running the Emulator

After building, you can run the emulator with a Game Boy ROM file. Place your ROM files in the `roms` directory and execute the emulator with the ROM file as an argument.

## Contributing

Feel free to contribute to the project by submitting issues or pull requests. Your feedback and contributions are welcome!

## License

This project is open-source and available under the MIT License.