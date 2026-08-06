# RMDB

<div align="center">
<img src="./docs/RMDB.jpg"  width=25%  /> 
</div>

A teaching-oriented relational database project built on the RMDB framework.  
It includes SQL parsing, query optimization, execution, storage management, transaction concurrency control, and recovery.

## Requirements

- Linux/macOS
- CMake >= 3.16
- GCC/Clang (C++17 for server, C++20 for client)
- `readline`
- `pthread`

For Linux(e.g. Ubuntu) users, you can install dependencies with:

```bash
sudo apt-get install build-essential # build-essential packages, including gcc, g++, make and so on
sudo apt-get install cmake # cmake package
sudo apt-get install flex bison # flex & bison packages
sudo apt-get install libreadline-dev # readline package
```

For macOS users, you can install dependencies with Homebrew:

```bash
brew install llvm # llvm package, including clang and clang++
brew install cmake # cmake package
brew install flex bison # flex & bison packages
brew install readline # readline package
```

## Project Structure

- `src/`: core database server source code (builds `rmdb`)
- `rmdb_client/`: interactive client source code (builds `rmdb_client`)
- `test/`: SQL test scripts and automation scripts
- `docs/`: design documents and competition-related materials
- `deps/`: third-party dependencies (including googletest)

## Build

### 1) Build the server

```bash
mkdir build
cd build
cmake .. [-DCMAKE_BUILD_TYPE=Debug]|[-DCMAKE_BUILD_TYPE=Release] # [] indicates optional, Debug mode includes debug symbols and assertions, while Release mode is optimized for performance
make rmdb <-j4>|<-j8>
```

Server binary: `build/bin/rmdb`

### 2) Build the client

```bash
cd rmdb_client
mkdir build
cd build
cmake .. [-DCMAKE_BUILD_TYPE=Debug]|[-DCMAKE_BUILD_TYPE=Release]
make rmdb_client <-j4>|<-j8>
```

Client binary: `rmdb_client/build/rmdb_client`

## Run

### Start the database server

```bash
./build/bin/rmdb <database_name>
```

Example:

```bash
./build/bin/rmdb test_db
```

The server listens on `127.0.0.1:8765` by default.

### Connect with the client

```bash
./rmdb_client/build/rmdb_client
```

Optional arguments:

- `-h <host>`: server host (default: `127.0.0.1`)
- `-p <port>`: server port (default: `8765`)
- `-s <unix_socket_path>`: connect via Unix Domain Socket

## Testing

- SQL scripts: `test/*.sql`
- Automated execution script: `test/test_db.py`
- Crash recovery test script: `test/auto_crash_test.sh`
- Unit test binary (after build): `build/bin/unit_test`

## Documentation

- [Project Overview](docs/README%20copy.md)
- [RMDB User Guide](docs/RMDB使用文档.pdf)
- [RMDB Environment Setup Guide](docs/RMDB环境配置文档.pdf)
- [RMDB Project Structure](docs/RMDB项目结构.pdf)
- [Architecture Diagram](docs/框架图.pdf)

## License

This project is licensed under Mulan PSL v2.  
See `License` for details.
