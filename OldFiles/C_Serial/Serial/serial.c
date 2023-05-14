#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <string.h>

HANDLE hSerial;
int main()
{
// OPEN SERIAL PORT AND SET INITAL UART PARAMETERS 
//=================================================
DCB dcbSerialParams;
COMMTIMEOUTS timeouts;
fprintf(stderr, "Opening serial port...");
hSerial = CreateFile("\\\\.\\COM3", GENERIC_READ|GENERIC_WRITE, 0, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

if (hSerial == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Error\n");
    return 1;
}
else {
    fprintf(stderr, "OK\n");
}

// Set device parameters (115200 baud, 1 start bit, 1 stop bit, no parity)
dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
if (GetCommState(hSerial, &dcbSerialParams) == 0) {
    fprintf(stderr, "Error getting device state\n");
    CloseHandle(hSerial);
    return 1;
}

dcbSerialParams.BaudRate = CBR_9600; 
dcbSerialParams.ByteSize = 8; 
dcbSerialParams.StopBits = ONESTOPBIT; 
dcbSerialParams.Parity = NOPARITY;

if(SetCommState(hSerial, &dcbSerialParams) == 0) {
    fprintf(stderr, "Error setting device parameters\n");
    CloseHandle(hSerial);
    return 1;
}

// Set COM port timeout settings
timeouts.ReadIntervalTimeout = 50; 
timeouts.ReadTotalTimeoutConstant = 50; 
timeouts.ReadTotalTimeoutMultiplier = 10;
timeouts.WriteTotalTimeoutConstant = 50; 
timeouts.WriteTotalTimeoutMultiplier = 10;

if(SetCommTimeouts(hSerial, &timeouts) == 0) { 
    fprintf(stderr, "Error setting timeouts\n");
    CloseHandle(hSerial);
    return 1;
}



// SETUP AND SEND DATA FROM UART
//============================== 
    int VarNum=8;
    char str[15];
    sprintf(str,"%ld",VarNum);
    LPCVOID send = 5;


    DWORD bytes_written, total_bytes_written = 0;
    fprintf(stderr, "Sending bytes...");
    if(!WriteFile(hSerial,&send, strlen(str), &bytes_written, NULL))
    {
        fprintf(stderr, "Error\n");
        CloseHandle(hSerial);
        return 1;
    }   
    fprintf(stderr, "%d bytes written\n", bytes_written); 
    /*
    unsigned char rx;
    if(!ReadFile(hSerial, &rx, 1, &bytes_written, NULL)) {
        fprintf(stderr, "Error\n");
        CloseHandle(hSerial);
        return 1;
    }
    fprintf(stdout, "%d\n", rx);
    */


// CLOSE SERIAL PORT AND EXIT MAIN FUNCTION
//=========================================
fprintf(stderr, "Closing serial port...");

if (CloseHandle(hSerial) == 0) {
    fprintf(stderr, "Error\n"); return 1;
}
fprintf(stderr, "OK\n");
return 0;

}