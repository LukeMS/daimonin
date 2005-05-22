#!/bin/sh
find -name "*.c" -or -name "*.h" -or -name "*.cpp" | xargs chmod 666
find -name "*.c" -or -name "*.h" -or -name "*.cpp" | xargs dos2unix 
