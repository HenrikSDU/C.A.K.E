#include <windows.h>
#include <tchar.h>
#include <stdio.h>


void PrintCommState(DCB dcb)
{
    //  Print some of the DCB structure values
    _tprintf( TEXT("\nBaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\n"), 
              dcb.BaudRate, 
              dcb.ByteSize, 
              dcb.Parity,
              dcb.StopBits );
}


int _tmain( int argc, TCHAR *argv[] ){
   
   COMMTIMEOUTS timeouts = {0};
   DWORD BytesWritten = 0;          // No of bytes written to the port
   DWORD NoBytesRead;     // Bytes read by ReadFile()
   DWORD dwEventMask;     // Event mask to trigger
   
   BOOL writesuccess, readsuccess, comreistatus;
   char data_to_send[20];
   unsigned char memory_init_flags[10];
   unsigned char memory_init_feedback[10];
   char data_to_read[20];
   char* filebuffer;
   
   char input;
   
   char  ReadData;        //temperory Character

   unsigned char i = 0;
   int filelength;

   //initialize
   DCB dcb;
   HANDLE hCom;
   BOOL fSuccess;
   TCHAR *pcCommPort = TEXT("COM3"); //  Most systems have a COM1 port

   //  Open a handle to the specified com port.
   hCom = CreateFile( pcCommPort,
                      GENERIC_READ | GENERIC_WRITE,
                      0,      //  must be opened with exclusive-access
                      NULL,   //  default security attributes
                      OPEN_EXISTING, //  must use OPEN_EXISTING
                      0,      //  not overlapped I/O
                      NULL ); //  hTemplate must be NULL for comm devices

   if (hCom == INVALID_HANDLE_VALUE){
       printf ("CreateFile failed with error %d.\n", GetLastError());
       return (1);
   }

   //Initialize the DCB structure.
   SecureZeroMemory(&dcb, sizeof(DCB));
   dcb.DCBlength = sizeof(DCB);

   
   fSuccess = GetCommState(hCom, &dcb);

   if (!fSuccess){
      printf ("GetCommState failed with error %d.\n", GetLastError());
      return (2);
   }

   PrintCommState(dcb);       //  Output to console

   //  Fill in some DCB values and set the com state: 
   //  9600 bps, 8 data bits, no parity, and 1 stop bit.
   dcb.BaudRate = CBR_9600;     //  baud rate
   dcb.ByteSize = 8;             //  data size, xmit and rcv
   dcb.Parity   = NOPARITY;      //  parity bit
   dcb.StopBits = ONESTOPBIT;    //  stop bit

   fSuccess = SetCommState(hCom, &dcb);

   if (!fSuccess){
      printf ("SetCommState failed with error %d.\n", GetLastError());
      return (3);
   }

   //  Get the comm config again.
   fSuccess = GetCommState(hCom, &dcb);

   if (!fSuccess){
      printf ("GetCommState failed with error %d.\n", GetLastError());
      return (2);
   }

   PrintCommState(dcb);       //  Output to console

   _tprintf (TEXT("Serial port %s successfully reconfigured.\n"), pcCommPort);

   //set timeouts
   timeouts.ReadIntervalTimeout = 50;
   timeouts.ReadTotalTimeoutConstant = 50;
   timeouts.ReadTotalTimeoutMultiplier = 10;
   timeouts.WriteTotalTimeoutConstant = 50;
   timeouts.WriteTotalTimeoutMultiplier = 10;

   if (SetCommTimeouts(hCom, &timeouts) == FALSE){

        printf_s("\nError to Setting Timeouts");
        CloseHandle(hCom);//Closing the Serial Port

    }

  //open file
  FILE *file = fopen("testfile.cake","r");
  if(file == NULL){
      printf("\nfopen failed");
      return 7;
  } 
  //getting file length

   fseek(file, 0L, SEEK_END);
   filelength = ftell(file);
   printf("\nFilelength: %d\n",filelength);
   rewind(file);//reseting cursor
   //fclose(file); //close file

   printf("\nSettingInitflags\n");

   //filling the array
   for(int r=0;r<10;r++){
      memory_init_flags[r]=r;
      printf("%d ", memory_init_flags[r]);
   }

   //putting filesize
   memory_init_flags[0]=(unsigned char)filelength;
   printf("\nDetermined filelength: %d \n",filelength);
   printf("Updated memoryinitflags:\n");
   for(int r=0;r<10;r++){
      printf("%d ", memory_init_flags[r]);
   }

   do{

      printf("\nInput: ");
      scanf("%d",&input);
      if (input==0){
            CloseHandle(hCom);
            fclose(file);
            printf("\nCancel Of Program");
         return 0;
      }
      
      comreistatus = SetCommMask(hCom, EV_RXCHAR);
      if(comreistatus == FALSE){
            CloseHandle(hCom);
            printf("Error in setting commask");
         return 5;
      }
      else
      printf("\nSuccessfully set Commask\n");
      
      //write to port
      printf("\nStart Writing to USB-Port\n");
      writesuccess = WriteFile(hCom,// Handle to the Serialport
                        memory_init_flags,            // Data to be written to the port
                        sizeof(memory_init_flags),   // No of bytes to write into the port
                        &BytesWritten,  // No of bytes written to the port
                        NULL);
                        

      if(writesuccess == FALSE){
            printf_s("\nFailed to write");
            fclose(file);
         return 4;
      }

      printf("\nNumber of bytes written to the serial port = %d\n", BytesWritten);

      await_file:
      comreistatus = WaitCommEvent(hCom, &dwEventMask, NULL); //Wait for the character to be received
      if(!comreistatus){
         printf("Error setting WaitCommEvent");
      }
      else
         printf("waited");
      
      i=0;
      
         do{
               comreistatus = ReadFile(hCom, &ReadData, sizeof(ReadData),&NoBytesRead, NULL);
               if(NoBytesRead!=0){//as long as byte
                  memory_init_feedback[i] = ReadData;
                  printf("\nfeddval%d:%d\n", i, memory_init_feedback[i]);
                  //printf("\nNumber of bytes read:%d\n",NoBytesRead);
               }
               i++;
         }while(NoBytesRead>0);
      
      
      printf("\n Bytes returned by our microcontroller:  \n");

      for(int q=0;q<10;q++){
         printf("%d ",memory_init_feedback[q]);
         if(memory_init_feedback[q]!=memory_init_flags[q]){
            printf("\n!Element %d is unequal: flags: %d and feedback %d\n",q,memory_init_flags[q],memory_init_feedback[q]);
         }
      }

      printf("\nReturned filelength: %u\n",memory_init_feedback[0]);
      if(memory_init_feedback[0]==filelength){
         printf("\nSuccessfull filelength transmit\n");
      }
      else{
         printf("\nError in filelength transmission\n");
      }

      
      printf("\nStart of File Transmission");
      int filesendbuffer_index=0, arraysize;
      unsigned char* filesendbuffer;
      filesendbuffer = (char*)malloc(filelength*sizeof(unsigned char));
      if(filesendbuffer!=NULL){
         arraysize=sizeof(filesendbuffer)/sizeof(unsigned char);
         printf("\nSuccessfully allocated memory for sendbuffer, %d bytes\n", sizeof(filesendbuffer));
      }
      int filereceivebuffer_index=0;
      unsigned char* filereceivebuffer;
      filereceivebuffer = (char*)malloc(filelength*sizeof(unsigned char));
      if(filereceivebuffer!=NULL){
         arraysize=sizeof(filereceivebuffer)/sizeof(unsigned char);
         printf("\nSuccessfully allocated memory for receivebuffer, %d bytes\n", sizeof(filereceivebuffer));
      }

      unsigned char character;
      while (filesendbuffer_index < filelength){ // Read the file byte by byte 
         character = fgetc(file);
         filesendbuffer[filesendbuffer_index]=character;
         printf("%c",filesendbuffer[filesendbuffer_index]);
         filesendbuffer_index++;
      }
      printf("\n");
      printf("\nStart Transmit of File\n");
      writesuccess = WriteFile(hCom,// Handle to the Serialport
                        filesendbuffer,            // Data to be written to the port
                        filelength,   // No of bytes to write into the port
                        &BytesWritten,  // No of bytes written to the port
                        NULL);
      if(writesuccess){
         printf("Write Success\n");
      }
      else
         printf("Write Error!\n");
      
      //Reading Feedback
      //scanf("%d",&input);
      comreistatus = WaitCommEvent(hCom, &dwEventMask, NULL); //Wait for the character to be received
      if(!comreistatus){
         printf("Error setting WaitCommEvent");
      }
      else
         printf("waited\n");

      while(!(dwEventMask&EV_RXCHAR)) printf("debug");
      //scanf("%d",&input);
      i=0;
      if(dwEventMask & EV_RXCHAR){
         printf("\nCharacter in recieve buffer!!!");
         do{
               //printf(" (LoIter: %u) ",i);
               comreistatus = ReadFile(hCom, &ReadData, sizeof(ReadData),&NoBytesRead, NULL);
               if(NoBytesRead!=0){
                  filereceivebuffer[i] = ReadData;
                  printf("%c", filereceivebuffer[i]);
                  //printf("\nNumber of bytes read:%d\n",NoBytesRead);
               }
               i++;
         }while(NoBytesRead>0);
      }
      else{
         printf("\nNothing waited for!!!");
         goto await_file;
      }
      free(filesendbuffer);
      free(filereceivebuffer);
   }while(input != 0);
   
   fclose(file); //closing file again
   printf("End of Program");
   return 0;
   }