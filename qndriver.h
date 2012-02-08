/************************************************Copyright(c)******************
**                   Quintic(Nanjing) Microelectronics Co,Ltd.
**
**                        http://www.quinticcorp.com
**
**-----------------------------File Info---------------------------------------
** File Name:                  qndriver.h
** subversion number:   1.0
**-----------------------------------------------------------------------------
******************************************************************************/

#ifndef _QNDRIVER_H
#define _QNDRIVER_H
#define QN_8036
#define _QNFUNC_H_

#define CCS_RX                          0
#define CCS_TX                          1

#define FREQ2CHREG(freq)                ((freq-6000)/5)
#define CHREG2FREQ(ch)                  (ch*5+6000)

#define _QNCOMMON_H_

#define QND_REG_NUM_MAX                 122
/**********************************QN8035's clock source selection**************
1.QN8035's default clock source is 32768HZ.
2.setting QN8035's clock source and clock source type(like sine-wave clock or digital clock).
3.user need to modify clock source according to actual hardware platform.
4.clock formula,the details please refer to the QN8035's datasheet
  XTAL_DIV = Round(Clock/32768);
  PLL_DLT = Round((28500000*512*XTAL_DIV)/Clock)-442368
*******************************************************************************/
#define QND_SINE_WAVE_CLOCK             0x00    //inject sine-wave clock
#define QND_DIGITAL_CLOCK               0x80    //inject digital clock,default is inject digital clock
//crystal clock is 32768HZ
#define QND_XTAL_DIV0                   0x01
#define QND_XTAL_DIV1                   0x08
#define QND_XTAL_DIV2                   0x5C

#define QND_MODE_SLEEP                  0
#define QND_MODE_WAKEUP                 1

// RX / TX value is using upper 8 bit
#define QND_MODE_RX                     0x8000
#define QND_MODE_TX                     0x4000
// AM / FM value is using lower 8 bit
// need to check datasheet to get right bit
#define QND_MODE_FM                     0x0000
#define BAND_FM                         0

// tune
#define QND_FSTEP_50KHZ                 0
#define QND_FSTEP_100KHZ                1
#define QND_FSTEP_200KHZ                2

// audio output format
#define QND_OUTPUT_ANALOG               0
#define QND_OUTPUT_IIS                  1

// stereo mode
#define QND_TX_AUDIO_MONO               0x10
#define QND_TX_AUDIO_STEREO             0x00

#define QND_RX_AUDIO_MONO               0x20
#define QND_RX_AUDIO_STEREO             0x00

#define QND_CONFIG_MONO                 0x01
#define QND_CONFIG_MUTE                 0x02
#define QND_CONFIG_SOFTCLIP             0x03
#define QND_CONFIG_AUTOAGC              0x04
#define QND_CONFIG_AGCGAIN              0x05
#define QND_CONFIG_EQUALIZER            0x06
#define QND_CONFIG_VOLUME               0x07
#define QND_CONFIG_BASS_QUALITY         0x08
#define QND_CONFIG_BASS_FREQ            0x09
#define QND_CONFIG_BASS_GAIN            0x0a
#define QND_CONFIG_MID_QUALITY          0x0b
#define QND_CONFIG_MID_FREQ             0x0c
#define QND_CONFIG_MID_GAIN             0x0d
#define QND_CONFIG_TREBLE_FREQ          0x0e
#define QND_CONFIG_TREBLE_GAIN          0x0f

#define QND_ENABLE_EQUALIZER            0x10
#define QND_DISABLE_EQUALIZER           0x00

#define QND_CONFIG_AUDIOPEAK_DEV        0x11
#define QND_CONFIG_PILOT_DEV            0x12
#define QND_CONFIG_RDS_DEV              0x13

// input format
#define QND_INPUT_ANALOG                0
#define QND_INPUT_IIS                   1

// i2s mode
#define QND_I2S_RX_ANALOG               0x00
#define QND_I2S_RX_DIGITAL              0x40
#define QND_I2S_TX_ANALOG               0x00
#define QND_I2S_TX_DIGITAL              0x20

