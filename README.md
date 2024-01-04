# Custom HTTP Server in C++

## Introduction
This project presents a custom-built HTTP server, developed in compliance with the key principles of the HTTP protocol. Designed and implemented in C++, this server demonstrates a deep understanding of web communication protocols and their implementation.

## Key Features
- **Robust Handling**: The server is built to withstand various operational scenarios without crashing or unexpected quits, even under memory constraints.
- **C++ Standards Compliance**: Adheres strictly to the C++ 98 standard and makes use of modern C++ features where possible.
- **Configuration Flexibility**: Supports custom configuration files, allowing for versatile deployment setups.
- **Non-Blocking Architecture**: Utilizes `poll()` for efficient, non-blocking I/O operations, ensuring the server remains responsive under load.
- **Browser Compatibility**: Designed to be compatible with modern web browsers, aligning with NGINX for HTTP 1.1 header and behavior standards.
- **Error Handling**: Implements accurate HTTP response status codes and provides default error pages.

## Getting Started

### Prerequisites
- A C++ compiler that supports the C++ 98 standard.
- GNU Make for building the project.
- A Linux environment for best compatibility.

### Installation
1. Clone the repository:
```
git clone [URL to repository]https://github.com/Dragoshiz/HTTP-Server/
```
3. Navigate to project directory:
```
cd [project-directory]
```
3. Compile the server using the Makefile:
```
make
```
## Running the Server
1. To start the server, run:
```
./webserv [path-to-configuration-file]
```

## Usage Examples
- Serve a static website by pointing the server to a directory containing HTML.
- Test server response using different HTTP methods through a web browser or tools like `curl`.

## Acknowledgments
Special thanks to my partners for this project:
- [Trung](https://github.com/Alohakaloha)
- [Valentin](https://github.com/minthe)
