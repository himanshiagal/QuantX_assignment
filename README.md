# High-Performance Market Data Processor

This project processes and merges market trading data from multiple files based on timestamp, producing a single sorted output. It includes an asynchronous logger implemented using a lock-free concurrent queue.

---

## Project Structure
```js
/project_root
├── /src
│   ├── main.cpp # Entry point: handles config loading, data processing
│   ├── Logger.cpp # Asynchronous logger implementation
│   └── configLoader.cpp # Loads key-value pairs from config.cfg
├── /include
│   ├── Logger.h
│   └── configLoader.h
├── /external
│   └── /concurrentqueue
│       └── concurrentqueue.h # Third-party lock-free queue header
├── CMakeLists.txt
└── /resources
    └── config.cfg
```


## Description

This project processes multiple market data files in a time-sorted fashion. Each file represents a tradable script with trading data. The goal is to:

- **Efficiently** read from all input files
- **Synchronize** and **merge** records based on timestamp
- Write merged, time-ordered records into an `output.txt` file
- Use a **concurrent logger** running in a separate thread for logging status messages

---

## Configuration File Format (`resources/config.cfg`)

Before running the executable, provide the following fields in your config file:

session_start=20250104093000
session_end=20250104160000
input_folder=C:/Users/Dell/Downloads/test/input
output_file=C:/Users/Dell/Downloads/test/output.txt
buffer_size=1048576
file_prefix=SPX

## Build Instructions
Install Dependencies (via vcpkg)

Ensure vcpkg is installed and properly set up

Clone concurrentqueue

It's included under /external/concurrentqueue

Configure CMake
Update your CMakeLists.txt to reflect the correct path to vcpkg:
set(CMAKE_TOOLCHAIN_FILE "C:/Users/Dell/vcpkg/scripts/buildsystems/vcpkg.cmake")


## under external folder concurrent queue library clone
git clone https://github.com/cameron314/concurrentqueue.git 


## Build Steps

mkdir build
cd build
cmake ..

## Run Instructions
After building:
.\Debug\demo.exe C:\Users\Dell\Downloads\test\resources\config.cfg

## Input File Format (inside /input/)
Each .txt file is structured like:

Ticker,Date,Time,LTP,BuyPrice,BuyQty,SellPrice,SellQty,LTQ,OpenInterest
SPXW20250401C05465000,01/04/2025,09:30:04,129.35,126.5,26.0,132.2,26.0,1.0,7950.0


## Output File Format (output.txt)
The generated output.txt will contain:

ScriptName,TimeStamp,Bid,Ask,Bid_Quantity,Ask_Quantity,LTP
SPX,20250401093004,5420,5440,100,80,5432

All prices are in paise (e.g., 5432 means ₹54.32)
TimeStamp is in the format yyyymmddhhmmss (e.g., 20250401093000 for 9:30 AM on 1 April, 2025)
