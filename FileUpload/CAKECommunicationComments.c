#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <math.h>
#include <time.h>


void PrintCommState(DCB dcb)
{
    // Print some of the DCB structure values
    _tprintf( TEXT("\nBaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d\n"), 
              dcb.BaudRate, 
              dcb.ByteSize, 
              dcb.Parity,
              dcb.StopBits );
}


int main(int argc, char *argv[]){

   printf("%d\n",argc); // Printing the argument count
   // Printing the arguments
   printf("%s\n",argv[1]); // COM name
   printf("%s\n",argv[2]); // Filepath

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

   // Initialize HANDLE, DCB and BOOL
   // Further explanation will follow. 
   DCB dcb; // DCB struct holds information about the configuration of the communication device
   HANDLE hCom; // In program reference to the usb port
   BOOL fSuccess;
   

   // The CreateFile function is a function provided by the windows.h header file.
   // When working with the WIN32 api a handle is a reference to a resource of the operating system
   // This can be a file, a window or in this case an USB-port
   // Under Windows the ports have names like COM1, COM2, COM3 etc.
   // To see which port the microcontroller is connected to you can check under "Device Manager".
   // This name is passed into the main function when calling the executable and stored in the
   // argv array.
   // The further arguments are chosen according to the Microsoft documentation:
   // https://learn.microsoft.com/en-us/windows/win32/devio/monitoring-communications-events
   hCom = CreateFile( argv[1], // name of the port
                      GENERIC_READ | GENERIC_WRITE, 
                      0,      //  must be opened with exclusive-access
                      NULL,   //  default security attributes
                      OPEN_EXISTING, //  must use OPEN_EXISTING
                      0,      //  not overlapped I/O
                      NULL ); //  hTemplate must be NULL for comm devices

   if(hCom == INVALID_HANDLE_VALUE) {
       printf ("CreateFile failed with error %d.\n", GetLastError());
       return (1);
   }

   // hCom is this programs reference to the specified port.
   // This HANDLE will be used in order to read from and write to the port.

   // Initialize the DCB structure.
   // The DCB (Device Control Block) structure is another custom data type of windows.
   // This structure holds information about the BAUD-rate, the number of stop bits and parity bits.
   // SecureZeroMemory will make sure that there is no old data saved at the address of the 
   // DCB structure.
   // The micosoft documentation for this and the further steps of initialization:
   // https://learn.microsoft.com/en-us/windows/win32/devio/configuring-a-communications-resource
   SecureZeroMemory(&dcb, sizeof(DCB));
   dcb.DCBlength = sizeof(DCB);

   // The GetCommState function - also provided by the windows.h header file loads the current configuration.
   // of the port in the DCB struct   
   fSuccess = GetCommState(hCom, &dcb);

   if(!fSuccess) {
      printf ("GetCommState failed with error %d.\n", GetLastError());
      return (2);
   }

   PrintCommState(dcb);       // Print the values to console

   // Fill in some DCB values and set the com state: 
   // 9600 bps, 8 data bits, no parity, and 1 stop bit.
   dcb.BaudRate = CBR_9600;     //  baud rate
   dcb.ByteSize = 8;             //  data size, xmit and rcv
   dcb.Parity   = NOPARITY;      //  parity bit
   dcb.StopBits = ONESTOPBIT;    //  stop bit

   // With this function the USB-port gets configured according to the DCB structure
   fSuccess = SetCommState(hCom, &dcb);

   if(!fSuccess) {
      printf ("SetCommState failed with error %d.\n", GetLastError());
      return (3);
   }

   // Get the port configuration again.
   fSuccess = GetCommState(hCom, &dcb);

   if(!fSuccess) {
      printf ("GetCommState failed with error %d.\n", GetLastError());
      return (2);
   }

   PrintCommState(dcb);       // Print the values to console to check correct configuration.

   _tprintf (TEXT("Serial port %s successfully reconfigured.\n"), argv[1]);

   // Set timeouts for read and write opertions for the port - also saved in a custom struct of windows
   timeouts.ReadIntervalTimeout = 50;
   timeouts.ReadTotalTimeoutConstant = 50;
   timeouts.ReadTotalTimeoutMultiplier = 10;
   timeouts.WriteTotalTimeoutConstant = 50;
   timeouts.WriteTotalTimeoutMultiplier = 10;

   if(SetCommTimeouts(hCom, &timeouts) == FALSE) {

        printf_s("\nError to Setting Timeouts");
        CloseHandle(hCom);//Closing the Serial Port

    }

   
   // At first an array is send to the microcontroller about the the incoming file
   // Originally thought for allocating memory dynamically - however, as the filesize is to big
   // dynamic allocation fails. Nevertheless, the send information seves the file processing 
   // functions as well as the motor control on the microcontroller.
   // For example, knowing how many instructions need to be executed.
   printf("\nSettingMemoryInitflags\n");

   // Filling the array initially with zeros only.
   for(int r=0;r<10;r++) {
      memory_init_flags[r]=0;
      printf("%d ", memory_init_flags[r]);
   }

   // Getting the information about the file:

   // Opening the file specified by the arguments of main - the path to the file that is desired to be sent
   FILE* file = fopen(argv[2], "r"); // Opening the file for read operations --> "r". 
   if(file == NULL) { 
      printf("\nfopen failed");
      CloseHandle(hCom); // Tell the OS that the program does not access the port anymore
      return 7;
   } 

   // Getting file length
   fseek(file, 0L, SEEK_END);
   filelength = ftell(file);
   printf("\nFilelength: %d\n", filelength);
   rewind(file);//reseting cursor to the start of the file
   
   // Putting filesize:
   // Here there is the following problem: Data later will be send in bytes - one byte, however, can just convey a number from 0-255.
   // But the target is a filisize of about 4000 bytes.
   // Therefore, the filesize will be encoded the following way. 
   // memory_init_flags[0] will carry the number of how many blocks 255 bytes are in the file, element [1]
   // will then carry the number of bytes remaining.
   double required_capacity;
   unsigned char remaining_bytes;
   required_capacity = (double)filelength/(double)255; //determining how many bytes are needed to transmit filelength
   
   // Calculating the required capacity
   required_capacity = floor(required_capacity); // Round off to the next full integer
   remaining_bytes = filelength - (required_capacity*255);

   // The same problem exists for how many instructions the file carries. The encoding will be the same
   // Determine how many instructions are saved, by counting the newline characters in the file.
   // (Each line carries one instruction)
   int instruction_count = 1;
   char temp;
   while((temp = fgetc(file)) != EOF){ //fgetc gets one byte of the file and increments the cursor (the index in the file) by one

      if(temp == '\n') //if new-line character gets recognized increment the instruction counter
         instruction_count++;

   }
   rewind(file); // Reset the file cursor
   printf("\n%d instructions found!", instruction_count); // Print the result

   // Calculating the required capacity for the instructions in the same way as for the filesize
   double required_instruction_capacity = (double)instruction_count/(double)255;
   required_instruction_capacity = floor(required_instruction_capacity);
   unsigned char remaining_instructions = instruction_count - (255 * required_instruction_capacity);
   
   // Loading the calculated values into the array to be send.
   memory_init_flags[0] = (unsigned char)required_capacity;
   memory_init_flags[1] = remaining_bytes;
   memory_init_flags[2] = (unsigned char)required_instruction_capacity;
   memory_init_flags[3] = remaining_instructions;
   
   printf("\nDetermined filelength: %d \n",filelength);

   // Check whether the flags have been updated successfully:
   printf("Updated memoryinitflags:\n");
   for(int r=0;r<10;r++){
      printf("%d ", memory_init_flags[r]);
   }
     
   // Specifing for which events the program shall look out in the future
   // In this case the program shall be notefied by the OS when the port
   // associated with hCom recieves a byte. 
   // For this the SetCommMask function provided by windows is used.
   comreistatus = SetCommMask(hCom, EV_RXCHAR);
      
      if(comreistatus == FALSE){
            CloseHandle(hCom);
            printf("Error in setting commask");
         return 5;
      }
      else
      printf("\nSuccessfully set Commask\n");
      
      // Write the memory init flags to the port:
      printf("\nStart Writing to USB-Port\n");

      // For writing to the port the WriteFile function is used - which is also provided by microsoft in the windows.h headerfile
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

      
      comreistatus = WaitCommEvent(hCom, &dwEventMask, NULL); // Wait for a byte to be received - program is paused/stopped in the meantime
      if(!comreistatus){
         printf("Error setting WaitCommEvent");
      }
      else
         printf("waited");
   
         // The next step is to read from the USB-port 
         i=0;
         do{
               comreistatus = ReadFile(hCom, &ReadData, sizeof(ReadData),&NoBytesRead, NULL);
               if(NoBytesRead!=0){ // As long as a byte has been received
                  memory_init_feedback[i] = ReadData;
                  printf("\nFeedbackValue %d:%d\n", i, memory_init_feedback[i]); // Printing what has been recieved
                  
               }
               i++;
         }while(NoBytesRead>0);
      
      
      printf("\n Bytes returned by the microcontroller:\n");

      // Printing the result of reading:
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

      // Starting the transmission of the actual file:
      printf("\nStart of File Transmission");

      // Allocating memory for sending the the file:
      
      int filesendbuffer_index=0;
      unsigned char* filesendbuffer;
      filesendbuffer = (char*)malloc(filelength*sizeof(unsigned char));
      if(filesendbuffer!=NULL){
         printf("\nSuccessfully allocated memory for sendbuffer, %d bytes\n", strlen(filesendbuffer));
      }

      // Allocating memory for recieving the the file:
      int filereceivebuffer_index=0;
      unsigned char* filereceivebuffer;
      filereceivebuffer = (char*)malloc(filelength*sizeof(unsigned char));
      if(filereceivebuffer!=NULL){
         printf("\nSuccessfully allocated memory for receivebuffer, %d bytes\n", strlen(filereceivebuffer));
      }

      // Loading the file into the sendbuffer
      unsigned char character;
      while (filesendbuffer_index < filelength){ // Read the file byte by byte 
         character = fgetc(file);
         filesendbuffer[filesendbuffer_index]=character;
         printf("%c",filesendbuffer[filesendbuffer_index]);
         filesendbuffer_index++;
      }
      printf("\n");

      printf("Purging Com");
      BOOL purgesuccess = PurgeComm(hCom, PURGE_RXCLEAR); // Clears any unread data in the recieve buffer - also provided by microsoft
      if(purgesuccess)
      printf("Surccessfully purged comm");

      // Sending all the bytes in the same way as the memory init flags before 
      printf("\nStart Transmit of File\n");
      writesuccess = WriteFile(hCom,// Handle to the Serialport
                        filesendbuffer,            // Data to be written to the port
                        filelength,   // No of bytes to write into the port
                        &BytesWritten,  // No of bytes written to the port
                        NULL);
      if(writesuccess) {
         printf("Write Success - %d bytes written!\n", BytesWritten);
      }
      else
         printf("Write Error!\n");
      
      
      Sleep(1000); //waiting for the microcontrollers response
      comreistatus = WaitCommEvent(hCom, &dwEventMask, NULL); //Wait for the character to be received
      if(!comreistatus){
         printf("Error setting WaitCommEvent");
      }
      else
         printf("waited\n");

      // Reading Feedback in the same way as before
      
      i=0;
      
         do{
               //printf(" (LoIter: %u) ",i);
               comreistatus = ReadFile(hCom, &ReadData, sizeof(ReadData),&NoBytesRead, NULL);
               if(NoBytesRead!=0){
                  filereceivebuffer[i] = ReadData;
                  printf("%c", filereceivebuffer[i]);
                  
               }
               i++;
         }while(NoBytesRead>0);

      while(1) {

         comreistatus = ReadFile(hCom, &ReadData, sizeof(ReadData),&NoBytesRead, NULL);
               if(NoBytesRead!=0){
                  
                  filereceivebuffer[i] = ReadData;
                  printf("%c", filereceivebuffer[i]);
                  
               }

      }
      
      
      free(filesendbuffer); // Free allocated memory
      free(filereceivebuffer); // Free allocated memory
      CloseHandle(hCom); // Closing port again
      fclose(file); // Closing file again

   printf("End of Program");
   return 0;
   }