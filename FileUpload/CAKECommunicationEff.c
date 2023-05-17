#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <math.h>
#include <time.h>


void PrintCommState(DCB dcb)
{
    //  Print some of the DCB structure values
    _tprintf( TEXT("\nBaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\n"), 
              dcb.BaudRate, 
              dcb.ByteSize, 
              dcb.Parity,
              dcb.StopBits );
}


int main(int argc, char *argv[]){

   printf("%d",argc); //printing the argument count
   printf("%s",argv[1]); //printing the arguments
   printf("%s",argv[2]); 

   COMMTIMEOUTS timeouts = {0};
   DWORD BytesWritten = 0;          // No of bytes written to the port
   DWORD NoBytesRead;     // Bytes read by ReadFile()
   DWORD dwEventMask;     // Event mask to trigger
   
   BOOL writesuccess, readsuccess, comreistatus;
   unsigned char memory_init_flags[10];
   unsigned char memory_init_feedback[10];
   char* filebuffer;

   
   char input;
   
   char  ReadData;        //temperory Character

   unsigned char i = 0;
   int filelength;

   //initialize
   DCB dcb;
   HANDLE hCom;
   BOOL fSuccess;
   char portname[] = "COM3";  //for debugging

   hCom = CreateFile( argv[1],
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

   _tprintf (TEXT("Serial port %s successfully reconfigured.\n"), portname);

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
  FILE *file = fopen(argv[2], "r");
  if(file == NULL){
      printf("\nfopen failed");
      CloseHandle(hCom);
      return 7;
  } 
  //getting file length

   fseek(file, 0L, SEEK_END);
   filelength = ftell(file);
   printf("\nFilelength: %d\n", filelength);
   rewind(file);//reseting cursor
   //fclose(file); //close file

   printf("\nSettingInitflags\n");

   //filling the array
   for(int r=0;r<10;r++){
      memory_init_flags[r]=0;
      printf("%d ", memory_init_flags[r]);
   }

   //putting filesize
   double required_capacity;
   unsigned char remaining_bytes;
   required_capacity = (double)filelength/(double)255; //determining how many bytes are needed to transmit filelength
   
   //calculating required capacity
   required_capacity = floor(required_capacity); //round up to the next full integer
   remaining_bytes = filelength - (required_capacity*255);

   //determine how many instructions are saved
   int instruction_count = 1;
   char temp;
   while((temp = fgetc(file)) != EOF){

      if(temp == '\n') //if new-line character gets recognized increment the instruction counter
         instruction_count++;

   }
   rewind(file);
   printf("\n%d instructions found!", instruction_count);

   memory_init_flags[0] = (unsigned char)required_capacity;
   memory_init_flags[1] = remaining_bytes;
   memory_init_flags[2] = instruction_count;
   
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
      if((memory_init_feedback[0]==required_capacity) && (memory_init_feedback[1]==remaining_bytes)){
         printf("\nSuccessfull filelength transmit\n");
      }
      else{
         printf("\nError in filelength transmission\n");
      }

      
      printf("\nStart of File Transmission");


      int filesendbuffer_index = 0, arraysize;
      
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

      /////////////////////////////////////////////////////////
      char instructions_to_send[32];
      int instruction_load_counter = 0;
      int instruction_count = 0;
      int file_reader = 0;
      int instruction_load_index = 0;
      rewind(file);
      while(file_reader < filelength){

         instruction_load_index = 0;
         SecureZeroMemory(instructions_to_send, sizeof(char)*32);

         for(instruction_load_counter = 0; instruction_load_counter < 4; file_reader++){

            instructions_to_send[instruction_load_index] = fgetc(file); 

            printf("%c", instructions_to_send[instruction_load_index]);
            instruction_load_index++;


            if(instructions_to_send[instruction_load_index] == '\n'){
               instruction_load_counter++;
            }

         }

         writesuccess = WriteFile(hCom,// Handle to the Serialport
                        instructions_to_send,            // Data to be written to the port
                        sizeof(char)*32,   // No of bytes to write into the port
                        &BytesWritten,  // No of bytes written to the port
                        NULL);
         
         if(writesuccess){
            printf("\nWritesuccess");
         }               

         Sleep(100);

      comreistatus = WaitCommEvent(hCom, &dwEventMask, NULL); //Wait for the character to be received
      if(!comreistatus){
         printf("Error setting WaitCommEvent");
      }
      else
         printf("waited\n");

      
      //scanf("%d",&input);
      i=0;
      
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
      /*
      printf("Purging Com");
      BOOL purgesuccess = PurgeComm(hCom, PURGE_RXCLEAR);
      if(purgesuccess)
      printf("Surccessfully purged comm");

      printf("\nStart Transmit of File\n");
      writesuccess = WriteFile(hCom,// Handle to the Serialport
                        filesendbuffer,            // Data to be written to the port
                        filelength,   // No of bytes to write into the port
                        &BytesWritten,  // No of bytes written to the port
                        NULL);
      if(writesuccess){
         printf("Write Success - %d bytes written!\n", BytesWritten);
      }
      else
         printf("Write Error!\n");
      
      //Reading Feedback
      //scanf("%d",&input);
      Sleep(500);
      comreistatus = WaitCommEvent(hCom, &dwEventMask, NULL); //Wait for the character to be received
      if(!comreistatus){
         printf("Error setting WaitCommEvent");
      }
      else
         printf("waited\n");

      
      //scanf("%d",&input);
      i=0;
      
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
      */
      
      free(filesendbuffer);
      free(filereceivebuffer);
   }while(input != 0);
   
   fclose(file); //closing file again
   printf("End of Program");
   return 0;
   }