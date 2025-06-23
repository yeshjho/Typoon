[![ko](https://img.shields.io/badge/lang-ko-blue)](https://github.com/yeshjho/Typoon/blob/main/README.md)
[![en](https://img.shields.io/badge/lang-en-red)](https://github.com/yeshjho/Typoon/blob/main/readme/README-en.md)

# <img src="https://raw.githubusercontent.com/yeshjho/Typoon/main/icon.ico" width="40"> Typoon

![GitHub release (latest by date)](https://img.shields.io/github/v/release/yeshjho/Typoon)
![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/yeshjho/Typoon?include_prereleases)
![GitHub license](https://img.shields.io/github/license/yeshjho/Typoon)

![Cover Gif](/readme/cover.gif)

> A text expander that handles Hangeul, the Korean letters properly

**Typoon** is a text expander specifically designed to work with Hangeul, the Korean letter.
Most of the text expander out there doesn't really work well with Korean typing system, so I ended up making one myself.
Of course, it works with the alphabets, too.

It is heavily inspired by [espanso](https://espanso.org/), which is a magnificent text expander, and you'll probably want to use it instead of Typoon if you use alphabets only.

## How to Install
Go to the [Latest Release](https://github.com/yeshjho/Typoon/releases/latest) to get the installer.

## How to Use
Refer to the [wiki](https://github.com/yeshjho/Typoon/wiki/How-to-Use).

## Features
- Image replacement: You can replace a text to an image.

- Run command: You can replace a text with the output of a command.

- Per-program configuration: Can be automatically turned on/off or use different matches depending on the currently focused program.

- File watcher: Detects the change of the config file and the match file, and apply those changes automatically.

- Cursor position: You can set where the cursor would be after the replacement happens.

- JSON5: The config file and the match file use the [JSON5](https://json5.org/) format. This project uses the [C++ implementation](https://github.com/P-i-N/json5) of the format. For those who don't know what JSON5 is, it's a JSON but more human-friendly.

### Options
#### Options Specifically Designed For Hangeul
- Full Composite: Gets triggered only when the composition is complete.
- Keep Composite: Keep the last letter as being composed after replacement happens.

#### Options only for alphabets
- Case Sensitive
- Propagate Case
    - Capitalize per Word
    
#### Options for both
- Word: Gets triggered only when the text is separated to be a word.
- Kor/Eng Insensitive: Gets triggered even if you type a Korean trigger in English mode and vice versa.

For detailed explanations and examples of these options, refer to the [wiki](https://github.com/yeshjho/Typoon/wiki/%EC%82%AC%EC%9A%A9-%EB%B0%A9%EB%B2%95#%EC%98%B5%EC%85%98).

## Limitations
### Keyboard Layout
Currently, Typoon works only if the user uses the **two-set** layout for Hangeul and **QWERTY** layout for the alphabets.

### Platform
Currently, Typoon is only for Windows. But the codes are designed with cross-platform in mind, so adding supports for the other platforms wouldn't be that bad. That being said, I don't use the other platforms that much, so the priority is pretty low for now. Contributions are welcomed!

## Future Plan
Reference to the [Github Project](https://github.com/users/yeshjho/projects/2) for the planned features. However, since this is a hobby project and I'm still a student, I cannot give a roadmap with due dates set. Contributions are always welcomed!

## Special Thanks to
- Incheol Hwang, for creating & providing the logo, coming up with the name of the program, and helping with several design decisions.
