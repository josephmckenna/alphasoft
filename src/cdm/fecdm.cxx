#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "lmk04800.h"

#define SPI_DEVICE "/dev/spidev1.0"

static void LMK04800_Write(unsigned short addr, unsigned char cmd);
static unsigned char LMK04800_Read(unsigned short addr);
static int GetSPIDevice(const char* device);
static void spi_transfer(int fd, uint8_t const *tx, size_t tx_len, uint8_t const *rx, size_t rx_len);

#define NUM_MSS_GPIO 32

static int fd_gpio[NUM_MSS_GPIO];
static unsigned char off_value = 0;
static unsigned char on_value = 1;

#define FP_LED1 fd_gpio[24]
#define FP_LED2 fd_gpio[25]
#define FP_LED3 fd_gpio[26]
#define FP_LED4 fd_gpio[27]
#define FP_LED5 fd_gpio[28]
#define FP_LED6 fd_gpio[29]
#define FP_LED7 fd_gpio[30]
#define FP_LED8 fd_gpio[31]

#define SEL_NIM fd_gpio[3]
#define SOURCE_SEL fd_gpio[4]
#define SEL_EXT fd_gpio[5]
#define CLK0_3_EN fd_gpio[6]
#define CLK4_7_EN fd_gpio[7]
#define CLK8_11_EN fd_gpio[8]
#define CLK12_15_EN fd_gpio[9]
#define CLK16_19_EN fd_gpio[10]
#define CLK20_23_EN fd_gpio[11]
#define LMK_SYNC fd_gpio[12]

void InitGPIO(void) {
    fd_gpio[1]  = open("/sys/class/gpio/gpio1/value",  O_WRONLY);
    fd_gpio[2]  = open("/sys/class/gpio/gpio2/value",  O_WRONLY);
    fd_gpio[3]  = open("/sys/class/gpio/gpio3/value",  O_WRONLY);
    fd_gpio[4]  = open("/sys/class/gpio/gpio4/value",  O_WRONLY);
    fd_gpio[5]  = open("/sys/class/gpio/gpio5/value",  O_WRONLY);
    fd_gpio[6]  = open("/sys/class/gpio/gpio6/value",  O_WRONLY);
    fd_gpio[7]  = open("/sys/class/gpio/gpio7/value",  O_WRONLY);
    fd_gpio[8]  = open("/sys/class/gpio/gpio8/value",  O_WRONLY);
    fd_gpio[9]  = open("/sys/class/gpio/gpio9/value",  O_WRONLY);
    fd_gpio[10] = open("/sys/class/gpio/gpio10/value", O_WRONLY);
    fd_gpio[11] = open("/sys/class/gpio/gpio11/value", O_WRONLY);
    fd_gpio[12] = open("/sys/class/gpio/gpio12/value", O_WRONLY);
    fd_gpio[13] = open("/sys/class/gpio/gpio13/value", O_RDONLY);
    fd_gpio[14] = open("/sys/class/gpio/gpio14/value", O_RDONLY);
    fd_gpio[15] = open("/sys/class/gpio/gpio15/value", O_RDONLY);
    fd_gpio[16] = open("/sys/class/gpio/gpio16/value", O_RDONLY);
    fd_gpio[17] = open("/sys/class/gpio/gpio17/value", O_RDONLY);
    fd_gpio[18] = open("/sys/class/gpio/gpio18/value", O_RDONLY);
    fd_gpio[19] = open("/sys/class/gpio/gpio19/value", O_RDONLY);
    fd_gpio[20] = open("/sys/class/gpio/gpio20/value", O_RDONLY);
    fd_gpio[21] = open("/sys/class/gpio/gpio21/value", O_RDONLY);
    fd_gpio[22] = open("/sys/class/gpio/gpio22/value", O_RDONLY);
    fd_gpio[23] = open("/sys/class/gpio/gpio23/value", O_RDONLY);
    fd_gpio[24] = open("/sys/class/gpio/gpio24/value", O_WRONLY);
    fd_gpio[25] = open("/sys/class/gpio/gpio25/value", O_WRONLY);
    fd_gpio[26] = open("/sys/class/gpio/gpio26/value", O_WRONLY);
    fd_gpio[27] = open("/sys/class/gpio/gpio27/value", O_WRONLY);
    fd_gpio[28] = open("/sys/class/gpio/gpio28/value", O_WRONLY);
    fd_gpio[29] = open("/sys/class/gpio/gpio29/value", O_WRONLY);
    fd_gpio[30] = open("/sys/class/gpio/gpio30/value", O_WRONLY);
    fd_gpio[31] = open("/sys/class/gpio/gpio31/value", O_WRONLY);

    write(SEL_NIM, "0", 1); // 0 - TTL, 1 - NIM
    write(SOURCE_SEL, "0", 1); // 0 - Internal OSC, 1 - Atomic Clock
    write(SEL_EXT, "1", 1); // 0 - eSATA, 1 - External SYNC (NIM/TTL)
    write(LMK_SYNC, "1", 1); // LMK SYNC OFF

    write(CLK0_3_EN, "1", 1);
    write(CLK4_7_EN, "1", 1);
    write(CLK8_11_EN, "1", 1);
    write(CLK12_15_EN, "1", 1);
    write(CLK16_19_EN, "1", 1);
    write(CLK20_23_EN, "1", 1);
}

