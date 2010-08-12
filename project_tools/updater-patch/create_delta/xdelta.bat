xdelta.exe -?
rem this line will patch the daimonin.p0 - alter it for different files
xdelta.exe encode -9 -s old/daimonin.p0 new/daimonin.p0 daimonin.p0.diff
pause
