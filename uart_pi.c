#include "uart_pi.h"

#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART

#include <stdio.h>
#include <string.h> 

int uart0_filestream = -1;

void init_uart()
{
    //-------------------------
    //----- SETUP USART 0 -----
    //-------------------------
    //At bootup, pins 8 and 10 are already set to UART0_TXD.

    //OPEN THE UART
    //The flags (defined in fcntl.h):
    //  Access modes (use 1 of these):
    //      O_RDONLY - Open for reading only.
    //      O_RDWR - Open for reading and writing.
    //      O_WRONLY - Open for writing only.
    //
    //  O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
    //                                          if there is no input immediately available (instead of blocking). Likewise, write requests can also return
    //                                          immediately with a failure status if the output can't be written immediately.
    //
    //  O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
    uart0_filestream = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);        //Open in non blocking read/write mode

    if (uart0_filestream == -1)
    {
        //ERROR - CAN'T OPEN SERIAL PORT
        perror("Error - Unable to open UART.  Ensure it is not in use by another application\n");
    }

    //CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)

    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;     //<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);
}

void transmit_string_uart(unsigned char* tx_str)
{
    const int udly_uart = 200; // UART delay time for STM32F103

    for ( int i = 0; i < strlen((char*)tx_str); i++ )
    {
        write(uart0_filestream, &tx_str[i], sizeof(tx_str[0]));
        usleep(udly_uart);
    }
    write(uart0_filestream, "", sizeof(tx_str[0]));
}

void receive_uart()
{
    const int udly_uart = 200; // UART delay time for STM32F103

    usleep(udly_uart*100);

    // Because O_NDELAY has been used this will exit if there are no receive bytes waiting (non blocking read), so if you want to hold waiting for input simply put this in a while loop
    // Read up to 255 characters from the port if they are there
    unsigned char rx_buffer[256];
    int rx_length = read(uart0_filestream, (void*)rx_buffer, 255);      //Filestream, buffer to store in, number of bytes to read (max)

    //Bytes received
    rx_buffer[rx_length] = '\0';
    printf("%i bytes read : %s", rx_length, rx_buffer);
}

int transmit_receive_uart(unsigned char* tx_str) // taking ~ 22 ms
{
    const int udly_uart = 200; // UART delay time for STM32F103

    //--------------------tx--------------------//
    for ( int i = 0; i < strlen((char*)tx_str); i++ )
    {
        write(uart0_filestream, &tx_str[i], sizeof(tx_str[0]));
        usleep(udly_uart);
    }
    write(uart0_filestream, "", sizeof(tx_str[0])); // Null terminating

    //--------------------rx--------------------//
    usleep(udly_uart*100);

    // Because O_NDELAY has been used this will exit if there are no receive bytes waiting (non blocking read), so if you want to hold waiting for input simply put this in a while loop
    // Read up to 255 characters from the port if they are there
    unsigned char rx_buffer[256];
    int rx_length = read(uart0_filestream, (void*)rx_buffer, 255);      //Filestream, buffer to store in, number of bytes to read (max)
    rx_buffer[rx_length] = '\0';

    //--------------------check--------------------//
    if ( strncmp((char*)tx_str, (char*)rx_buffer, strlen((char*)tx_str)) == 0 && rx_length <= (7+2) ) // 7+2 means "max no. of commands + \r\n"
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

