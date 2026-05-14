# CHTTP

A lightweight, cross-platform HTTP server written in C23. 

CHTTP is designed to be a minimal and dependency-free web server for serving static files. It features built-in custom logging, robust configuration parsing (`.env` or `.cfg`), and native socket implementations for both Windows and Linux environments.

## Features

* **Cross-Platform**: Compiles and runs natively on Linux (POSIX sockets) and Windows (Winsock2).
* **Flexible Configuration**: Configure the server host and port via Environment variables or a custom Configuration file.
* **Custom Logging**: Includes a thread-safe custom logger with severity levels, stack tracing, and file output.
* **Modern C**: Written targeting the C23 standard.

## Prerequisites

* CMake (Version 3.10 or higher)
* A modern C compiler supporting C23 (GCC, Clang, or MSVC)

## Build Instructions

1. **Clone the repository:**
```bash
git clone https://github.com/monakaibrahim-cmyk/simple-chttp.git chttp
cd chttp
```

Build the project

```bash
cmake -B build
cmake --build build
```

## 📁 Project Structure
```
www/
---/index.html
```
