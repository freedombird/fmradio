#include <stdio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <strings.h> // bzero
#include <string.h>  // strlen
#include <signal.h>
#include <sys/mman.h>

#include <alsa/asoundlib.h>
#include <pthread.h>
	
#include "fmradio.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define FM_I2C_ADDR (0x42)
#define I2C_FILE_NAME "/dev/i2c-0"

int i2c_file = 0, pid_file;


static void print_hex(FILE *file, char *bytes, int size) 
{
    int i;
	
    for(i=0; i<size; i++) {
        fprintf(file, "%02x ", bytes[i]);
        if(!((i+1)%16))
            fprintf(file, "\n");
    }
}

static int set_fm_register(int file, unsigned char reg, unsigned char value) 
{
    unsigned char outbuf[2];
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];

    messages[0].addr = FM_I2C_ADDR;
    messages[0].flags = 0;
    messages[0].len = sizeof(outbuf);
    messages[0].buf = outbuf;

    outbuf[0] = reg;
    outbuf[1] = value;

    packets.msgs = messages;
    packets.nmsgs = 1;

    if(ioctl(file, I2C_RDWR, &packets) < 0) {
        perror("Unable to write data");
        return 1;
    }
    return 0;
}


static int get_fm_register(int file, unsigned char reg, int *val) 
{
    unsigned char inoutbuf[4];
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    inoutbuf[0] = reg;

    messages[0].addr = FM_I2C_ADDR;
    messages[0].flags = 0;
    messages[0].len = 1;
    messages[0].buf = inoutbuf;

    messages[1].addr = FM_I2C_ADDR;
    messages[1].flags = I2C_M_RD;
    messages[1].len = 1;
    messages[1].buf = &inoutbuf[1];

    packets.msgs = messages;
    packets.nmsgs = 2;

    if(ioctl(file, I2C_RDWR, &packets) < 0) {
        perror("Unable to write/read data");
        return 1;
    }

    *val = inoutbuf[1];
    return 0;
}


static int dump_fm_registers(int file) {
    unsigned char inbuf[128], outbuf = 0;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];
	
    bzero(inbuf, sizeof(inbuf));

    messages[0].addr = FM_I2C_ADDR;
    messages[0].flags = 0;			// write flag
    messages[0].len = 1;
    messages[0].buf = &outbuf;

    messages[1].addr = FM_I2C_ADDR;
    messages[1].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
    messages[1].len = sizeof(inbuf);
    messages[1].buf = inbuf;

    packets.msgs = messages;
    packets.nmsgs = 2;

    if(ioctl(file, I2C_RDWR, &packets) < 0) {
        perror("Unable to send data");
        return 1;
    }
    print_hex(stdout, (char *)inbuf, sizeof(inbuf));
    fprintf(stdout, "\n");
    return 0;
}

void fm_init(void)
{
    if(i2c_file != 0)
        return;
    
    // Open a connection to the I2C userspace control file.
    if ((i2c_file = open(I2C_FILE_NAME, O_RDWR)) < 0)
    {
        perror("Unable to open i2c control file");
        exit(1);
    }
    printf("fm init...\n");
    QND_Init();
    QND_SetCountry(COUNTRY_CHINA);
    QND_SetSysMode(QND_MODE_FM|QND_MODE_RX);
    QND_RXConfigAudio(QND_CONFIG_MONO, 40);
    QND_RXConfigAudio(QND_CONFIG_MUTE, 0);
}

// auto scan fm radio from 87.5Mhz  to 108Mhz, and reture the avaliable radio num. 
UINT16* fm_rx_scan(int *cnt, int db)
{
        UINT8 channel_count;
		  
        printf("%s begin db=%d\n", __FUNCTION__, db);
        QND_SetCountry(COUNTRY_CHINA);
        QND_SetSysMode(QND_MODE_RX|QND_MODE_FM);        
        QND_RXConfigAudio(QND_CONFIG_MONO, 0);
        QND_RXConfigAudio(QND_CONFIG_MUTE, 0);
        
        channel_count = QND_RXSeekCHAll(8750, 10800, FM_LEAP_STEP_20, db, 1);
       *cnt = channel_count ;
       
	 return &qnd_ChList[0];
}

// show the radio's signal 
UINT8 fm_get_rx_rssi(UINT16 freq)
{    
    return QND_GetRSSI(freq);
}

