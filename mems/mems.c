#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include "iobb.h"
#include <stdint.h>


#define REF_ON 0x01000008 //command to turn on internal VREF
#define X_OFFSET 3
#define Y_OFFSET 0
#define X_DRIVE  1
#define Y_DRIVE  2
#define SETV 3
#define SPI_BUS SPI0

uint32_t convert_spi(uint16_t dac_val,uint8_t channel);
uint32_t to_send (uint32_t spi_data);

int main(int argc, char **argv) {
  printf("AAA!\n");
  int i, fd, debug=0, loop=0;
  int x_off=-1, y_off=-1;
  int x_val=-1, y_val=-1;
  int rxval;
  int status;
  unsigned int buf = REF_ON;
  unsigned int dac_value = 1; // 0 to 65535 value to set dac output
  unsigned int spi_bytes = 0; // spi communication bytes to send
  unsigned int pin_direction;
  char direction = 0; // Direction of triangle wave ramp
  

  /* fd = open("/dev/spidev1.0", O_RDWR); */
  /* if(fd < 0) printf("spi failed to open\n"); */
 
  
  iolib_init();
  //Set LDAC control pin to output 
  iolib_setdir(9,15,1);
  //Tying LDAC low will update dac channel as soon as it has new data 
  pin_low(9,15); //sel0
  /* write(fd,&buf,4); //set internal vref on */

  pin_direction=SPI_INOUT;

  /* get parameters */
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-' && argv[i][1] == 'd')
      debug = 1;
    if (argv[i][0] == '-' && argv[i][1] == 'l')
      loop = 1;
    else if (argv[i][0] == '-') {
      if (i + 1 >= argc || argv[i + 1][0] == '-')
	goto usage;
      if (strncmp(argv[i], "-x", 2) == 0)
	x_val = atoi(argv[++i]);
      if (strncmp(argv[i], "-y", 2) == 0)
	y_val = atoi(argv[++i]);
      if (strncmp(argv[i], "-ox", 3) == 0)
	x_off = atoi(argv[++i]);
      else if (strncmp(argv[i], "-oy", 3) == 0)
	y_off = atoi(argv[++i]);
    } else {
    usage:
      printf("usage: mem [-ox value>] [-oy value] [-x value] [-y value] -loop\n");
      return 0;
    }
  }
  
  status=spi_enable(SPI_BUS);
  if (status) {
    spi_ctrl(SPI_BUS, SPI_CH0, SPI_MASTER, SPI_RXTX, /* use SPI_TX or SPI_RXTX depending on needs */
	     SPI_DIV4, SPI_CLOCKMODE2, /* 48 MHz divided by 32, SPI mode 3 */
	     SPI_CE_ACT_LOW, pin_direction, 32); //32-bit transactions
  } else {
    printf("error, spi is not enabled\n");
    iolib_free();
    return(1);
  }
    
  printf("CCC!\n");

  //Manual offset settings only
  if (x_off != -1) {
  //set X offset
    /* printf("setting x-off:%d\n", x_off); */
    spi_bytes=convert_spi(x_off, X_OFFSET);//format bytes for write function
    to_send(spi_bytes);
  }
  if (y_off != -1) {
   //set Y offset
    /* printf("setting y-off:%d\n", y_off); */
    spi_bytes=convert_spi(y_off, Y_OFFSET);//format bytes for write function
    to_send(spi_bytes);
  }
  if (x_val != -1) {
    //set X value
    /* printf("setting x-val:%d\n", x_val); */
    spi_bytes=convert_spi(x_val, X_DRIVE);//format bytes for write function
    to_send(spi_bytes);
  }
  if (y_val != -1) {
    //set Y value
    /* printf("setting y-val:%d\n", y_val); */
    spi_bytes=convert_spi(y_val, Y_DRIVE);//format bytes for write function
    to_send(spi_bytes);
  }

  return 0;
}

uint32_t convert_spi(uint16_t dac_val,uint8_t channel){
  uint32_t spi_data=0;

  spi_data |= SETV << 24;
  spi_data |= (channel & 0xF) << 20;
  spi_data |= (dac_val << 4);
  return spi_data;
}
                                                                                                                                                                                                   
 uint32_t to_send (uint32_t spi_data){
   unsigned int rxval1;
   spi_transact(SPI_BUS, SPI_CH0, spi_data, &rxval1); 
   /* printf("rx: 0x%x\t0x%x\n",rxval1); */
   return rxval1;
 }
