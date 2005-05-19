#!/bin/sh
find -name "*.c" -or -name "*.h" -or -name "*.cpp" | xargs dos2unix 
find -name "*.c" -or -name "*.h" -or -name "*.cpp" | xargs chmod 666