// tune the receive freq to @freq, example : 87.5Mhz=8750
void fm_rx_freq(int freq)
{
	linein_to_hp(1);	
	QND_TuneToCH(freq);
	QNF_SetMute(0);
}

// change the FM mode to rx mode, and set the transmit  audio at @freq.
void fm_tx_freq(int freq)
{
	QND_SetSysMode(QND_MODE_TX);	// Enter the transmit mode
	QND_TuneToCH(freq);	// can set a default NO. such as 7800
	QND_TXSetPower(32);			// Maximum the output power
	QND_Delay(10);
	QND_SetSysMode(QND_MODE_TX);	// Enter the transmit mode again
}

void fm_clean(void) 
{
    if(i2c_file > 0) {
        close(i2c_file);
        i2c_file = 0;
    }
	linein_to_hp(0);    
}

/***************************** codec on/off functions   ***************************/
const char card[] = "default";
const char ctl_elem_name[] = "HP Mux";

static int found = 0;
static int numid = -1;
static int iface = SND_CTL_ELEM_IFACE_MIXER;

#define HW_AUDIOOUT_HPVOL_SET_ADDR      (0x80048054)
#define HW_AUDIOOUT_HPVOL_CLR_ADDR      (0x80048058)
#define HP_MUTE_BIT                     (0x1 << 24)

static int hp_mute_switch(int value)
{
    int fd;
	void * mem;
	void * aligned_vaddr;
	unsigned long aligned_paddr;
	unsigned long aligned_size;

	const int g_size = 4;
	unsigned long g_paddr;

    if (value) {        /* clear bit */
        g_paddr = HW_AUDIOOUT_HPVOL_CLR_ADDR;
    } else {
        g_paddr = HW_AUDIOOUT_HPVOL_SET_ADDR;
    }

	/* Align address to access size */
	g_paddr &= ~(g_size - 1);

	aligned_paddr = g_paddr & ~(4096 - 1);
	aligned_size = g_paddr - aligned_paddr + g_size;
	aligned_size = (aligned_size + 4096 - 1) & ~(4096 - 1);

	if ((fd = open("/dev/mem", O_RDWR, 0)) < 0)
		return 1;

	aligned_vaddr = mmap(NULL, aligned_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, aligned_paddr);
	if (aligned_vaddr == NULL) {
		printf("Error mapping address\n");
		close(fd);
		return 1;
	}

	mem = (void *)((unsigned int)aligned_vaddr + (g_paddr - aligned_paddr));

    *((unsigned int *)mem) = HP_MUTE_BIT;
 	munmap(aligned_vaddr, aligned_size);
	close(fd);
	
    return 0;
}

static int get_hp_mux_iface(void)
{
	int err;
	snd_hctl_t *handle;
	snd_hctl_elem_t *elem;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_info_alloca(&info);

	if ((err = snd_hctl_open(&handle, card, 0)) < 0) {
		printf("Control %s open error: %s", card, snd_strerror(err));
		return err;
	}
	if ((err = snd_hctl_load(handle)) < 0) {
		printf("Control %s local error: %s\n", card, snd_strerror(err));
		return err;
	}

    found = 0;
	for (elem = snd_hctl_first_elem(handle); elem; elem = snd_hctl_elem_next(elem)) {
		if ((err = snd_hctl_elem_info(elem, info)) < 0) {
			printf("Control %s snd_hctl_elem_info error: %s\n", card, snd_strerror(err));
			return err;
		}

		snd_hctl_elem_get_id(elem, id);
		if (!strcmp(snd_ctl_elem_id_get_name(id), ctl_elem_name)) {
		    numid = snd_ctl_elem_id_get_numid(id);
		    iface = snd_ctl_elem_id_get_interface(id);
		    found = 1;
            break;
	    }
	}

	snd_hctl_close(handle);

	return found;
}

