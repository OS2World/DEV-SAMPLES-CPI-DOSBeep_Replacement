#define INCL_PM
#define INCL_BASE
#include <os2.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <builtin.h>

#define INCL_OS2MM
#include <os2me.h>

#define M_PI    3.14159265358979323846

typedef ULONG (APIENTRY FN_SPIGETHANDLER)(PSZ,HID*,HID*);
typedef FN_SPIGETHANDLER *PFN_SPIGETHANDLER;

typedef ULONG (APIENTRY FN_SPIGETPROTOCOL)(HID,PSPCBKEY,PSPCB);
typedef FN_SPIGETPROTOCOL *PFN_SPIGETPROTOCOL;

typedef ULONG (APIENTRY FN_MCISENDCOMMAND)(USHORT,USHORT,ULONG,PVOID,USHORT);
typedef FN_MCISENDCOMMAND *PFN_MCISENDCOMMAND;

typedef ULONG (APIENTRY FN_MCIGETERRORSTRING)(ULONG,PSZ,USHORT);
typedef FN_MCIGETERRORSTRING *PFN_MCIGETERRORSTRING;

typedef struct _PLAYLISTCMD
{
   ULONG opCode;
   ULONG p1;
   ULONG p2;
   ULONG p3;
}
PLAYLISTCMD, *PPLAYLISTCMD;

APIRET APIENTRY OrigDosBeep(ULONG freq,ULONG dur);
APIRET16 APIENTRY16 OrigDos16Beep(USHORT freq,USHORT dur);
BOOL APIENTRY CommonBeep(ULONG freq,ULONG dur);

HMODULE ghModMDM=NULLHANDLE;
HMODULE ghModSSM=NULLHANDLE;
#ifdef __cplusplus
PCSZ
#else
PSZ
#endif
gpszMMBase=NULL;
PFN_SPIGETHANDLER gSpiGetHandler=NULL;
PFN_SPIGETPROTOCOL gSpiGetProtocol=NULL;
PFN_MCISENDCOMMAND gmciSendCommand=NULL;
PFN_MCIGETERRORSTRING gmciGetErrorString=NULL;


#ifdef __cplusplus
extern "C" {
#endif
int _CRT_init(void);
void _CRT_term(void);
void __ctordtorInit(void);
void __ctordtorTerm(void);
#ifdef __cplusplus
}
#endif

ULONG APIENTRY _DLL_InitTerm(ULONG hMod,ULONG flag)
{
   APIRET rc=NO_ERROR;
   CHAR error[CCHMAXPATH];

   if (flag == 0)
   {
      if (_CRT_init())
      {
         return 0;
      }

#ifdef __cplusplus
      __ctordtorInit();
#endif
        /*
         * MMPM is very sensitive to the environment var MMBASE being set
         * if it is not, various MMPM DLLs will crash and bring down the whole
         * system without further notice. Protect ourselves by doing a runtime (dynamic)
         * load of MDM.DLL instead of doing a loadtime load
         * if MMBASE is not set, abort the whole DLL load
         */
        if (NO_ERROR != DosScanEnv("MMBASE",&gpszMMBase))
        {
            return 0;
        }

        if (NO_ERROR != DosLoadModule(error,sizeof(error),"MDM",&ghModMDM))
        {
            return 0;
        }

        if (NO_ERROR != DosLoadModule(error,sizeof(error),"SSM",&ghModSSM))
        {
            DosFreeModule(ghModMDM);
            return 0;
        }
        rc = DosQueryProcAddr(ghModSSM,3,NULL,(PFN *)&gSpiGetHandler);
        rc = DosQueryProcAddr(ghModSSM,14,NULL,(PFN *)&gSpiGetProtocol);

        rc = DosQueryProcAddr(ghModMDM,1,NULL,(PFN *)&gmciSendCommand);
        rc = DosQueryProcAddr(ghModMDM,3,NULL,(PFN *)&gmciGetErrorString);

        return 1;
   }
   else if (flag == 1)
   {
       rc = DosFreeModule(ghModSSM);
       rc = DosFreeModule(ghModMDM);

#ifdef __cplusplus
      __ctordtorTerm();
#endif

      _CRT_term();

      return 1;
   }
   else
   {
      return 0;
   }
}