static void LMK04800_Write(unsigned short addr, unsigned char cmd) {
  unsigned char tx[3];

  int fd = GetSPIDevice(SPI_DEVICE);

  tx[0] = (addr >> 8) & 0xff;
  tx[1] = (addr >> 0) & 0xff;
  tx[2] = cmd;
  spi_transfer(fd, tx, 3, 0, 0); 
  // printf("LMK04800 Write: 0x%X,\t0x%X\n", addr, cmd);
}

static unsigned char LMK04800_Read(unsigned short addr) {
  unsigned char tx[2];
  unsigned char rx[1];

  int fd = GetSPIDevice(SPI_DEVICE);

  tx[0] = 0x80 | ((addr >> 8) & 0xff);
  tx[1] = (addr >> 0) & 0xff;
 
  spi_transfer(fd, tx, 2, rx, 1); 

  return rx[0];
}

static void spi_transfer(int fd, uint8_t const *tx, size_t tx_len, uint8_t const *rx, size_t rx_len) {
	struct spi_ioc_transfer	xfer[2];
	unsigned char		*bp;
	int			status;

	memset(xfer, 0, sizeof xfer);

	xfer[0].tx_buf = (__u64) tx;
	xfer[0].len = tx_len;

	if(rx) {
	  xfer[1].rx_buf = (__u64) rx;
	  xfer[1].len = rx_len;
	  status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
	} else {
	  status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
	}
	if (status < 0) {
		perror("SPI_IOC_MESSAGE");
		return;
	}

	if(rx) {
	  printf("response(%2d, %2d): ", rx_len, status);
	  for (const unsigned char* bp = rx; rx_len; rx_len--)
	    printf(" %02x", *bp++);
	  printf("\n");
	}

}

static int GetSPIDevice(const char* device) {
  static int fd;
  static int init;

  if(!init) {
    init = 1;
    fd = open(device, O_RDWR);
  }

  return fd;
}