//i2s clock data rate
#define QND_I2S_DATA_RATE_32K           0x00
#define QND_I2S_DATA_RATE_40K           0x10
#define QND_I2S_DATA_RATE_44K           0x20
#define QND_I2S_DATA_RATE_48K           0x30

//i2s clock Bit Wise
#define QND_I2S_BIT_8                   0x00
#define QND_I2S_BIT_16                  0x40
#define QND_I2S_BIT_24                  0x80
#define QND_I2S_BIT_32                  0xc0

//i2s Control mode
#define QND_I2S_MASTER                  1
#define QND_I2S_SLAVE                   0

//i2s Control mode
#define QND_I2S_MSB                     0x00
#define QND_I2S_I2S                     0x01
#define QND_I2S_DSP1                    0x02
#define QND_I2S_DSP2                    0x03
#define QND_I2S_LSB                     0x04

#define QND_EQUALIZE_BASS               0x00
#define QND_EQUALIZE_MID                0x01
#define QND_EQUALIZE_TREBLE             0x02

// RDS, TMC
#define QND_EUROPE_FLEXIBILITY_DISABLE  0
#define QND_EUROPE_FLEXIBILITY_ENABLE   1
#define QND_RDS_OFF                     0
#define QND_RDS_ON                      1
#define QND_RDS_BUFFER_NOT_READY        0
#define QND_RDS_BUFFER_READY            1

/****************************Chips ID definition*******************************/
#define CHIPID_QN8000                   0x00
#define CHIPID_QN8005                   0x20
#define CHIPID_QN8005B1                 0x21
#define CHIPID_QN8006                   0x30
#define CHIPID_QN8006LB                 0x71
#define CHIPID_QN8007B1                 0x11
#define CHIPID_QN8007                   0x10
#define CHIPID_QN8006A1                 0x30
#define CHIPID_QN8006B1                 0x31
#define CHIPID_QN8016                   0xe0
#define CHIPID_QN8016_1                 0xb0
#define CHIPID_QN8015                   0xa0
#define CHIPID_QN8065                   0xa0
#define CHIPID_QN8067                   0xd0
#define CHIPID_QN8065N                  0xa0
#define CHIPID_QN8027                   0x40
#define CHIPID_QN8025                   0x80
#define CHIPID_QN8035                   0x84
#define CHIPSUBID_QN8035A0              0x01
#define CHIPSUBID_QN8035A1              0x02
#define CHIPID_QN8026                   0x3C
#define CHIPID_QN8036                   0x34
/***************************************End************************************/
typedef unsigned char                   UINT8;
typedef char                            INT8;
typedef unsigned short                  UINT16;
typedef unsigned int                    UINT32;
typedef short                           INT16;

/**************************minor feature selection*****************************/
#define  QN_CCA_MAX_CH                  50
/***************************************End************************************/

#define _QNCONFIG_H_
/******************************country selection*******************************/
#define COUNTRY_CHINA                   0
#define COUNTRY_USA                     1
#define COUNTRY_JAPAN                   2
/***************************************End************************************/

/*******************************************************************************
Performance configuration
*******************************************************************************/
#define SMSTART_VAL                     19
#define HCCSTART_VAL                    25
#define SNCSTART_VAL                    51

enum {
    // Bit[23-16] of the word: RSSI high Threshold
    // Bit[15-8] of the word: RSSI low Threshold
    // Bit[7-0] of the word: SNR Threshold
    // e.g. 0x641E0A => RSSI_HTH = 0x64,RSSI_LTH = 0x1E, SNR_TH = 0x0A
    // notice: the rang of RSSI is 0x0A ~ 0x3F
    // notice: the rang of SNR is 0x00 ~ 0x3F

