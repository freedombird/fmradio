#include <unistd.h>
#include <stdio.h>
#include "qndriver.h"
#include "qnio.h"

#ifdef  __cplusplus
extern "C" {
#endif

UINT8   qnd_Country  = COUNTRY_CHINA ;
UINT16  qnd_CH_START = 8750;
UINT16  qnd_CH_STOP  = 10800;
UINT8   qnd_CH_STEP  = 1;

UINT8  qnd_AutoScanAll = 0;
UINT8  qnd_ChCount;
UINT16 qnd_ChList[QN_CCA_MAX_CH];
UINT8   qnd_StepTbl[3]={5,10,20};

QND_SeekCallBack qnd_CallBackFunc = 0;

static void   QNF_SetRegBit(UINT8 reg, UINT8 bitMask, UINT8 data_val) ;
static void QND_RXSeekCHInit(UINT8 th);

/**********************************************************************
void QNF_RXInit()
**********************************************************************
Description: set to SNR based MPX control. Call this function before
             tune to one specific channel
Parameters:
None
Return Value:
None
**********************************************************************/
static void QNF_RXInit()
{
    QNF_SetRegBit(0x49,0x10,0x10);
    //Let NFILT adjust freely
    QNF_SetRegBit(GAIN_SEL,NFILT_WEN,0x00);
    QNF_SetRegBit(SMP_HLD_THRD,CC_SNR_RSSI_SEL,CC_SNR_RSSI_SEL);
    QNF_SetRegBit(0x4F,0x70,0x70);
    //Enable the channel condition filter3 adaptation,Let ccfilter3 adjust freely
    QNF_SetRegBit(SYSTEM_CTL2,0x40,0x00);
    QNF_SetRegBit(SMSTART, 0x7F,SMSTART_VAL);
    QNF_SetRegBit(SNCSTART,0x7F,SNCSTART_VAL);
    QNF_SetRegBit(HCCSTART,0x7F,HCCSTART_VAL);
}

/**********************************************************************
void QNF_SetMute(UINT8 On)
**********************************************************************
Description: set register specified bit

Parameters:
On:        1: mute, 0: unmute
Return Value:
None
**********************************************************************/
void QNF_SetMute(UINT8 On)
{
	if(On)
	{
        	QNF_SetRegBit(VOL_CTL, DAC_HOLD, DAC_HOLD); //mute DAC
	}
	else
	{
        	QNF_SetRegBit(VOL_CTL, DAC_HOLD, 0x00); //unmute DAC
	}
}

/**********************************************************************
void QNF_SetRegBit(UINT8 reg, UINT8 bitMask, UINT8 data_val)
**********************************************************************
Description: set register specified bit

Parameters:
    reg:        register that will be set
    bitMask:    mask specified bit of register
    data_val:    data will be set for specified bit
Return Value:
    None
**********************************************************************/
static void QNF_SetRegBit(UINT8 reg, UINT8 bitMask, UINT8 data_val)
{
    UINT8 temp;
    temp = QND_ReadReg(reg);
    temp &= (UINT8)(~bitMask);
    temp |= data_val & bitMask;
    QND_WriteReg(reg, temp);
}

/**********************************************************************
UINT16 QNF_GetCh()
**********************************************************************
Description: get current channel frequency

Parameters:
    None
Return Value:
    channel frequency
**********************************************************************/
static UINT16 QNF_GetCh()
{
    UINT8 tCh;
    UINT8  tStep;
    UINT16 ch = 0;
    // set to reg: CH_STEP
    tStep = QND_ReadReg(CH_STEP);
    tStep &= CH_CH;
    ch  =  tStep ;
    tCh= QND_ReadReg(RX_CH);
    ch = (ch<<8)+tCh;
    return CHREG2FREQ(ch);
}

/**********************************************************************
void QNF_SetCh(UINT16 freq)
**********************************************************************
Description: set channel frequency

Parameters:
    freq:  channel frequency to be set
Return Value:
    None
**********************************************************************/
static void QNF_SetCh(UINT16 freq)
{
    // calculate ch parameter used for register setting
    UINT8 tStep;
    UINT8 tCh;
    UINT16 f;
    UINT16 temp;

    if(QND_ReadReg(SYSTEM1) & TXREQ)
    {
        //working in TX mode
        //Manually set TX Channel index
		QNF_SetRegBit(SYSTEM1, CCS_CH_DIS,CCS_CH_DIS);
		f = FREQ2CHREG(freq);
		temp = (UINT8)(f >> 8);
		QNF_SetRegBit(INT_CTRL, 0x03,temp);
		temp = (UINT8)f;
		QND_WriteReg(TXCH, temp^0x01);
		QNF_SetRegBit(0x49, 0x20, 0x20);
		QND_Delay(ENABLE_2K_SPEED_PLL_DELAY);
		QND_WriteReg(TXCH, temp);
		QNF_SetRegBit(0x49, 0x20, 0x00);
		QNF_SetRegBit(0x2d, 0x08, 0x00);
		QNF_SetRegBit(0x2d, 0x08, 0x08);
		return;
	}
	//Manually set RX Channel index
    QNF_SetRegBit(SYSTEM1, CCA_CH_DIS, CCA_CH_DIS);
    f = FREQ2CHREG(freq);
    // set to reg: CH
    tCh = (UINT8) f;
    QND_WriteReg(RX_CH, tCh);
    // set to reg: CH_STEP
    tStep = QND_ReadReg(CH_STEP);
    tStep &= ~CH_CH;
    tStep |= ((UINT8) (f >> 8) & CH_CH);
    QND_WriteReg(CH_STEP, tStep);
}

/**********************************************************************
void QNF_ConfigScan(UINT16 start,UINT16 stop, UINT8 step)
**********************************************************************
Description: config start, stop, step register for FM/AM CCA or CCS

Parameters:
    start
        Set the frequency (10kHz) where scan to be started,
        e.g. 7600 for 76.00MHz.
    stop
        Set the frequency (10kHz) where scan to be stopped,
        e.g. 10800 for 108.00MHz
    step
        1: set leap step to (FM)100kHz / 10kHz(AM)
        2: set leap step to (FM)200kHz / 1kHz(AM)
        0:  set leap step to (FM)50kHz / 9kHz(AM)
Return Value:
         None
**********************************************************************/
static void QNF_ConfigScan(UINT16 start,UINT16 stop, UINT8 step)
{
    // calculate ch para
    UINT8 tStep = 0;
    UINT8 tS;
    UINT16 fStart;
    UINT16 fStop;

    fStart = FREQ2CHREG(start);
    fStop = FREQ2CHREG(stop);
    // set to reg: CH_START
    tS = (UINT8) fStart;
    QND_WriteReg(CH_START, tS);
    tStep |= ((UINT8) (fStart >> 6) & CH_CH_START);
    // set to reg: CH_STOP
    tS = (UINT8) fStop;
    QND_WriteReg(CH_STOP, tS);
    tStep |= ((UINT8) (fStop >> 4) & CH_CH_STOP);
    // set to reg: CH_STEP
    tStep |= step << 6;
    QND_WriteReg(CH_STEP, tStep);
}

/**********************************************************************
int QND_Delay()
**********************************************************************
Description: Delay for some ms, to be customized according to user
             application platform

Parameters:
        ms: ms counts
Return Value:
        None

**********************************************************************/
 void QND_Delay(UINT16 ms)
{
    usleep(ms*1000);
}

/**********************************************************************
UINT8 QND_GetRSSI(UINT16 ch)
**********************************************************************
Description:    Get the RSSI value
Parameters:
Return Value:
RSSI value  of the channel settled
**********************************************************************/
UINT8 QND_GetRSSI(UINT16 ch)
{
    QNF_SetRegBit(REG_REF,ICPREF,0x0A);
    QNF_SetCh(ch);
    QNF_SetRegBit(0x49, 0x20, 0x20);
    QND_Delay(ENABLE_2K_SPEED_PLL_DELAY);
    QNF_SetRegBit(0x49, 0x20, 0x00);
    QNF_SetRegBit(REG_REF,ICPREF,0x00);
    return QND_ReadReg(RSSISIG);
}

/**********************************************************************
void QN_ChipInitialization()
**********************************************************************
Description: chip first step initialization, called only by QND_Init()

Parameters:
None
Return Value:
None
**********************************************************************/
static void QN_ChipInitialization()
{
    QNF_SetRegBit(0x00,0x80,0x80);
    QND_Delay(10);
    // change crystal frequency setting here
    //setting input clock source type:sine-wave clock or digital clock
    QNF_SetRegBit(0x02,0x80,QND_DIGITAL_CLOCK);
    QND_WriteReg(XTAL_DIV0, QND_XTAL_DIV0);
    QND_WriteReg(XTAL_DIV1, QND_XTAL_DIV1);
    QND_WriteReg(XTAL_DIV2, QND_XTAL_DIV2);
    QND_Delay(10);
    QND_WriteReg(0x57,0x05);
    QND_WriteReg(0x5B,0x42);
    QNF_SetRegBit(0x6E,0x70,0x60);
}

/**********************************************************************
int QND_Init()
**********************************************************************
Description: Initialize device to make it ready to have all functionality ready for use.

Parameters:
    None
Return Value:
    1: Device is ready to use.
    0: Device is not ready to serve function.
**********************************************************************/
UINT8 QND_Init()
{
    QN_ChipInitialization();
    return 1;
}

/**********************************************************************
void QND_SetSysMode(UINT16 mode)
***********************************************************************
Description: Set device system mode(like: sleep ,wakeup etc)
Parameters:
mode:  set the system mode , it will be set by  some macro define usually:

QND_MODE_SLEEP  : set chip to sleep mode
QND_MODE_WAKEUP : wake up chip
QND_MODE_RX     : set chip work on RX mode
QND_MODE_TX     : set chip work on TX mode
Return Value:
None
**********************************************************************/
void QND_SetSysMode(UINT16 mode)
{
    switch(mode)
    {
        case QND_MODE_SLEEP:        //set sleep mode
            QNF_SetRegBit(SYSTEM1, STNBY, STNBY);
            break;
        case QND_MODE_WAKEUP:     //set wakeup mode
            if(QND_ReadReg(SYSTEM1) & RXREQ)
            {
                QNF_SetMute(1);
                QNF_SetRegBit(SYSTEM1, STNBY, 0x00);
                //avoid noise from sleep to wakeup mode during.
                QND_Delay(SLEEP_TO_WAKEUP_DELAY_TIME);
                QNF_SetMute(0);
            }
            else
            {
                QNF_SetRegBit(SYSTEM1, STNBY, 0x00);
            }
            break;
        case QND_MODE_RX:
            QNF_SetMute(1);
            QNF_SetRegBit(SYSTEM1,0x38,0x10);
            break;
        case QND_MODE_TX:
            QNF_SetRegBit(SYSTEM1,0x38,0x08);
            break;
        default:
            break;
    }
}

/**********************************************************************
void QND_TuneToCH(UINT16 ch)
**********************************************************************
Description: Tune to the specific channel. call QND_SetSysMode() before
call this function
Parameters:
ch
Set the frequency (10kHz) to be tuned,
e.g. 101.30MHz will be set to 10130.
Return Value:
None
**********************************************************************/
void QND_TuneToCH(UINT16 ch)
{
    // if chip is working on TX mode, just set CH
    if(QND_ReadReg(SYSTEM1) & TXREQ)
    {
        QNF_SetCh(ch);
        return;
    }
    QNF_SetRegBit(REG_REF,ICPREF,0x0A);
    QNF_RXInit();

    QNF_SetMute(1);
    if ((ch==6910)||(ch==7290)||(ch==8430)||(ch==10710))
    {
        QNF_SetRegBit(CCA, IMR, IMR);
    }
    else
    {
        QNF_SetRegBit(CCA, IMR, 0x00);
    }
    QNF_SetCh(ch);
    QNF_SetRegBit(0x49, 0x20, 0x20);
    QND_Delay(ENABLE_2K_SPEED_PLL_DELAY);
    QNF_SetRegBit(0x49, 0x20, 0x00);
    //avoid the noise which are "POP" and "sha sha" noise.
    QND_Delay(CH_SETUP_DELAY_TIME);
    QNF_SetRegBit(REG_REF,ICPREF,0x00);
    QNF_SetMute(0);
}

/**********************************************************************
void QND_SetCountry(UINT8 country)
***********************************************************************
Description: Set start, stop, step for RX and TX based on different
             country
Parameters:
country:
Set the chip used in specified country:
    CHINA:
    USA:
    JAPAN:
Return Value:
    None
**********************************************************************/
void QND_SetCountry(UINT8 country)
{
    qnd_Country = country;
    switch(country)
    {
        case COUNTRY_CHINA:
            qnd_CH_START = 8750;
            qnd_CH_STOP = 10800;
            qnd_CH_STEP = 1;
            break;
        case COUNTRY_USA:
            qnd_CH_START = 8810;
            qnd_CH_STOP = 10790;
            qnd_CH_STEP = 2;
            break;
        case COUNTRY_JAPAN:
            qnd_CH_START = 7600;
            qnd_CH_STOP = 9000;
            qnd_CH_STEP = 1;
            break;
        default:
            break;
    }
}

/***********************************************************************
void QND_RXSeekCHConfig(UINT8 th)
***********************************************************************
Description: configuration chip before automatic scan channel
th:
  Setting threshold for quality of channel to be searched,
  the range of th value:CCA_SENSITIVITY_LEVEL_0 ~ CCA_SENSITIVITY_LEVEL_9
Return Value:
  None
***********************************************************************/
static void QND_RXSeekCHInit(UINT8 th)
{
 	UINT8 rssiLTH,rssiHTH;
    UINT8 snrTH;
    UINT32 rssi_snr_TH;
    UINT32 rssi_snr_TH_tbl [10] = { CCA_SENSITIVITY_LEVEL_0,CCA_SENSITIVITY_LEVEL_1,
                                    CCA_SENSITIVITY_LEVEL_2,CCA_SENSITIVITY_LEVEL_3,
                                    CCA_SENSITIVITY_LEVEL_4,CCA_SENSITIVITY_LEVEL_5,
                                    CCA_SENSITIVITY_LEVEL_6,CCA_SENSITIVITY_LEVEL_7,
                                    CCA_SENSITIVITY_LEVEL_8,CCA_SENSITIVITY_LEVEL_9
                                  };

    rssi_snr_TH = rssi_snr_TH_tbl[th];
    rssiHTH = (UINT8)(rssi_snr_TH >> 16) & 0xff;
    rssiLTH = (UINT8)(rssi_snr_TH >> 8) & 0xff;
    snrTH = (UINT8)rssi_snr_TH & 0xff;
    QNF_SetRegBit(0x49,0x10,0x00);
    //setting IF counter counts for 20ms
	QNF_SetRegBit(CCA2,0x38,0x00);
    QNF_SetRegBit(REG_REF,ICPREF,0x0A);
    QNF_SetRegBit(GAIN_SEL,NFILT_WEN,NFILT_WEN);
    QNF_SetRegBit(SMP_HLD_THRD,CC_SNR_RSSI_SEL,CC_SNR_RSSI_SEL);
    QNF_SetRegBit(CCA1,NFILT,FILT1);
    //Enable the channel condition filter3 adaptation,Let ccfilter3 adjust freely
    QNF_SetRegBit(SYSTEM_CTL2,0x40,0x00);
    //RSSI: 5ms  <->IF count:2226;
    //RSSI: 10ms <->IF count:4453;
	//RSSI: 20ms <->IF count:8906;
	//RSSI: 25ms <->IF count:11132;
	//RSSI: 30ms <->IF count:13359;
	//RSSI: 40ms <->IF count:17812;
    //setting lower 8 bits of CCA IF counter error range value
    QND_WRITE(CCA_CNT1,0x00);
    QNF_SetRegBit(CCA_CNT2,0x3F,0x01);
    QNF_SetRegBit(CCA_NCNT1, CCA_NAGC, CCA_NAGC_20MS);
    QNF_SetRegBit(CCA_NCNT1, CCA_NRSSI, CCA_NRSSI_10MS);
    QNF_SetRegBit(CCA_NCNT2, CCA_NSNR, CCA_NSNR_0MS);
    QNF_SetRegBit(CCA_NCNT2, CCA_NEND, CCA_NEND_5MS);
    QNF_SetRegBit(CCA_SIG_LTH, 0x7F, rssiLTH);
    QNF_SetRegBit(CCA_SIG_HTH, 0x7F, rssiHTH);
	QNF_SetRegBit(CCA ,SNR_CCA_TH, snrTH);
}

/***********************************************************************
UINT8 QND_RXValidCH(UINT16 freq, UINT8 step);
***********************************************************************
Description: to validate a ch (frequency)(if it's a valid channel)
Freq: specific channel frequency, unit: 10Khz
  e.g. 108.00MHz will be set to 10800.
Step:
  FM:
  QND_FMSTEP_100KHZ: set leap step to 100kHz
  QND_FMSTEP_200KHZ: set leap step to 200kHz
  QND_FMSTEP_50KHZ:  set leap step to 50kHz
Return Value:
  0: not a valid channel
  1: a valid channel at this frequency
***********************************************************************/
UINT8 QND_RXValidCH(UINT16 freq, UINT8 step)
{
    UINT8 regValue;
    UINT8 timeOut = CCA_TIME_OUT;
    UINT8 isValidChannelFlag = 0;
    UINT16 ifCount,ifCountError;
    UINT8 snr;
    //set the pilot detect threshold to the maximum value for
    //indirectly disable pilot as hardware CCA condition
    QNF_SetRegBit(PLT2,PLTTH,PLTTH);
    QNF_ConfigScan(freq, freq, step);
    //enter CCA mode,channel index is decided by internal CCA
    QNF_SetRegBit(SYSTEM1,RXCCA_MASK,RX_CCA);
    do
    {
        regValue = QND_ReadReg(SYSTEM1);
        QND_Delay(5);   //delay 5ms
        timeOut--;
        //TRACE("CHSC:0x%02x timeOut:%d \n",regValue&CHSC,timeOut);
    } while((regValue & CHSC) && timeOut);//when it seeks a channel or the time is overtime,the loop is quited
    //read out the rxcca_fail flag of RXCCA status
    isValidChannelFlag = QND_ReadReg(STATUS1) & RXCCA_FAIL ? 0:1;
    //after CCA,recover the pilot detect threshold to chip's default value
    QNF_SetRegBit(PLT2,PLTTH,0x08);
    if(isValidChannelFlag)
    {
		QNF_SetRegBit(0x49, 0x20, 0x20);
		QND_Delay(ENABLE_2K_SPEED_PLL_DELAY);
		QNF_SetRegBit(0x49, 0x20, 0x00);
		snr = QND_ReadReg(SNR);
		QNF_SetRegBit(RDSAGC2,0x80,0x80);
		QND_Delay(30);
		ifCount = QND_ReadReg(CCA5) & 0x3F << 8;
		ifCount |= QND_ReadReg(CCA4) << 8;
		ifCount|= QND_ReadReg(CCA3);
		QNF_SetRegBit(RDSAGC2,0x80,0x00);
		if(ifCount>=8906)
		{
			ifCountError = ifCount - 8906;
		}
		else
		{
			ifCountError = 8906 - ifCount;
		}
		if(ifCountError<=400)
		{
//			TRACE("ValidFreq:%d ifCountError:%d SNR:%d \n",freq,ifCountError,snr);
			isValidChannelFlag = 1;
		}
		else
		{
			isValidChannelFlag = 0;
//			TRACE("InvalidFreq:%d ifCountError:%d SNR:%d \n",freq,ifCountError,snr);
		}
	}
    if(isValidChannelFlag)
    {
     //   if(qnd_CallBackFunc)
     //   qnd_CallBackFunc(freq, BAND_FM);
        return 1;
    }
    else
    {
        return 0;
    }
}

/***********************************************************************
UINT16 QND_RXSeekCH(UINT16 start, UINT16 stop, UINT16 step, UINT8 db, UINT8 up);
***********************************************************************
Description: Automatically scans the frequency range, and detects the
first channel(FM, it will be determine by the system mode which set
by QND_SetSysMode).
A threshold value needs to be passed in for channel detection.
Parameters:
start
Set the frequency (10kHz) where scan will be started,
e.g. 76.00MHz will be set to 7600.
stop
Set the frequency (10kHz) where scan will be stopped,
e.g. 108.00MHz will be set to 10800.
step
FM:
QND_FMSTEP_100KHZ: set leap step to 100kHz
QND_FMSTEP_200KHZ: set leap step to 200kHz
QND_FMSTEP_50KHZ:  set leap step to 50kHz
db:
Set threshold for quality of channel to be searched,
the range of db value:CCA_SENSITIVITY_LEVEL_0 ~ CCA_SENSITIVITY_LEVEL_19
up:
Set the seach direction :
Up;0,seach from stop to start
Up:1 seach from start to stop
Return Value:
The channel frequency (unit: 10kHz)
***********************************************************************/
UINT16 QND_RXSeekCH(UINT16 start, UINT16 stop, UINT8 step, UINT8 db, UINT8 up)
{
    UINT16 freq;
    UINT8 stepValue;
    UINT16 pStart = start;
    UINT16 pStop = stop;
    UINT8 regValue;
    UINT8 timeOut;
    UINT8 isValidChannelFlag = 0;
    UINT16 ifCount,ifCountError;
    UINT8 snr;

    if(qnd_AutoScanAll == 0)
    {
        QNF_SetMute(1);
        QND_RXSeekCHInit(db);
    }
    stepValue = qnd_StepTbl[step];
    do
    {
        //set the pilot detect threshold to the maximum value for
        //indirectly disable pilot as hardware CCA condition
        QNF_SetRegBit(PLT2,PLTTH,PLTTH);
        QNF_ConfigScan(pStart, pStop, step);
        //enter CCA mode,channel index is decided by internal CCA
        QNF_SetRegBit(SYSTEM1,RXCCA_MASK,RX_CCA);
        timeOut = CCA_TIME_OUT;
        do
        {
            regValue = QND_ReadReg(SYSTEM1);
            QND_Delay(5);   //delay 5ms
            timeOut--;
            //TRACE("CHSC:0x%02x timeOut:%d \n",regValue&CHSC,timeOut);
        } while((regValue & CHSC) && timeOut);//when it seeks a channel or the time is overtime,the loop is quited
        //read out the rxcca_fail flag of RXCCA status
        isValidChannelFlag = QND_ReadReg(STATUS1) & RXCCA_FAIL ? 0:1;
        //after CCA,recover the pilot detect threshold to chip's default value
        QNF_SetRegBit(PLT2,PLTTH,0x08);
        freq = QNF_GetCh();
        if(isValidChannelFlag)
        {
            QNF_SetRegBit(0x49, 0x20, 0x20);
            QND_Delay(ENABLE_2K_SPEED_PLL_DELAY);
            QNF_SetRegBit(0x49, 0x20, 0x00);
            snr = QND_ReadReg(SNR);
			QNF_SetRegBit(RDSAGC2,0x80,0x80);
			QND_Delay(30);
			ifCount = QND_ReadReg(CCA5) & 0x3F << 8;
			ifCount |= QND_ReadReg(CCA4) << 8;
			ifCount|= QND_ReadReg(CCA3);
			QNF_SetRegBit(RDSAGC2,0x80,0x00);
			if(ifCount>=8906)
			{
				ifCountError = ifCount - 8906;
			}
			else
			{
				ifCountError = 8906 - ifCount;
			}
			if(ifCountError<=400)
			{
//				TRACE("ValidFreq:%d ifCountError:%d SNR:%d \n",freq,ifCountError,snr);
				isValidChannelFlag = 1;
				break;
			}
			else
			{
				isValidChannelFlag = 0;
//				TRACE("InvalidFreq:%d ifCountError:%d SNR:%d \n",freq,ifCountError,snr);
			}
        }
		if(isValidChannelFlag == 0)
		{
	        pStart = freq + (up ? stepValue : -stepValue);
	    }
    }
    while ((isValidChannelFlag == 0) && (up ? (pStart<=pStop):(pStart>=pStop)));
    if(isValidChannelFlag)
    {
        if(qnd_CallBackFunc)
        	qnd_CallBackFunc(freq, BAND_FM);
    }
    else
    {
        freq = 0;
    }
    if (qnd_AutoScanAll == 0)
    {
        QND_TuneToCH(freq ? freq: stop);
    }
    return freq;
}

/**********************************************************************
UINT8 QND_RXSeekCHAll(UINT16 start, UINT16 stop, UINT16 step, UINT8 db, UINT8 up)
**********************************************************************
Description:    Automatically scans the complete FM or AM band and detects
            all the available  channels(AM or FM, it will be determine by
            the workmode which set by QND_SetSysmode). A threshold value
            needs to be passed in for the channel detection.
Parameters:
    start
        Set the frequency (10kHz) where scan will be started,
        e.g. 76.00MHz will be set to 7600.
    stop
        Set the frequency (10kHz) where scan will be stopped,
        e.g. 108.00MHz will be set to 10800.
    Step
        FM:
            QND_FMSTEP_100KHZ: set leap step to 100kHz  -1
            QND_FMSTEP_200KHZ: set leap step to 200kHz  -2
            QND_FMSTEP_50KHZ:  set leap step to 50kHz  -0
    db:
    Set threshold for quality of channel to be searched,
    the range of db value:CCA_SENSITIVITY_LEVEL_0 ~ CCA_SENSITIVITY_LEVEL_19
    up:
        Set the seach direction :
        Up;0,seach from stop to start
        Up:1 seach from start to stop

Return Value:
  The channel count found by this function
  0: no channel found
**********************************************************************/
UINT8 QND_RXSeekCHAll(UINT16 start, UINT16 stop, UINT8 step, UINT8 db, UINT8 up)
{
    UINT16 pStart = start;
    UINT16 temp;
    UINT8  stepValue;
    UINT16 pStop = stop > qnd_CH_STOP ? qnd_CH_STOP : stop;

    QNF_SetMute(1);
    up=(start<stop) ? 1 : 0;
    qnd_AutoScanAll = 1;
    qnd_ChCount = 0;
    stepValue = qnd_StepTbl[step];
 //   TRACE("/*Automaic Scan Channle starting...!start:%d stop:%d up:%d*/ \n",start,stop,up);
    QND_RXSeekCHInit(db);
    do
    {
        temp = QND_RXSeekCH(pStart, pStop, step, db, up);
        if (temp)
        {
            qnd_ChList[qnd_ChCount++] = temp;
        }
        else
        {
            temp = pStop;
        }
        pStart = temp + (up ? stepValue : -stepValue);
    }
    while((up ? (pStart<=pStop):(pStart>=pStop)) && (qnd_ChCount < QN_CCA_MAX_CH));
    QND_TuneToCH((qnd_ChCount >= 1)? qnd_ChList[0] : pStop);
    qnd_AutoScanAll = 0;
    return qnd_ChCount;
}

/************************************************************************
void QND_RXConfigAudio(UINT8 optiontype, UINT8 option )
*************************************************************************
Description: config audio
Parameters:
  optiontype: option
    QND_CONFIG_MONO; ¡®option¡¯control mono, 0: stereo receive mode ,1: mono receiver mode
    QND_CONFIG_MUTE; ¡®option¡¯control mute, 0:mute disable,1:mute enable
    QND_CONFIG_VOLUME: 'option' control the volume gain,range : 0~47(-47db~0db)

Return Value:
    none
**********************************************************************/
void QND_RXConfigAudio(UINT8 optiontype, UINT8 option )
{
    UINT8 regVal;
    UINT8 muteFlag = 0;

    switch(optiontype)
    {
        case QND_CONFIG_MONO:
            if (option)
                QNF_SetRegBit(SYSTEM2,RX_MONO,RX_MONO);
            else
                QNF_SetRegBit(SYSTEM2,RX_MONO,0x00);
            break;
        case QND_CONFIG_MUTE:
            if (option)
                QNF_SetMute(1);
            else
                QNF_SetMute(0);
            break;
        case QND_CONFIG_VOLUME:
            if (option > 47)
            {
                option = 47;
            }
            //audio is muted when the volume is adjusted to minimum
            if (option == 0)
            {
                //mute audio
                QNF_SetRegBit(SYSTEM2, RX_MUTE, 0x08);
                muteFlag = 1;
            }
            if(muteFlag)
            {
                // disable mute audio
                QNF_SetRegBit(SYSTEM2, RX_MUTE, 0x00);
                muteFlag = 0;
            }
            regVal = (UINT8)(option/6);
            //set analog gain
            QNF_SetRegBit(VOL_CTL, 0x07, regVal);
            regVal = (UINT8)(option%6);
            //set digital gain
            QNF_SetRegBit(VOL_CTL, 0x38, (UINT8)((5-regVal)<<3));
            break;
        default:
            break;
    }
}

/**********************************************************************
UINT16 QND_TXClearChannelScan(UINT16 start, UINT16 stop, UINT16 step,UINT8 db)
**********************************************************************
Description:   Find the best clear channel for transmission,must call
                   QND_SetSysMode(QND_MODE_FM|QND_MODE_TX)function before call this function.

Parameters:
  Start
    Set the frequency (10kHz) where scan will be started,
    e.g. 7600 for 76.00MHz
  Stop
    Set the frequency (10kHz) where scan will be stopped,
    e.g. 10800 for 108.00MHz
  Step
    QND_FSTEP_100KHZ: Set leap step to 100kHz.
    QND_FSTEP_200KHZ: Set leap step to 200kHz.
    QND_FSTEP_50KHZ:  Set leap step to 50kHz.
  db
   is not used.
Return Value:
  The channel frequency (unit: 10kHz)
**********************************************************************/
UINT16 QND_TXClearChannelScan(UINT16 start, UINT16 stop, UINT8 step,UINT8 db)
{
    UINT8 regValue;
    UINT8 timeOut;
    UINT16 ch,freq;

    timeOut = CCA_TIME_OUT;
    QNF_SetRegBit(CCA_NCNT1, CCA_NAGC, CCA_NAGC_10MS);
    QNF_ConfigScan(start, stop, step);
    //enable channel scan mode,tx channel index is decided by internal CCS.
    QNF_SetRegBit(SYSTEM1, TXCCS_MASK, TX_CCS);
    do
    {
        regValue = QND_ReadReg(SYSTEM1);
        QND_Delay(100);   //delay 100ms
        timeOut--;
        //TRACE("CHSC:0x%02x timeOut:%d \n",regValue&CHSC,timeOut);
    } while((regValue & CHSC) && timeOut);//when it seeks a channel or the time is overtime,the loop is quited
    ch = (UINT16)QND_ReadReg(INT_CTRL) & 0x03;
    ch <<= 8;
    ch |= (UINT16)QND_ReadReg(TXCH);
    freq = CHREG2FREQ(ch);
    return freq;
}

/**********************************************************************
void QND_TXSetPower( UINT8 gain)
**********************************************************************
Description:    Setting the PA output power.

Parameters:
    gain: the range of gain is 0~32.
    notes:the real output power is:0.84*(gain+24)+71.5,unit:dBuv
**********************************************************************/
void QND_TXSetPower( UINT8 gain)
{
    UINT8 value;

	if(gain > 32)
	{
		gain = 32;
	}
    value = gain + 24;
    QNF_SetRegBit(PAC,PA_TRGT,value);
    QNF_SetRegBit(0x76, 0x80, 0x80);
    QNF_SetRegBit(0x76, 0x80, 0x00);
    QND_Delay(20);
}

/**********************************************************************
void QND_TXConfigAudio(UINT8 optiontype, UINT8 option )
**********************************************************************
Description: Config the TX audio (e.g. mono/stereo,mute,etc)
Parameters:
  optiontype:option     :
    QND_CONFIG_MONO     : ¡®option¡¯control mono, 0: stereo transmit mode,1: mono transmit mode.
    QND_CONFIG_MUTE     : ¡®option¡¯control mute, 0:mute disable,1:mute enable.
    QND_CONFIG_SOFTCLIP :¡®option¡¯set softclip,0:disable soft clip, 1:enable soft clip.
    QND_CONFIG_AGCGAIN  :¡®option¡¯set AGC gain, range:0~5
Return Value:
  none
**********************************************************************/
void QND_TXConfigAudio(UINT8 optiontype, UINT8 option )
{
    switch(optiontype)
    {
    case QND_CONFIG_MONO:
        if (option)
            QNF_SetRegBit(SYSTEM2,TX_MONO,TX_MONO); //tx mono mode
        else
            QNF_SetRegBit(SYSTEM2,TX_MONO,0x00); //tx stereo mode
        break;
    case QND_CONFIG_MUTE:
        if (option)
            QNF_SetRegBit(SYSTEM2,TX_MUTE,TX_MUTE); //tx audio mute enable
        else
            QNF_SetRegBit(SYSTEM2,TX_MUTE,0x00); //tx audio mute disable
        break;
    case QND_CONFIG_SOFTCLIP:
        if (option)
            QNF_SetRegBit(REG_VGA,TX_SFTCLPEN,TX_SFTCLPEN); //enable tx soft clipping.
        else
            QNF_SetRegBit(REG_VGA,TX_SFTCLPEN,0x00); //disable tx soft clipping.
        break;
    case QND_CONFIG_AGCGAIN:
        QNF_SetRegBit(TXAGC1,TXAGC_GAIN_SEL,TXAGC_GAIN_SEL);//TXAGC_GVGA by user control
        QNF_SetRegBit(REG_VGA,TXAGC_GVGA,(UINT8)(option<<4));
        break;
    default:
        break;
    }
}
#ifdef  __cplusplus
}
#endif
