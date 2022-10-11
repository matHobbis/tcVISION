// TCSUTILS
//
// tcSCRIPT exit utilities (tcVISION related)
//-----------------------------------------------------------------------------

#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include "tcsexit.h"

#ifdef _WINDOWS
  #include <windows.h>
#endif

#define DEZPUNKTchar   "."

unsigned char const IBM1047_CP1252[256] = {
  0,   1,   2,   3, 135,  9,  134, 127, /*   0 -   7  */
159, 139, 147,  11,  12,  13,  14,  15, /*   8 -  15  */
 16,  17,  18,  19, 136, 154,   8, 138, /*  16 -  23  */
 24,  25, 150, 144,  28,  29,  30,  31, /*  24 -  31  */
143, 128, 129, 132, 153,  10,  23,  27, /*  32 -  39  */
130, 157, 131, 156, 146,   5,   6,   7, /*  40 -  47  */
140, 152,  22, 141, 137, 145, 133,   4, /*  48 -  55  */
151, 155, 142, 148,  20,  21, 149,  26, /*  56 -  63  */
 32, 160, 226, 228, 224, 225, 227, 229, /*  64 -  71  */
231, 241, 162,  46,  60,  40,  43, 124, /*  72 -  79  */
 38, 233, 234, 235, 232, 237, 238, 239, /*  80 -  87  */
236, 223,  33,  36,  42,  41,  59,  94, /*  88 -  95  */
 45,  47, 194, 196, 192, 193, 195, 197, /*  96 - 103  */
199, 209, 166,  44,  37,  95,  62,  63, /* 104 - 111  */
248, 201, 202, 203, 200, 205, 206, 207, /* 112 - 119  */
204,  96,  58,  35,  64,  39,  61,  34, /* 120 - 127  */
216,  97,  98,  99, 100, 101, 102, 103, /* 128 - 135  */
104, 105, 171, 187, 240, 253, 254, 177, /* 136 - 143  */
176, 106, 107, 108, 109, 110, 111, 112, /* 144 - 151  */
113, 114, 170, 186, 230, 184, 198, 164, /* 152 - 159  */
181, 126, 115, 116, 117, 118, 119, 120, /* 160 - 167  */
121, 122, 161, 191, 208,  91, 222, 174, /* 168 - 175  */
172, 163, 165, 183, 169, 167, 182, 188, /* 176 - 183  */
189, 190, 221, 168, 175,  93, 180, 215, /* 184 - 191  */
123,  65,  66,  67,  68,  69,  70,  71, /* 192 - 199  */
 72,  73, 173, 244, 246, 242, 243, 245, /* 200 - 207  */
125,  74,  75,  76,  77,  78,  79,  80, /* 208 - 215  */
 81,  82, 185, 251, 252, 249, 250, 255, /* 216 - 223  */
 92, 247,  83,  84,  85,  86,  87,  88, /* 224 - 231  */
 89,  90, 178, 212, 214, 210, 211, 213, /* 232 - 239  */
 48,  49,  50,  51,  52,  53,  54,  55, /* 240 - 247  */
 56,  57, 179, 219, 220, 217, 218, 158, /* 248 - 255  */
};

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