    CCA_SENSITIVITY_LEVEL_0 = 0x641E06,
    CCA_SENSITIVITY_LEVEL_1 = 0x641E07,   //if using the pilot as CCA,reference this item.
    CCA_SENSITIVITY_LEVEL_2 = 0x641E08,
    CCA_SENSITIVITY_LEVEL_3 = 0x641E09,
    CCA_SENSITIVITY_LEVEL_4 = 0x641E0A,  //if not using the pilot as CCA,reference this item.
    CCA_SENSITIVITY_LEVEL_5 = 0x641E0B,
    CCA_SENSITIVITY_LEVEL_6 = 0x641E0C,
    CCA_SENSITIVITY_LEVEL_7 = 0x641E0D,
    CCA_SENSITIVITY_LEVEL_8 = 0x641E0E,
    CCA_SENSITIVITY_LEVEL_9 = 0x641E1F
};
/***************************************End************************************/

/*******************************************************************************
limitation configuration
*******************************************************************************/
#define QND_READ_RSSI_DELAY             5
#define ENABLE_2K_SPEED_PLL_DELAY          20
#define SLEEP_TO_WAKEUP_DELAY_TIME      500
#define CH_SETUP_DELAY_TIME             200
#define CCA_TIME_OUT					200

enum {
	FM_LEAP_STEP_5 = 0,
	FM_LEAP_STEP_10,
	FM_LEAP_STEP_20
};

/***************************************End************************************/
#ifndef assert
#define assert(str)
#endif

#define QND_LOG(a)
#define QND_LOGA(a,b)
#define QND_LOGB(a,b)
#define QND_LOGHEX(a,b)
#define _QNREG_H_
/*******************************************************************************
 definition register
*******************************************************************************/
#define SYSTEM1         				0x00
#define SYSTEM2         				0x01
#define CCA             				0x02
#define SNR            					0x03
#define RSSISIG         				0x04
#define CID1            				0x05
#define CID2            				0x06
#define	XTAL_DIV0						0x07
#define	XTAL_DIV1						0x08
#define	XTAL_DIV2						0x09
#define	STATUS1							0x0A
#define	RX_CH						    0x0B
#define	CH_START						0x0C
#define	CH_STOP							0x0D
#define	CH_STEP							0x0E
#define	RX_RDSD0						0x0F
#define	RX_RDSD1						0x10
#define	RX_RDSD2						0x11
#define	RX_RDSD3						0x12
#define	RX_RDSD4						0x13
#define	RX_RDSD5						0x14
#define	RX_RDSD6						0x15
#define	RX_RDSD7						0x16
#define	STATUS2							0x17
#define	VOL_CTL							0x18
#define INT_CTRL						0x19
#define STATUS3                         0x1A
#define TXCH   						    0x1B
#define	TX_RDSD0					    0x1C
#define	TX_RDSD1						0x1D
#define	TX_RDSD2						0x1E
#define	TX_RDSD3						0x1F
#define	TX_RDSD4						0x20
#define	TX_RDSD5						0x21
#define	TX_RDSD6						0x22
#define	TX_RDSD7						0x23
#define	PAC						        0x24
#define	FDEV						    0x25
#define	RDS                             0x26
#define	GPLT                            0x27
#define	REG_VGA						    0x28
#define SMP_HLD_THRD					0x29
#define	FM_AGC_GAIN						0x2A
#define GAIN_SEL						0x2B
#define	SYSTEM_CTL1						0x2C
#define	SYSTEM_CTL2						0x2D
#define RDSCOSTAS						0x2E
#define REG_TEST						0x2F
#define STATUS4							0x30
#define RDSAGC2							0x31
#define RXAGC1							0x32
#define RXAGC2							0x33
#define RXAGC3							0x34
#define RXAGC4							0x35
#define RXAGC5							0x36
#define	CCA1							0x37
#define	CCA2							0x38
#define	CCA3							0x39
#define	CCA4							0x3A
#define	CCA5							0x3B
#define	CCOND1                          0x3C
#define	CCOND2                          0x3D
#define	CCOND3                          0x3E
#define	PLT1                            0x3F
#define	PLT2                            0x40
#define	SMSTART							0x44
#define	SNCSTART						0x45
#define	HCCSTART						0x46
#define	CCA_CNT1					    0x47
#define	CCA_CNT2					    0x48
#define	CCA_SNR_TH                      0x49
#define	REG_REF                         0x58
#define	REG_PD1                         0x5C
#define	REG_PD2                         0x5D
#define CCA_SIG_LTH                     0x65
#define CCA_SIG_HTH                     0x66
#define	CCA_NCNT1                       0x67
#define	CCA_NCNT2                       0x68
#define TXAGC1                          0x6E
#define	TXAGC3                          0x70
#define	TXAGC4                          0x71
#define	PAG_CAL1                        0x77
/***************************************End************************************/