static int hp_mux_cset(int value)
{
	int err;
	static snd_ctl_t *handle = NULL;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_value_t *control;
	unsigned int idx, count;
	snd_ctl_elem_type_t type;
	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_value_alloca(&control);

    if (!get_hp_mux_iface()) {
        return -1;
    }

    snd_ctl_elem_id_set_numid(id, numid);
    snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
    snd_ctl_elem_id_set_name(id, ctl_elem_name);

	if (handle == NULL &&
	    (err = snd_ctl_open(&handle, card, 0)) < 0) {
		printf("Control %s open error: %s\n", card, snd_strerror(err));
		return err;
	}
	snd_ctl_elem_info_set_id(info, id);
	if ((err = snd_ctl_elem_info(handle, info)) < 0) {
		printf("Cannot find the given element from control %s\n", card);
		return err;
	}
	snd_ctl_elem_info_get_id(info, id);	/* FIXME: Remove it when hctl find works ok !!! */
	type = snd_ctl_elem_info_get_type(info);
	count = snd_ctl_elem_info_get_count(info);
	snd_ctl_elem_value_set_id(control, id);


	for (idx = 0; idx < count && idx < 128; idx++) {
	    if (type == SND_CTL_ELEM_TYPE_ENUMERATED) {
			snd_ctl_elem_value_set_enumerated(control, idx, value);
			break;
	    }
	}

	if ((err = snd_ctl_elem_write(handle, control)) < 0) {
		printf("Control %s element write error: %s\n", card, snd_strerror(err));
		return err;
	}

	snd_ctl_close(handle);
    handle = NULL;

    return 0;
}

int linein_to_hp(int value)
{
    if (value) {
        hp_mux_cset(1);
        hp_mute_switch(1);
    } else {
        hp_mux_cset(0);
        hp_mute_switch(0);
    }

    return 0;
}
/*********************************** end codec on/off **************************************/
int main(int argc, char **argv) {
    int cnt = 0;
    UINT16 *pChlist = NULL;

    // Open a connection to the I2C userspace control file.
    if ((i2c_file = open(I2C_FILE_NAME, O_RDWR)) < 0) {
        perror("Unable to open i2c control file");
    }

    if(argc > 2 && !strcmp(argv[1], "r")) {
        int reg = strtol(argv[2], NULL, 0);
        int value;
        if(get_fm_register(i2c_file, reg, &value)) {
            fprintf(stderr, "Unable to get register!\n");
        }
        else {
            fprintf(stderr, "Register %d: %d (%x)\n", reg, value, value);
        }
    }

    else if(argc > 1 && !strcmp(argv[1], "d")) {
        if(dump_fm_registers(i2c_file)) {
            fprintf(stderr, "Unable to dump registers!\n");
        }
    }

    else if(argc > 3 && !strcmp(argv[1], "w")) {
        int reg = strtol(argv[2], NULL, 0);
        int value = strtol(argv[3], NULL, 0);
        if(set_fm_register(i2c_file, reg, value)) {
            fprintf(stderr, "Unable to get register!\n");
        }
        else {
            fprintf(stderr, "Set register %d: %d (%x)\n", reg, value, value);
        }
    }
    else if(argc > 1 && !strcmp(argv[1], "i")) {
        fm_init();
    }
    else if(argc > 2 && !strcmp(argv[1], "s")) {
        int i;
        int rxdb = strtol(argv[2], NULL, 0);
        UINT8 rssi = 0;
        pChlist = fm_rx_scan(&cnt, rxdb);
        printf("Found %d channels:\n", cnt);
        
        for(i=0; i<cnt; i++) {
        	rssi = fm_get_rx_rssi(pChlist[i]);
            fprintf(stderr, "    %d.%02d MHz RSSI=%d\n",
                    pChlist[i]/100,
                    pChlist[i]-100*(pChlist[i]/100), rssi);
        }
    }
    else if(argc > 2 && !strcmp(argv[1], "t")) {
        int rxfreq = strtol(argv[2], NULL, 0);
        
        fprintf(stderr, "Tuning to channel %d.%d\n", rxfreq/100, rxfreq-100*(rxfreq/100));
        fm_rx_freq(rxfreq);
    }
    else if (argc > 2 && !strcmp(argv[1], "tx")) {
        UINT16 txfreq = strtol(argv[2], NULL, 0);
        fprintf(stderr, "TX manual at channel %d.%d\n", txfreq / 100, txfreq-100*(txfreq/100));
        fm_tx_freq(txfreq);
    } else {
        fprintf(stderr, "Usage:\n"
                "  r [register]   to read value from [register]\n"
                "  w [register] [value]   to write a value [value] to register [register]\n"
                "  d              to dump all registers from 0-255\n"
                "  i              Initialize the FM radio\n"
                "  t [frequency]  [scan level] Tune in to [frequency]\n"
                "  s              Perform an FM scan\n"
                "  rds            Display RDS data\n"
                " tx               set transmit freq\n"
                );
    }
    
    //fm_clean();

    return 0;
}
#ifdef  __cplusplus
}
#endif