#ifdef _WINDOWS
struct timezone
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag = 0;

  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);

    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    tmpres /= 10;  /*convert into microseconds*/
    /*converting file time to unix epoch*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS;
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }

  if (NULL != tz)
  {
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }

  return 0;
}
#endif

void write_message(FILE *pOutputFile, const char * format, ...)
{
  va_list args;
  
  if (NULL != pOutputFile)
  {
    char pszBuffer[LOGBUFFERSIZE];
  
#ifdef _WINDOWS
    SYSTEMTIME st;
    struct timeval currenttime;
    
    GetLocalTime ( &st);
    gettimeofday ( &currenttime, NULL);
    
    snprintf ( pszBuffer, LOGBUFFERSIZE, "%.4d-%.2d-%.2d-%.2d.%.2d.%.2d.%.6lu ",
               st.wYear, st.wMonth,  st.wDay,
               st.wHour, st.wMinute, st.wSecond,
               currenttime.tv_usec);
    fwrite ( pszBuffer, strlen( pszBuffer), 1, pOutputFile);
#else
    struct timeval currenttime;
    struct timeb timebstruct;
    struct tm lotime = { 0 };
    
    ftime(&timebstruct);
    localtime_r(&timebstruct.time, &lotime);
    gettimeofday(&currenttime, NULL);
    snprintf(pszBuffer, LOGBUFFERSIZE, "%.4d-%.2d-%.2d-%.2d.%.2d.%.2d.%.6lu ",
             lotime.tm_year + 1900, lotime.tm_mon + 1, lotime.tm_mday,
             lotime.tm_hour, lotime.tm_min, lotime.tm_sec,
             (unsigned long int) currenttime.tv_usec);
    
    fwrite(pszBuffer, strlen(pszBuffer), 1, pOutputFile);
#endif

    memset(pszBuffer, 0, LOGBUFFERSIZE);
    if (format)
    {
      va_start(args, format);
      vsprintf(pszBuffer, format, args);
      va_end(args);
    }
    
    fwrite(pszBuffer, strlen(pszBuffer), 1, pOutputFile);
    fflush(pOutputFile);
    
    // additionally to stdout...
    printf(pszBuffer);
  };

  return;
}

int print_packed_decimal(unsigned char* buf, int nPrecision, int nScale, unsigned char* szOutput)
{
  int  nFieldLength = nPrecision / 2 + 1;
  int  nCount       = 0;
  int  nDotPos      = nPrecision - nScale;
  char tmpbuffer[3] = { 0 };
  char szTmp[100]   = { 0 };
  
  if ((buf[nFieldLength - 1] & 0x0F) == 0x0D)
    szTmp[0] = '-';

  while (nCount < nFieldLength - 1)
  {
    if (0 != (nPrecision % 2) || (0 != nCount))
    {
      if (0 == nDotPos--)
        strcat(szTmp, DEZPUNKTchar);
      snprintf(tmpbuffer, 3, "%c", buf[nCount] >> 4 | '0');
      strcat(szTmp, tmpbuffer);
    };
    
    if (0 == nDotPos--)
      strcat(szTmp, DEZPUNKTchar);
    snprintf(tmpbuffer, 3, "%c", (buf[nCount] & 0x0F) | '0');
    strcat(szTmp, tmpbuffer);
    nCount++;
  };
  
  // last digit
  if (0 == nDotPos--)
    strcat(szTmp, DEZPUNKTchar);
  snprintf(tmpbuffer, 3, "%c", buf[nCount] >> 4 | '0');
  
  strcat(szTmp, tmpbuffer);
  strcpy((char*) szOutput, szTmp);
  
  return nFieldLength;
}
;

void dump_hex_data(FILE* f, unsigned char* data, int len, char* prefix)
{
  int i;
  int j;
  const int bytesPerLine = 16;
  char *szLine;
  char *szBuffer;
  
  if (len <= 0)
    return;
  
  szLine = (char*) malloc(500);
  if (!szLine)
    return;
  
  szBuffer = (char*) malloc(300);
  if (!szBuffer)
    return;
  
  memset(szLine, 0, 500);
  memset(szBuffer, 0, 300);
  
  for (i = 0; i < len; i += bytesPerLine)
  {
    memset(szLine, 0, sizeof(szLine));
    
    snprintf(szBuffer, 300, "%s %04x  ", prefix, i);
    strcat(szLine, szBuffer);
    
    for (j = i; j < len && (j - i) < bytesPerLine; j++)
    {
      snprintf(szBuffer, 300, "%02x ", data[j]);
      strcat(szLine, szBuffer);
    }
    
    for (; 0 != (j % bytesPerLine); j++)
      strcat(szLine, "   ");
    
    strcat(szLine, "  |");
    
    for (j = i; j < len && (j - i) < bytesPerLine; j++)
    {
      snprintf(szBuffer, 300, "%c", (isprint(data[j]) && data[j] != '%') ? data[j] : '.');
      strcat(szLine, szBuffer);
    }
    
    for (; 0 != (j % bytesPerLine); j++)
      strcat(szLine, " ");
    
    strcat(szLine, "|");
    
    for (j = i; j < len && (j - i) < bytesPerLine; j++)
    {
      unsigned char x = IBM1047_CP1252[data[j]];
      snprintf(szBuffer, 300, "%c", (isprint(x) && x != '%') ? x : '.');
      strcat(szLine, szBuffer);
    };
    
    for (; 0 != (j % bytesPerLine); j++)
      strcat(szLine, " ");
    
    strcat(szLine, "|\n");
    
    // write
    if (f)
      fprintf(f, szLine);
    else
      printf(szLine);
  };
  
  free(szLine);
  free(szBuffer);
  
  return;
}
;

#ifdef _WINDOWS
  #if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) || defined(__WATCOMC__)
    #define DELTA_EPOCH_IN_USEC  11644473600000000Ui64
  #else
    #define DELTA_EPOCH_IN_USEC  11644473600000000ULL
  #endif
  static unsigned _int64 filetime_to_unix_epoch (const FILETIME *ft)
  {
    unsigned _int64 res = ( unsigned _int64) ft->dwHighDateTime << 32;
    
    res |= ft->dwLowDateTime;
    res /= 10;                   /* from 100 nano-sec periods to usec */
    res -= DELTA_EPOCH_IN_USEC;  /* from Win epoch to Unix epoch */
    return ( res);
  }
  int gettimeofday (struct timeval *tv, void *tz)
  {
    FILETIME        ft;
    unsigned _int64 tim;
    
    if ( !tv) 
    return (-1);
    
    GetSystemTimeAsFileTime ( &ft);
    
    tim = filetime_to_unix_epoch ( &ft);
    
    tv->tv_sec  = (long) (tim / 1000000L);
    tv->tv_usec = (long) (tim % 1000000L);
    
    return (0);
  }
