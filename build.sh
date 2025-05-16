#!/bin/bash
g++ ./src/client/*.cpp  -o ./debug/client -lncurses -lenet
g++ ./src/server/*.cpp  -o ./debug/server -lncurses -lenet
