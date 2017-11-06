/** Very simple bitstream programmer for Lattice iCE40 FPGAs
 * using plain FTDI C232HM cable in bitbang mode so to support
 * Go Board. Uses libftdi directly, not libmpsse
 *
 * See http://jdelfes.blogspot.it/2014/02/spi-bitbang-ft232r.html
 *
 * david.siorpaes@st.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ftdi.h>

#define ADBUS0 (1 << 0)
#define ADBUS1 (1 << 1)
#define ADBUS2 (1 << 2)
#define ADBUS3 (1 << 3)
#define ADBUS4 (1 << 4)
#define ADBUS5 (1 << 5)
#define ADBUS6 (1 << 6)
#define ADBUS7 (1 << 7)

/* MPSSE Cable colors */
#define ORANGE  ADBUS0
#define YELLOW  ADBUS1
#define GREEN   ADBUS2
#define BROWN   ADBU33
#define GREY    ADBUS4
#define PURPLE  ADBUS5
#define WHITE   ADBUS6
#define BLUE    ADBUS7

/* Select here target board */

#if 0
/* Actual pinout for upduino board */
#define PIN_SCK   ORANGE
#define PIN_SS    WHITE
#define PIN_MOSI  YELLOW
#define PIN_RST   BLUE
#define FTDI_MINOR (0x6014)

#else

/* Pinout for Go Board. Note that MISO/MOSI are reverted with respect to the iCE40 master mode! */
#define PIN_SCK   ADBUS0
#define PIN_SS    ADBUS4
#define PIN_MOSI  ADBUS2
#define PIN_RST   ADBUS7
#define FTDI_MINOR (0x6010)
#endif

unsigned char gpio_out = PIN_SCK | PIN_SS | PIN_MOSI | PIN_RST;

uint8_t* buffer;
int bufferPos;

int digitalWrite(struct ftdi_context *ftdi, uint8_t pin, int value)
{	
	// store current value
	static uint8_t r = 0;
	
	if (value)
		r |= pin;
	else
		r &= ~pin;

	/* Prepare buffer */
	buffer[bufferPos++] = r;

	return 0;
}


void spi_byte(struct ftdi_context *ftdi, uint8_t byte)
{
	int i;
	
	for (i = 7; i >= 0; i--) { // most significant bit first
		if (byte & (1 << i))
			digitalWrite(ftdi, PIN_MOSI, 1);
		else
			digitalWrite(ftdi, PIN_MOSI, 0);
		
		// do a clock
		digitalWrite(ftdi, PIN_SCK, 1);
		digitalWrite(ftdi, PIN_SCK, 0);
	}
}


void spi_send(struct ftdi_context *ftdi, uint8_t* buf, int len)
{
	int i;

	for(i=0; i<len; i++){
		spi_byte(ftdi, buf[i]);
	}
}


int main(int argc, char** argv)
{
	struct ftdi_context *ftdi;
	struct ftdi_version_info version;
	int filefd;
	struct stat in_stat;
	void* fileaddr;	
	int err, r, i;

	uint32_t sync = 0x7eaa997e;
	uint8_t trailer[4];
	
	if(argc < 2){
		fprintf(stderr, "Usage: %s <file> [file] ...\n", argv[0]);
		exit(1);
	}


	/* Open and mmap file */
	if((filefd = open(argv[1], O_RDONLY)) < 0) {
		fprintf(stderr, "Failed to open %s: %s\n", argv[1], strerror(errno));
		return errno;
	}

	fstat(filefd, &in_stat);
	printf("File length: %u\n", (int)in_stat.st_size);

	fileaddr = mmap(NULL, in_stat.st_size, PROT_READ, MAP_SHARED, filefd, 0);
	if(fileaddr == MAP_FAILED){
		fprintf(stderr, "Failed to mmap %s: %s\n", argv[1], strerror(errno));
		return errno;
	}

	ftdi = ftdi_new();
	if (ftdi == 0) {
		fprintf(stderr, "ftdi_new failed\n");
		return EXIT_FAILURE;
	}
	
	// show library version
	version = ftdi_get_library_version();
	printf("Initialized libftdi %s (major: %d, minor: %d, micro: %d,"
		" snapshot ver: %s)\n", version.version_str, version.major,
		version.minor, version.micro, version.snapshot_str);
	
	// try to open usb
	r = ftdi_usb_open(ftdi, 0x0403, FTDI_MINOR);
	if (r != 0) {
		fprintf(stderr, "unable to open ftdi device: %d (%s)\n", r, ftdi_get_error_string(ftdi));
		ftdi_free(ftdi);
		return EXIT_FAILURE;
	}
	
	// Read out FTDIChip-ID
	if (1 || ftdi->type == TYPE_R) {
		unsigned int chipid = 0;
		
		printf("ftdi_read_chipid: %d\n", ftdi_read_chipid(ftdi, &chipid));
		printf("FTDI chipid: %X\n", chipid);
	}

	err = ftdi_set_bitmode(ftdi, gpio_out, BITMODE_SYNCBB);
	//err = ftdi_set_bitmode(ftdi, gpio_out, BITMODE_BITBANG);
	if(err){
		printf("Error %i in ftdi_set_bitmode(): %s", err, ftdi_get_error_string(ftdi));
		return EXIT_FAILURE;
	}


	/* For each bit there's a full port write.
	 * Each byte requires 10 bit writes
	 * Add extra heading/trailer
	 */
	buffer = malloc(10*8*in_stat.st_size + 1024);
	if(buffer == NULL){
		printf("Error allocating buffer\r");
		return EXIT_FAILURE;
	}
	
	/* Prepare SPI */
	digitalWrite(ftdi, PIN_SS,   1);
	digitalWrite(ftdi, PIN_MOSI, 0);
	digitalWrite(ftdi, PIN_SCK,  0);

	/* Put iCE40 in slave mode */
	digitalWrite(ftdi, PIN_SS,  0);
	digitalWrite(ftdi, PIN_RST, 0);
	usleep(5000);
	digitalWrite(ftdi, PIN_RST, 1);
	usleep(5000);

	
	/* First, send Synchronization Pattern. Cfr TN1248 */
	spi_send(ftdi, (uint8_t*)&sync, sizeof(sync));

	/* Send bitstream */
	spi_send(ftdi, (uint8_t*)fileaddr, in_stat.st_size);

	/* Send dummy bits */
	spi_send(ftdi, (uint8_t*)trailer, sizeof(trailer));

#define CHUNK 1024

	/* Commit actual full buffer */
	//err = ftdi_write_data(ftdi, buffer, bufferPos);
	for(i=0; i<bufferPos/CHUNK; i++){
		err = ftdi_write_data(ftdi, &buffer[i*CHUNK], CHUNK);
		if(err != sizeof(r)){
			printf("Error %i %i in ftdi_write_data(): %s\n", err, i, ftdi_get_error_string(ftdi));
		}
	}
	
	/* Clean up */
	r = ftdi_usb_close(ftdi);
	if (r != 0) {
		fprintf(stderr, "unable to close ftdi device: %d (%s)\n", r,
			ftdi_get_error_string(ftdi));
		ftdi_free(ftdi);
		return EXIT_FAILURE;
	}
	
	ftdi_free(ftdi);

	munmap(fileaddr, in_stat.st_size);
	
	return 0;
}
