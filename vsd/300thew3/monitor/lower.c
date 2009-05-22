/************************************************************************/
/*      H8/300H Monitor Program (Normal Mode)           Ver. 2.0A(HEW3) */
/*              Copyright (C) 2003 Renesas Technology Corp.             */
/************************************************************************/
#include <no_float.h>                           /* no_float.h           */
#include <stdio.h>                              /* stdio.h              */
struct st_sci {                                 /* struct SCI           */
              char              wk[3];          /* SMR,BRR,SCR          */
              unsigned char     TDR;            /* TDR                  */
              union {                           /* SSR                  */
                    unsigned char BYTE;         /*  Byte Access         */
                    struct {                    /*  Bit  Access         */
                           unsigned char TDRE:1;/*    TDRE              */
                           unsigned char RDRF:1;/*    RDRF              */
                           }      BIT;          /*                      */
                    }           SSR;            /*                      */
              unsigned char     RDR;            /* RDR                  */
} ;                                             /*                      */
void * const _freeptr = 0;                      /* _freeptr             */
volatile int _errno;                            /* _errno               */
unsigned char sml_buf[4];                       /* buffer               */
struct { char RD,WT,CNT[24]; } BUF;             /* scanf buffer         */
FILE _iob[2];                                   /* FILE pointer         */
char *sbrk(size_t size)                         /* sbrk function        */
{                                               /*                      */
  return (char *)-1;                            /* dummy                */
}                                               /*                      */
int write(int fileno,char *buf,unsigned int cnt)/* Lower of printf      */
{                                               /*                      */
static int (* const func)(const char *,...) = printf; /* Include printf */
volatile struct st_sci *SCI = __sectop("SCI");  /* SCI Register Address */
  if( !cnt )                                    /* Check Data Count     */
    return 0;                                   /* Nothing              */
  if( *buf == '\n' )  {                         /* Send Data is '\n' ?  */
    while( !SCI->SSR.BIT.TDRE )  ;              /* Wait TDR Empty       */
    SCI->TDR = '\r';                            /* Set Send Data ('\r') */
    }                                           /*                      */
  else if( *buf == '\b' )  {                    /* Send Data is '\b' ?  */
    while( !SCI->SSR.BIT.TDRE )  ;              /* Wait TDR Empty       */
    SCI->TDR = '\b';                            /* Set Send Data ('\b') */
    while( !SCI->SSR.BIT.TDRE )  ;              /* Wait TDR Empty       */
    SCI->TDR = ' ';                             /* Set Send Data (' ')  */
    }                                           /*                      */
  while( !SCI->SSR.BIT.TDRE )  ;                /* Wait TDR Empty       */
  SCI->TDR = *buf;                              /* Set Send Data        */
  return 1;                                     /* return               */
}                                               /*                      */
int read(int fileno,char *buf,unsigned int cnt) /* Lower of scanf       */
{                                               /*                      */
static int (* const func)(const char *,...) = scanf; /* Include scanf   */
volatile struct st_sci *SCI = __sectop("SCI");  /* SCI Register Address */
  if( !cnt )                                    /* Check Data Count     */
    return 0;                                   /* Nothing              */
  if( BUF.RD == BUF.WT )                        /* Nothing Data ?       */
    do {                                        /*                      */
      while( !SCI->SSR.BIT.RDRF )               /* Wait Normal Receive  */
        SCI->SSR.BYTE &= 0xC0;                  /* Clear Error Flag     */
      *buf = SCI->RDR;                          /* Set Receive Data     */
      switch( *buf )                            /* Receive Data ?       */
        {                                       /*                      */
        case '\b' :                             /* Data is '\b' ?       */
          if( BUF.RD == BUF.WT )                /* Nothing Exist Data ? */
            continue;                           /* Next Receive         */
          BUF.WT--;                             /* Adjust Write Pointer */
          break;                                /*                      */
        case '\r' :                             /* Data is '\r' ?       */
            BUF.CNT[BUF.WT++] = *buf = '\n';    /* Change Receive Data  */
            BUF.CNT[BUF.WT++] = '\r';           /* Set Receive Data     */
          break;                                /*                      */
        default :                               /*                      */
          if( BUF.WT == 22 )                    /* Full Buffer ?        */
            *buf = '\a';                        /* Set Send Data '\a'   */
          else                                  /*                      */
            BUF.CNT[BUF.WT++] = *buf;           /* Set Receive Data     */
        }                                       /*                      */
      write( fileno, buf, cnt );                /* Echo Back            */
      } while( *buf != '\n' )  ;                /* Receive Data '\n' ?  */
  *buf = BUF.CNT[BUF.RD++];                     /* Set Receive Data     */
  if( BUF.RD == BUF.WT )                        /* Nothing Exist Data ? */
    BUF.RD = BUF.WT = 0;                        /* Reset buffer         */
  return 1;                                     /* return               */
}                                               /*                      */