/*******************************************************************************
 definition operation bit of registers
*******************************************************************************/
#define CCA_CH_DIS      				0x01
#define CCS_CH_DIS      				0x02
#define CHSC            				0x04
#define TXREQ           				0x08
#define RXREQ                           0x10
#define STNBY      				        0x20
#define RECAL      				        0x40
#define SWRST                           0x80

#define RDSRDY                          0x02
#define TX_MUTE                         0x04
#define RX_MUTE                         0x08
#define TX_MONO                         0x10
#define RX_MONO                         0x20
#define TX_RDSEN                        0x40
#define RX_RDSEN                        0x80

#define CH_CH		    				0x03
#define CH_CH_START     				0x0c
#define CH_CH_STOP      				0x30

#define PRIV_MODE        				0x04
#define RDS_4K_MODE      				0x08
#define S1K_EN      				    0x10
#define RDS_ONLY      				    0x20
#define CCA_INT_EN      				0x40
#define RDS_INT_EN      				0x80

#define IMR             				0x40
#define RDS_RXUPD       				0x80
#define RXCCA_MASK                      0x05
#define RX_CCA                          0x04
#define RXCCA_FAIL                      0x08
#define TXCCS_MASK                      0x06
#define TX_CCS                          0x04
#define PA_TRGT                         0x7F
#define PAIPOW                          0x03
#define IPOW_ATT                        0x20
#define TXAGC_GAIN_SEL                  0x80
#define TXAGC_GVGA                      0x70
#define TX_SFTCLPEN         			0x80
#define T1M_SEL                         0x30
#define GAIN_TXPLT                      0x08
#define RDSFDEV                         0x7F
#define TX_DIS         			        0x08
#define RDS_TXUPD                       0x04
#define ICPREF                          0x0F
#define PLTTH                           0x1C

#define NFILT_WEN            			0x08
#define NFILT            			    0x30
#define NO_FILT            			    0x00
#define FILT1            			    0x10
#define FILT2            			    0x20
#define FILT3            			    0x30

#define SNR_CCA_TH                      0x3F

#define CCA_NAGC                        0x38
#define CCA_NAGC_0_5MS                  0x00     //0.5ms
#define CCA_NAGC_2_5MS                  0x08     //2.5ms
#define CCA_NAGC_5MS                    0x10
#define CCA_NAGC_10MS                   0x18
#define CCA_NAGC_20MS                   0x20
#define CCA_NAGC_30MS                   0x28
#define CCA_NAGC_40MS                   0x30
#define CCA_NAGC_60MS                   0x38

#define CCA_NRSSI                       0x07
#define CCA_NRSSI_2_5MS                 0x00    //2.5ms
#define CCA_NRSSI_5MS                   0x01
#define CCA_NRSSI_10MS                  0x02
#define CCA_NRSSI_15MS                  0x03
#define CCA_NRSSI_20MS                  0x04
#define CCA_NRSSI_30MS                  0x05
#define CCA_NRSSI_40MS                  0x06
#define CCA_NRSSI_60MS                  0x07

#define CCA_NSNR                        0x38
#define CCA_NSNR_0MS                    0x00     //0ms
#define CCA_NSNR_2_5MS                  0x08     //2.5ms
#define CCA_NSNR_5MS                    0x10
#define CCA_NSNR_10MS                   0x18
#define CCA_NSNR_20MS                   0x20
#define CCA_NSNR_30MS                   0x28
#define CCA_NSNR_40MS                   0x30
#define CCA_NSNR_60MS                   0x38

#define CCA_NEND                        0x07
#define CCA_NEND_5MS                    0x00
#define CCA_NEND_10MS                   0x01
#define CCA_NEND_20MS                   0x02
#define CCA_NEND_40MS                   0x03
#define CCA_NEND_60MS                   0x04
#define CCA_NEND_80MS                   0x05
#define CCA_NEND_100MS                  0x06
#define CCA_NEND_120MS                  0x07

