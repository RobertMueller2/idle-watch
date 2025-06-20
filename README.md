# 💤 `idle-watch` — Wayland Idle State Notifier

This is a small C program that uses the [`ext_idle_notifier_v1`](https://wayland.app/protocols/ext-idle-notify-unstable-v1) Wayland protocol to detect when the user's seat becomes idle or active again. It prints customizable messages on these events, optionally timestamped.

It was created for use in a [custom](https://github.com/RobertMueller2/Dotfile-Snippets/blob/94245797d45c52fb2eddd1fd3abdaa7800106727/ironbar/config.corn#L92) [ironbar](https://github.com/JakeStanger/ironbar) idle module.

The program structure was inspired by [grim](https://git.sr.ht/~emersion/grim/).

README was initially created by ChatGPT. Writing gives me writer's block. 

## Features

* Detects when the seat goes **idle** or **resumes activity**
* Supports **custom output** for each event
* Can print a **timestamp**
* Optional **initial output** for integration with status bars
* Configurable **idle timeout** (default: 1000ms)
* Clean Wayland event loop using `wl_display_dispatch`
* Uses standard Wayland protocols only

## Planned Features

* Include systemd inhibit
* Adjust for the event that the protocol is moved out of staging
* I actually do like Rust, feel more comfortable with it and would like to eventually rewrite this in Rust. That said, full abstractions via rusty wayland-client are an entirely different beast and I'm not sure I want to learn all that. I'm debating the amount of abstraction I want to use with myself. Perhaps I'll never do it.

## Build Instructions

You’ll need:

* Wayland development headers (`libwayland-client`)
* A C compiler (e.g., `gcc` or `clang`)
* Wayland-protocols containing `ext-idle-notify-v1` via `wayland-scanner`

Compile and install:

```sh
make
sudo make install
```

or override prefix to install elsewhere, e.g. without requiring root privileges:

```sh
make PREFIX=/opt/idle_watch install
```

## Usage

```sh
./idle_watch [options]
```

### Options

| Flag                     | Description                                                       |
| ------------------------ | ----------------------------------------------------------------- |
| `--timestamp`            | Print a timestamp before each output line                         |
| `--idle <string>`        | Output when seat goes idle (default: `"Seat has gone idle"`)      |
| `--resume <string>`      | Output when seat resumes activity (default: `"Activity resumed"`) |
| `--initial-output <str>` | Print this message immediately on startup                         |
| `--timeout <ms>`         | Idle timeout in milliseconds (default: `1000`)                    |
| `--help`                 | Print usage information                                           |

## Example

```sh
./idle_watch \
  --idle "🛌" \
  --resume "🏃" \
  --timestamp \
  --timeout 0 \
  --initial-output "🏃"
```

This produces:

```
[2025-10-04 13:45:00] 🏃
[2025-10-04 13:50:33] 🛌
[2025-10-04 13:50:35] 🏃
```