#endif

int DisplayTo_S390_Float (unsigned char *pbyteChar, unsigned char *pbyteDouble)
{
  double d = atof((const char *) pbyteChar);
  double dFraction;
  double dMult;
  double dTemp;
  unsigned long  ulFractionDigit;
  char   charExponent;
  unsigned char  Negative = d < 0;
  
  memset((void*) pbyteDouble, 0, 8);

  if (Negative)
    d *= -1;

  charExponent = (char) (log(d) / log((double)16)) + 1;
  
  pbyteDouble[0] = (unsigned char) ((long) charExponent + 64);
  if (Negative)
    pbyteDouble[0] |= 0x80;

  dFraction = (d / pow((double)16, (int)charExponent));

  ulFractionDigit = 1;
  dMult           = 1;

  while (dFraction && (ulFractionDigit < 15))
  {
    dMult *= 16;

    dTemp = dFraction * dMult;

    if (ulFractionDigit % 2)
      pbyteDouble[((ulFractionDigit - 1) / 2) + 1] |= ((unsigned char) dTemp) << 4;
    else
      pbyteDouble[((ulFractionDigit - 1) / 2) + 1] |= (unsigned char) dTemp;

    dFraction = (dTemp - (double) (unsigned long) dTemp) / dMult;

    ulFractionDigit++;
  }
  return(0);
}