int main(int argc, char **argv) {
  int ret = -1;
  int fd;

  int n;
  tLMK04800 settings;

  InitGPIO();
  LMK04800_SetDefaults(&settings);

  settings.SPI_3WIRE_DIS = 1;
  settings.RESET_MUX 	= 6;	// SPI Readback
  settings.RESET_TYPE = 3;	// Output Push/Pull
  settings.OSCin_FREQ = 1; 	// 63-127 MHz. actual freq is 100 MHz
  settings.SYNC_POL   = 1;
  settings.VCO_MUX    = 1;
  settings.OSCout_FMT = 0; 
  settings.SYSREF_GBL_PD = 1;
  settings.CLKin_SEL_MODE = 0; // 0 - Atomic, 1 - eSATA, 2 - NIM
  settings.HOLDOVER_EN = 0;
  settings.PLL1_CP_POL = 1;


  if (0) {
    settings.CLKin_SEL_MODE = 0; // 0 - Atomic, 1 - eSATA, 2 - NIM

    settings.CLKin0_R = 1; //16; 	// 10 MHz Input clock expected from Atomic Clock
    settings.CLKin1_R = 10; 	// 62.5 Mhz Input clock expected from eSATA
    settings.CLKin2_R = 10; 	// 62.5 Mhz Input clock expected from external clock
    settings.PLL1_N = 10; //160; 	// This makes the above "R" settings work!

    // PLL1 freq 10 MHz
  }

  if (1) {
    settings.CLKin_SEL_MODE = 2; // 0 - Atomic, 1 - eSATA, 2 - NIM

    settings.CLKin0_R = 1; //16; 	// 10 MHz Input clock expected from Atomic Clock
    settings.CLKin1_R = 10; 	// 62.5 Mhz Input clock expected from eSATA
    settings.CLKin2_R = 10; 	// 62.5 Mhz Input clock expected from external clock
    settings.PLL1_N = 16; 	// This makes the above "R" settings work!

    // PLL1 freq 6.25 MHz
    // PLL1 R side is: 62.5 MHz/R = 62.5/10 = 6.25 MHz
    // PLL1 N side is: 100 MHz/N = 100/16 = 6.25 MHz
  }

  // PLL2 R side: 100 MHz / R2 divider = 100/64 = 1.5625 MHz
  // PLL2 N side: 1500 MHz / VCO1_DIV = 1500/2 / N2 prescaler = 1500/(2*8) / N2 divider = 1500/(2*8*60) = 1.5625
  // PLL2 freq = 1500 MHz

  settings.PLL2_R = 64; // "R2 divider"
  settings.PLL2_P = 8; // PLL2_N_Prescaler or "N2 prescaler"
  settings.PLL2_N = 60; // 120; // "N2 divider"
  settings.PLL2_N_CAL = 60; // 120;
  settings.VCO1_DIV = 0; // value 0 = divide by 2
  settings.OSCout_FMT = 0;
  settings.CLKin_OVERRIDE = 1;

  for(n=0; n<4; n++) {
    settings.ch[n].DCLKoutX_DIV = 24;		// 30 - 50MHz, 24 - Should come out as 62.5 MHz on LMK04821
    settings.ch[n].DCLKoutX_DDLY_PD = 1;	// Disable delay
    settings.ch[n].CLKoutX_Y_PD = 0;	// Enable
    settings.ch[n].SDCLKoutY_PD = 0;	// Enable
    settings.ch[n].DCLKoutX_FMT = 1;	// LVPECL
    settings.ch[n].SDCLKoutY_FMT = 1;	// LVPECL
    settings.ch[n].DCLKoutX_ADLYg_PD = 0;
    settings.ch[n].DCLKoutX_ADLY_PD = 0;
    settings.ch[n].DCLKoutX_MUX = 3;
  }

  for(n=4; n<7; n++) {
    settings.ch[n].DCLKoutX_DIV = 24;		// 30 - 50MHz, 24 - Should come out as 62.5 MHz on LMK04821
    settings.ch[n].DCLKoutX_DDLY_PD = 1;	// Disable delay
    settings.ch[n].CLKoutX_Y_PD = 0;	// Enable
    settings.ch[n].SDCLKoutY_PD = 0;	// Enable
    settings.ch[n].DCLKoutX_FMT = 1;	// LVPECL
    settings.ch[n].SDCLKoutY_FMT = 1;	// LVPECL
    settings.ch[n].DCLKoutX_ADLYg_PD = 0;
    settings.ch[n].DCLKoutX_ADLY_PD = 0;
    settings.ch[n].DCLKoutX_MUX = 3;
  }

  LMK04800_Program(&settings, LMK04800_Write);
  
  write(LMK_SYNC, "1", 1);
  write(LMK_SYNC, "0", 1);
  write(LMK_SYNC, "1", 1);

  close(GetSPIDevice(SPI_DEVICE));
 
  do {
    for(n = 24; n<NUM_MSS_GPIO; n++) {
      write(fd_gpio[n], "1", 1);
    }
    usleep(200000);

    for(n = 24; n<NUM_MSS_GPIO; n++) {
      write(fd_gpio[n], "0", 1);
    }
    usleep(200000);
  } while(1);
 
  return 0;
}