APIRET APIENTRY ShowError(PSZ msg)
{
    PTIB ptib=NULL;
    PPIB ppib=NULL;
    ULONG pibType=0;
    HAB hab = NULLHANDLE;
    HMQ hmq = NULLHANDLE;

    /*
     * need to morph the application type to a PM process
     * to display a message box
     * a message box requires a message queue which can only
     * be created for a process of PM type
     * cannot know beforehand if this DLL is called from a PM process
     * or a VIO process
     */
    DosGetInfoBlocks(&ptib,&ppib);
    pibType = ppib->pib_ultype;
    if (PT_PM != pibType)
    {
        ppib->pib_ultype = PT_PM;
    }
    hab = WinInitialize(0);
    hmq = WinCreateMsgQueue(hab,0);

    WinMessageBox(HWND_DESKTOP,NULLHANDLE,msg,"Error",1,MB_ERROR | MB_OK);

    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);

    /*
     * revert back the process type to what it was originally
     */
    ppib->pib_ultype = pibType;

    return NO_ERROR;
}

APIRET16 APIENTRY16 Dos16Beep(USHORT freq,USHORT dur)
{
    if (CommonBeep(freq,dur))
    {
        return (APIRET16)NO_ERROR;
    }
    return OrigDos16Beep(freq,dur);
}

APIRET APIENTRY DosBeep(ULONG freq,ULONG dur)
{
    if (CommonBeep(freq,dur))
    {
        return NO_ERROR;
    }
    return OrigDosBeep(freq,dur);
}

