# Apple II Editor

A full-screen text/programmer's editor for the Apple II, originally written in **Aztec C and 6502 assembly language**, now being ported to the **cc65** toolchain.

## About the Project

This editor was originally developed on the Apple II, primarily using an Apple //c and later an Apple //c+. It was written as a working programmer's editor rather than as a demonstration program, and eventually became capable enough that it was used to edit its own source code.

The original program ran under the **Aztec C environment for ProDOS**. Aztec C provided a Unix-like command interpreter and C runtime on top of ProDOS. The editor could therefore be invoked from the command line with a filename, much as an editor would be launched on a Unix system.

The editor consists of a C-based full-screen editing environment built around a lower-level **6502 assembly-language line editor**. The assembly portion handles interactive editing of the current line, including cursor movement, insertion and overstrike, while passing commands that require action at the document level back to the C code.

The source survived on old Apple II disk images and has now been recovered and made buildable using modern tools.

## The Port

The initial goal of this project is deliberately conservative:

**Create a straightforward cc65 port of the original editor before attempting to modernize or redesign it.**

Where possible, the original source structure, algorithms, interfaces, and behavior are being preserved. Changes are made where required by differences between Aztec C and cc65, by the modern build environment, or to correct problems uncovered while bringing the program back to life.

This approach serves two purposes. First, it provides a working reference implementation that remains close to the original program. Second, it creates a known baseline from which later improvements can be made without losing track of how the original editor worked.

The current port targets an enhanced Apple II and is built with **cc65/ca65**.

## Current Status

The editor builds and runs under ProDOS and is capable of loading, editing, and saving text files.

The port is still a work in progress. Some parts of the original environment do not have direct equivalents under the standard cc65/ProDOS setup, and some functionality remains to be restored or adapted.

In its current form, the project is probably most useful as:

- an example of a non-trivial Apple II application written in a mixture of C and 6502 assembly;
- an example of porting historical Aztec C code to cc65;
- a reference for Apple II text-screen and keyboard handling;
- a demonstration of integrating assembly-language code with a larger C application; and
- a look at the implementation of the assembly-language line editor at the heart of the program.

The line editor in particular may be of interest independently of the complete application. It implements the low-level interactive editing behavior used by the full-screen editor and represents the core from which the larger program developed.

## Command-Line Limitation

One significant difference between the original Aztec C environment and the current cc65/ProDOS environment is **command-line argument handling**.

Under the original Aztec C command interpreter, the editor could simply be launched with a filename:

```text
ed source.c
```

The filename was supplied to the C program through the normal argument mechanism and the file was opened automatically.

ProDOS `BASIC.SYSTEM` does not provide an equivalent conventional command line for ordinary binary programs. A small ProDOS/BASIC.SYSTEM launcher or command extension is being considered so that a future version can once again support an invocation resembling:

```text
ED SOURCE.C
```

Meanwhile, the editor can be executed from BASIC.SYSTEM using the `-` command:

```text
-ED
```

To supply a filename on startup, cc65's Applesoft argument convention can be used. First load the editor, then call its entry point with the filename following a `REM` statement:

```text
BLOAD ED
CALL 4099:REM SOURCE.C
```

This is considerably more awkward than the original Aztec C command-line interface. The `BLOAD` should be performed before each `CALL`; the editor and its C runtime are not intended to be re-entered simply by calling a previously loaded copy again.

## Building

The project is built with the **cc65** development package and currently targets `apple2enh`.

The source tree separates the recovered/ported C and assembly-language components from build products and test programs. See the project's `Makefile` for the current source list, compiler options, linker configuration, and output settings.

## Historical Context

This project is not intended to be a new editor merely inspired by an old Apple II program. It is an effort to recover and preserve the original program by moving it from its original Aztec C toolchain to one that remains readily available today.

For that reason, improvements that might make the code cleaner or more modern are intentionally secondary during the initial port. Once the original functionality has been reproduced and validated, subsequent development can explore improvements without obscuring the historical implementation.

## Supported Key Commands

### Editor Commands

| Key | Command |
| --- | --- |
| **Open-Apple-D** | Delete current line |
| **Open-Apple-F** | Find/Search |
| **Open-Apple-R** | Replace |
| **Open-Apple-S** | Save |
| **Open-Apple-L** | Load |
| **Open-Apple-W** | Set window divider |
| **Open-Apple-J** | Jump to other viewport |
| **Open-Apple-G** | Goto line |
| **Open-Apple-C** | Copy |
| **Open-Apple-P** or **Open-Apple-V** | Paste |
| **Open-Apple-X** | Cut |
| **Open-Apple-I** | File information |
| **Open-Apple + Down Arrow** | Page down |
| **Open-Apple + Up Arrow** | Page up |
| **Open-Apple-Q** | Quit |

### Line Editor Commands

These commands are handled by the assembly-language line editor:

| Key | Command |
| --- | --- |
| **Left / Right Arrow** | Move within the current line |
| **Up / Down Arrow** | Move between source lines |
| **Return** | Insert a new line |
| **Delete** or **Control-D** | Delete character |
| **Tab** | Tab |
| **Escape** | Cancel current line edit |
| **Control-E** or **Open-Apple-E** | Toggle insert/overstrike mode |
| **Control-Y**, **Open-Apple-T**, or **Open-Apple-Y** | Truncate line at cursor |

### Copy and Paste

The current copy/paste workflow is:

1. Press **Open-Apple-C** to enter/select copy mode.
2. Use **Down Arrow** to extend the selected range. Selected lines are highlighted as the range is extended.
3. Use **Up Arrow** to reduce the selected range if necessary.
4. Press **Open-Apple-C** again to complete the copy operation.
5. Move to the destination. **Open-Apple-J** can be used to jump to the other viewport.
6. Press **Open-Apple-P** or **Open-Apple-V** to paste.