#define CC_SNR_RSSI_SEL                 0x80

#define ST_MO_RX                        0x01
#define DAC_HOLD                        0x40
#define LINE_IN_EN                      0x80

#define RIN                             0x03
#define AUDIO_INPUT_IMPEDANCE_10K       0x00
#define AUDIO_INPUT_IMPEDANCE_20K       0x01
#define AUDIO_INPUT_IMPEDANCE_40K       0x02
#define AUDIO_INPUT_IMPEDANCE_80K       0x03
/***************************************End************************************/

#define _QNSYS_H_

#define CHANNEL_FILTER

// external driver interface
// logical layer
/*****************************************************************************
Driver API Macro Definition
*****************************************************************************/
#define QNM_GetMonoMode() \
        QND_ReadReg(STATUS1) & ST_MO_RX
#define QNM_SetAudioInputImpedance(AudioImpendance) \
        QNF_SetRegBit(REG_VGA,RIN,AudioImpendance)
#define QNM_SetFrequecyDeviation(FreqDeviation) \
        QND_ReadReg(FDEV,FreqDeviation)
#define QNM_SetRDSFrequecyDeviation(RDSFreqDeviation) \
        QND_ReadReg(RDS,RDSFDEV,RDSFreqDeviation)
#define QNM_SetPAOffTime(Time) \
        QNF_SetRegBit(GPLT,T1M_SEL,Time)
#define QNM_SetTXPilotFrequencyDeviation(PilotFreqDeviation) \
        QNF_SetRegBit(GPLT,GAIN_TXPLT,PilotFreqDeviation)
#define QNM_GetRssi() \
        QND_ReadReg(RSSISIG)
#define QND_READ(adr)    QND_ReadReg(adr)
#define QND_WRITE(adr, value)  QND_WriteReg(adr, value)

//extern static void   QNF_SetRegBit(UINT8 reg, UINT8 bitMask, UINT8 data_val) ;
//extern UINT8   qnd_Country;
//extern UINT16  qnd_CH_START;
//extern UINT16  qnd_CH_STOP;
//extern UINT8   qnd_CH_STEP;
//extern UINT16 QNF_GetCh();
extern void QND_Delay(UINT16 ms);
extern UINT8 QND_GetRSSI(UINT16 ch) ;
extern UINT8 QND_Init() ;
extern void  QND_TuneToCH(UINT16 ch) ;
extern void  QND_SetSysMode(UINT16 mode) ;
extern void  QND_SetCountry(UINT8 country) ;

#define QN_RX
#define _QNRX_H_
typedef void  (*QND_SeekCallBack)(UINT16 ch, UINT8 bandtype);
//extern UINT8  qnd_ChCount;
extern UINT16 qnd_ChList[QN_CCA_MAX_CH];
//extern UINT8  qnd_StepTbl[3];
//extern UINT8  qnd_AutoScanAll;

//extern void   QND_SetSeekCallBack(QND_SeekCallBack func);
extern void   QND_RXConfigAudio(UINT8 optiontype, UINT8 option) ;
//extern void QND_RXSeekCHInit(UINT8 th);
extern UINT8 QND_RXValidCH(UINT16 freq, UINT8 step);

extern UINT16 QND_RXSeekCH(UINT16 start, UINT16 stop, UINT8 step, UINT8 db, UINT8 up) ;
extern UINT8  QND_RXSeekCHAll(UINT16 start, UINT16 stop, UINT8 step, UINT8 db, UINT8 up) ;
#define QN_RDS
#define _QNRDS_H_
#define QN_TX
#define _QNTX_H_
extern void QND_TXSetPower(UINT8 gain);
extern void QND_TXConfigAudio(UINT8 optiontype, UINT8 option );
extern UINT16 QND_TXClearChannelScan(UINT16 start, UINT16 stop, UINT8 step,UINT8 db);
extern void QNF_SetMute(UINT8 On);

#endif

