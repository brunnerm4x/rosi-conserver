# ROSI - Conserver
## Backup Server for ROSI-Clients

### Description
Small, simple server that saves encrypted (by the client) backups to disc and returns them when needed.
Written in C++ with Qt Network Library.

### Dependencies
* Qt (qmake)

### Compilation / Installation
1. `git clone https://github.com/brunnerm4x/rosi-conserver.git`
2. `cd rosi-conserver`
3. `export qmake=\path\to\qmake`
4. `chmod +x build.sh && ./build.sh`
5. `cd bin/ && ./rosiConserver`

Alternatively the rosiConserver.pro file can be opened with QtCreator and built with help of this tool.

### Configuration

* default port: 12000

To change configuration, edit the source. Port can be changed in server.cpp.