int DisplayTo_S390_Packed (char *pbyteChar, unsigned short usPrecision, unsigned short usScale, unsigned char *pbytePacked, char DecimalCharacter)
{
  char szTemp[100];
  char szChar[100];
  unsigned short us;
  char *psz1;
  char *psz; 
  char *epsz;
  short sign;
  int len;
  int scal;
  short exp;
  
  if ((usPrecision % 2) == 0)
    usPrecision++;
  
  memset((void *) szTemp, 0, sizeof(szTemp));
  strcpy(szChar, pbyteChar);
  //stpcvt((char *) szChar, RWHITE|TOUP);
  epsz = strchr((char *) szChar, 'E');

  sign = 0;
  if (psz = strchr((char *) szChar, '-'))
  {
    if (!epsz || psz < epsz)
    {
      sign = -1;
	  *psz = ' ';
    }
  }
  if (psz = strchr((char *) szChar, '+'))
  {
    if (!epsz || psz < epsz)
  	  *psz = ' ';
  }

  psz1 = (char*) szChar;
  while (*psz1 == '0' || *psz1 == ' ')
	  psz1++;

  //Check for scientific
  if (epsz)
  {
      exp = atoi(epsz + 1);
      *epsz = 0;
  }
  else
      exp = 0;

  if (psz = strchr((char *) psz1, DecimalCharacter))
  {
	  len = (int) (usPrecision + 1 - usScale);
	  *psz++ = 0;
	  if (exp >= 0 && strlen(psz1) + exp > (unsigned int) (usPrecision - usScale))
		  return (1);

      len  = max(0, len  - exp);
      scal = max(0, usScale + exp);
	  sprintf(szTemp, "%0*.*s%-0*.*s0", len, len, psz1, scal, scal, psz);
  }
  else
  {
	  len = (int) (usPrecision + 1 - usScale);
	  if (strlen(psz1) > (unsigned int) (usPrecision - usScale))
		  return (1);
	  sprintf(szTemp, "%0*.*s%-0*.*s0", len, len, psz1, usScale, usScale, "0");
  }

  while (psz = strchr(szTemp , ' '))
    *psz = '0';

  for (us = 1; us <= usPrecision; us += 2)
  {
    if (szTemp[us] < '0' || szTemp[us] > '9')
        return(2);
    if (szTemp[us + 1] < '0' || szTemp[us + 1] > '9')
        return(2);

    if (szTemp[us])
      pbytePacked[us/2] = (szTemp[us] - '0') << 4;
      
    if (szTemp[us + 1])
      pbytePacked[us/2] += szTemp[us + 1] - '0';
  }

  if (sign < 0)
    pbytePacked[usPrecision / 2] |= 0x0D;
  else
    pbytePacked[usPrecision / 2] |= 0x0C;

  return 0;
}



int getendian ( void)
{
#ifdef _WINDOWS
  
  // windows ist nur für Mickey Mouse
  return(0);

#else
  
  unsigned long ul = 0x12345678UL;
  char*  psz  = (char*) &ul;
  int    n = sizeof(unsigned long);
  return(psz[n-1] == 0x78);

#endif
}

void chng8bytesBE ( unsigned char* bytes)
{
  if ( getendian())
  {
    unsigned char x;
    x = bytes[0]; bytes[0] = bytes[7]; bytes[7] = x;
    x = bytes[1]; bytes[1] = bytes[6]; bytes[6] = x;
    x = bytes[2]; bytes[2] = bytes[5]; bytes[5] = x;
    x = bytes[3]; bytes[3] = bytes[4]; bytes[4] = x;
  };
  return;
};

int print_S390_float (unsigned char* pbyteData, unsigned char*  pszOutput, int bDoublePrecision, int OutScale)
{
  char *Eptr;

  long long m;
  double    d;
  double    e;
  int       k, ende;
  int  fNegative = (( pbyteData[0] & 0x80) != 0);
  int  nExponent = (( pbyteData[0] & 0x7f) - 64);

  unsigned char* szm = (unsigned char*) &m;
  m = 0;
  ende = bDoublePrecision ? 14 : 6;
  nExponent-=ende;
  ende = bDoublePrecision ? 7 : 3;
  for ( k = 0; k < ende; k++)
  {
    szm[k] = pbyteData[(bDoublePrecision ? 7 : 3)-k];
  };

  chng8bytesBE ( szm);

  if ( nExponent >= 0)
  {
    e = pow ( 16, nExponent);
    d = (double)m * (double)e;
  }
  else
  {
    e = pow ( 16, nExponent * (-1));
    d = (double)(m / e);
  };

  if (fNegative && d)
    d*=(-1);

  if (OutScale == 0)
      OutScale = 13;
  if (OutScale > 20)
      OutScale = 20;

  if (d >= 0)
    sprintf ((char *)pszOutput, "+%.*E", OutScale, d);
  else
    sprintf ((char *)pszOutput,  "%.*E", OutScale, d);

  //sprintf(s, "+%.*E") may produce 2 digits for the exponent unless 3 didgits are required, but we want always 3 digits, so we enlarge if neccessary to a total string length of OutScale + 8 Bytes.
  //Default OutScale is 13 -> Default total string length = 21
  if (Eptr = strchr((char*)pszOutput, 'E'))
  {
      Eptr += 2;
      if (strlen(Eptr) == 2)
      {
          memmove(Eptr + 1, Eptr, 3);
          Eptr[0] = '0';
      }
  }

  return strlen((char *)pszOutput);
};