BOOL APIENTRY CommonBeep(ULONG freq,ULONG dur)
{
    #define MCI_ERROR_LENGTH 128
    CHAR errorBuffer[MCI_ERROR_LENGTH]={0};
    MCI_OPEN_PARMS mop={0};
    MCI_WAVE_SET_PARMS mwp={0};
    MCI_PLAY_PARMS mpp={0};
    MCI_GENERIC_PARMS mgp={0};
    ULONG rc;
    CHAR pszFileName[CCHMAXPATH]={0};
    PLAYLISTCMD playList[2]={0};
    PSHORT pBuffer = NULL;
    int end=0;
    SPCBKEY key={DATATYPE_WAVEFORM,WAVE_FORMAT_4S16,0};
    SPCB spcb={0};

    do
    {
        ULONG ulNumSamples = 0;
        ULONG ulMinNumSamples = 0;
        ULONG ulNumBytes = 0;
        ULONG i=0;
        double sampleValue = 0.0;
        HID hidTarget = NULLHANDLE,hidUnused = NULLHANDLE;

        /*
         * query stream manager parameters for the given wave format:
         * 44100 Hz, 2 channels, 16-bit
         */
        gSpiGetHandler("AUDIOSH$",&hidUnused,&hidTarget);
        gSpiGetProtocol(hidTarget,&key,&spcb);

        /*
         * these are the frequency boundaries defined by the original
         * DosBeep function
         */
        if (freq < 0x25 || freq > 0x7fff)
        {
            strcpy(errorBuffer,"Frequency out of the allowed range (37 to 32767 Hz).");
            ShowError(errorBuffer);
            break;
        }

        /*
         * define a sensible time limit:
         * limit the time span to 1 min = 60 s = 60000 ms
         */
        dur = min(dur,60000UL);

        /*
         * the stream manager needs a MINIMUM number of bytes to play,
         * the limits stem from the settings in SPI.INI
         * what is relevant is the number of bytes per base time unit
         * and the minimum number of buffers to play
         */
        ulNumSamples    = (ULONG)floor(dur*44100.0/1000.0);
        ulMinNumSamples = spcb.ulBytesPerUnit*spcb.ulMinBuf/spcb.ulBlockSize;
        ulNumSamples    = max(ulNumSamples,ulMinNumSamples);
        ulNumBytes      = ulNumSamples*4;

        pBuffer = (PSHORT)malloc(ulNumBytes);
        if (!pBuffer)
        {
            strcpy(errorBuffer,"Could not allocate memory for sample creation.");
            ShowError(errorBuffer);
            break;
        }

        /*
         * create a sine wave with the given freq for the given (potentially limited) dur
         * create a 44100 Hz, 2 channels, 16-bit PCM stream
         */
        for (i=0;i<ulNumSamples;i++)
        {
            double angle = 2*M_PI*freq*(i+0.5)/44100;

            angle = fmod(angle,2*M_PI);
            sampleValue = SHRT_MAX * _fsin(angle);
            if (sampleValue >= 1.0*SHRT_MAX)
            {
                sampleValue = 1.0*SHRT_MAX;
            }
            pBuffer[2*i]   = (SHORT)sampleValue;
            pBuffer[2*i+1] = (SHORT)sampleValue;
        }

        playList[0].opCode = DATA_OPERATION;
        playList[0].p1     = (ULONG)pBuffer;
        playList[0].p2     = ulNumBytes;
        playList[0].p3     = 0;

        playList[1].opCode = EXIT_OPERATION;

        mop.pszElementName = (PSZ)playList;
        mop.pszDeviceType = (PSZ)MAKEULONG(MCI_DEVTYPE_WAVEFORM_AUDIO, 0);
        rc = gmciSendCommand(0,MCI_OPEN,MCI_WAIT|MCI_OPEN_PLAYLIST|MCI_OPEN_TYPE_ID,&mop,0);
        if (LOUSHORT(rc) != MCIERR_SUCCESS)
        {
            gmciGetErrorString(rc,errorBuffer,MCI_ERROR_LENGTH);
            ShowError(errorBuffer);
            break;
        }

        mwp.usFormatTag      = MCI_WAVE_FORMAT_PCM;
        mwp.usChannels       = 2;
        mwp.ulSamplesPerSec  = 44100;
        mwp.usBitsPerSample  = 16;
        mwp.ulAvgBytesPerSec = 44100 * 4;
        mwp.ulTimeFormat     = MCI_FORMAT_BYTES;
        rc = gmciSendCommand(   mop.usDeviceID,
                                MCI_SET,
                                MCI_WAIT|
                                MCI_SET_TIME_FORMAT |
                                MCI_WAVE_SET_FORMATTAG|
                                MCI_WAVE_SET_CHANNELS|
                                MCI_WAVE_SET_SAMPLESPERSEC|
                                MCI_WAVE_SET_AVGBYTESPERSEC|
                                MCI_WAVE_SET_BITSPERSAMPLE,
                                &mwp,
                                0);
        if (LOUSHORT(rc) != MCIERR_SUCCESS)
        {
            gmciGetErrorString(rc,errorBuffer,MCI_ERROR_LENGTH);
            ShowError(errorBuffer);
            break;
        }

        memset(&mpp,0,sizeof(mpp));
        rc = gmciSendCommand(mop.usDeviceID,MCI_PLAY,MCI_WAIT,&mpp,0);
        if (LOUSHORT(rc) != MCIERR_SUCCESS)
        {
            gmciGetErrorString(rc,errorBuffer,MCI_ERROR_LENGTH);
            ShowError(errorBuffer);
            break;
        }

        memset(&mgp,0,sizeof(mgp));
        rc = gmciSendCommand(mop.usDeviceID,MCI_STOP,MCI_WAIT,&mgp,0);

        memset(&mgp,0,sizeof(mgp));
        rc = gmciSendCommand(mop.usDeviceID,MCI_CLOSE,MCI_WAIT,&mgp,0);

        free(pBuffer);

        return TRUE;
    } while (end);
    free(pBuffer);
    /*
     * fall back to the original routine in case anything goes wrong
     */
    return FALSE;
}

