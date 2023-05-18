@echo off
echo Hello, World!
mode COM4 BAUD=9600 PARITY=n DATA=8
COPY COM4 data.txt
pause