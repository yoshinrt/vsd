/************************************************************************/
/*      H8/36037 Series Include File                       Ver 2.0      */
/************************************************************************/
struct st_tinycan {                                     /* TinyCAN      */
                  union {                               /* MCR          */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char      :6;   /*              */
                               unsigned char HLTRQ:1;   /*    HLTRQ     */
                               unsigned char RSTRQ:1;   /*    RSTRQ     */
                               }      BIT;              /*              */
                        }       MCR;                    /*              */
                  union {                               /* GSR          */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char      :2;   /*              */
                               unsigned char ERPS :1;   /*    ERPS      */
                               unsigned char HALT :1;   /*    HALT      */
                               unsigned char RESET:1;   /*    RESET     */
                               unsigned char TCMPL:1;   /*    TCMPL     */
                               unsigned char ECWRG:1;   /*    ECWRG     */
                               unsigned char BOFF :1;   /*    BOFF      */
                               }      BIT;              /*              */
                        }       GSR;                    /*              */
                  union {                               /* BCR1         */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char     :1;    /*              */
                               unsigned char TSG2:3;    /*    TSG2      */
                               unsigned char TSG1:4;    /*    TSG1      */
                               }      BIT;              /*              */
                        }       BCR1;                   /*              */
                  union {                               /* BCR0         */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char SJW:2;     /*    SJW       */
                               unsigned char BRP:6;     /*    BRP       */
                               }      BIT;              /*              */
                        }       BCR0;                   /*              */
                  union {                               /* MBCR         */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char    :4;     /*              */
                               unsigned char MB3:1;     /*    MB3       */
                               unsigned char MB2:1;     /*    MB2       */
                               unsigned char MB1:1;     /*    MB1       */
                               }      BIT;              /*              */
                        }       MBCR;                   /*              */
                  union {                               /* TCMSC        */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char MSTTC:1;   /*    MSTTC     */
                               unsigned char      :5;   /*              */
                               unsigned char PMR97:1;   /*    PMR97     */
                               unsigned char PMR96:1;   /*    PMR96     */
                               }      BIT;              /*              */
                        }       TCMSC;                  /*              */
                  union {                               /* TXPR         */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char    :4;     /*              */
                               unsigned char MB3:1;     /*    MB3       */
                               unsigned char MB2:1;     /*    MB2       */
                               unsigned char MB1:1;     /*    MB1       */
                               }      BIT;              /*              */
                        }       TXPR;                   /*              */
                  unsigned char wk1;                    /*              */
                  union {                               /* TXCR         */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char    :4;     /*              */
                               unsigned char MB3:1;     /*    MB3       */
                               unsigned char MB2:1;     /*    MB2       */
                               unsigned char MB1:1;     /*    MB1       */
                               }      BIT;              /*              */
                        }       TXCR;                   /*              */
                  unsigned char wk2;                    /*              */
                  union {                               /* TXACK        */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char    :4;     /*              */
                               unsigned char MB3:1;     /*    MB3       */
                               unsigned char MB2:1;     /*    MB2       */
                               unsigned char MB1:1;     /*    MB1       */
                               }      BIT;              /*              */
                        }       TXACK;                  /*              */
                  unsigned char wk3;                    /*              */
                  union {                               /* ABACK        */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char    :4;     /*              */
                               unsigned char MB3:1;     /*    MB3       */
                               unsigned char MB2:1;     /*    MB2       */
                               unsigned char MB1:1;     /*    MB1       */
                               }      BIT;              /*              */
                        }       ABACK;                  /*              */
                  unsigned char wk4;                    /*              */
                  union {                               /* RXPR         */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char    :4;     /*              */
                               unsigned char MB3:1;     /*    MB3       */
                               unsigned char MB2:1;     /*    MB2       */
                               unsigned char MB1:1;     /*    MB1       */
                               unsigned char MB0:1;     /*    MB0       */
                               }      BIT;              /*              */
                        }       RXPR;                   /*              */
                  unsigned char wk5;                    /*              */
                  union {                               /* RFPR         */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char    :4;     /*              */
                               unsigned char MB3:1;     /*    MB3       */
                               unsigned char MB2:1;     /*    MB2       */
                               unsigned char MB1:1;     /*    MB1       */
                               unsigned char MB0:1;     /*    MB0       */
                               }      BIT;              /*              */
                        }       RFPR;                   /*              */
                  unsigned char wk6;                    /*              */
                  union {                               /* IRR1         */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char     :3;    /*              */
                               unsigned char WUPI:1;    /*    WUPI      */
                               unsigned char     :2;    /*              */
                               unsigned char OVRI:1;    /*    OVRI      */
                               unsigned char EMPI:1;    /*    EMPI      */
                               }      BIT;              /*              */
                        }       _IRR1;                  /*              */
                  union {                               /* IRR0         */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char OVLI:1;    /*    OVLI      */
                               unsigned char BOFI:1;    /*    BOFI      */
                               unsigned char EPI :1;    /*    EPI       */
                               unsigned char ROWI:1;    /*    ROWI      */
                               unsigned char TOWI:1;    /*    TOWI      */
                               unsigned char RFRI:1;    /*    RFRI      */
                               unsigned char DFRI:1;    /*    DFRI      */
                               unsigned char RHI :1;    /*    RHI       */
                               }      BIT;              /*              */
                        }       IRR0;                   /*              */
                  union {                               /* MBIMR        */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char    :4;     /*              */
                               unsigned char MB3:1;     /*    MB3       */
                               unsigned char MB2:1;     /*    MB2       */
                               unsigned char MB1:1;     /*    MB1       */
                               unsigned char MB0:1;     /*    MB0       */
                               }      BIT;              /*              */
                        }       MBIMR;                  /*              */
                  unsigned char wk7;                    /*              */
                  union {                               /* IMR1         */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char      :3;   /*              */
                               unsigned char WUPIM:1;   /*    WUPIM     */
                               unsigned char      :2;   /*              */
                               unsigned char OVRIM:1;   /*    OVRIM     */
                               unsigned char EMPIM:1;   /*    EMPIM     */
                               }      BIT;              /*              */
                        }       IMR1;                   /*              */
                  union {                               /* IMR0         */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char OVLIM:1;   /*    OVLIM     */
                               unsigned char BOFIM:1;   /*    BOFIM     */
                               unsigned char EPIM :1;   /*    EPIM      */
                               unsigned char ROWIM:1;   /*    ROWIM     */
                               unsigned char TOWIM:1;   /*    TOWIM     */
                               unsigned char RFRIM:1;   /*    RFRIM     */
                               unsigned char DFRIM:1;   /*    DFRIM     */
                               unsigned char RHIM :1;   /*    RHIM      */
                               }      BIT;              /*              */
                        }       IMR0;                   /*              */
                  unsigned char REC;                    /* REC          */
                  unsigned char TEC;                    /* TEC          */
                  union {                               /* TCR          */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char TSTMD:1;   /*    TSTMD     */
                               unsigned char WREC :1;   /*    WREC      */
                               unsigned char FERPS:1;   /*    FERPS     */
                               unsigned char ATACK:1;   /*    ATACK     */
                               unsigned char DEC  :1;   /*    DEC       */
                               unsigned char DRXIN:1;   /*    DRXIN     */
                               unsigned char DTXOT:1;   /*    DTXOT     */
                               unsigned char INTLE:1;   /*    INTLE     */
                               }      BIT;              /*              */
                        }       TCR;                    /*              */
                  union {                               /* UMSR         */
                        unsigned char BYTE;             /*  Byte Access */
                        struct {                        /*  Bit Access  */
                               unsigned char    :4;     /*              */
                               unsigned char MB3:1;     /*    MB3       */
                               unsigned char MB2:1;     /*    MB2       */
                               unsigned char MB1:1;     /*    MB1       */
                               unsigned char MB0:1;     /*    MB0       */
                               }      BIT;              /*              */
                        }       UMSR;                   /*              */
                  unsigned char wk8[4];                 /*              */
                  unsigned char MC[4][8];               /* MC[x][y]     */
                  unsigned char MD[4][8];               /* MD[x][y]     */
                  unsigned long LAFM[4];                /* LAFM         */
};                                                      /*              */
struct st_ssu {                                         /* struct SSU   */
              union {                                   /* SSCRH        */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char MSS :1;        /*    MSS       */
                           unsigned char BIDE:1;        /*    BIDE      */
                           unsigned char SOOS:1;        /*    SOOS      */
                           unsigned char SOL :1;        /*    SOL       */
                           unsigned char SOLP:1;        /*    SOLP      */
                           unsigned char SCKS:1;        /*    SCKS      */
                           unsigned char CSS :2;        /*    CSS       */
                           }      BIT;                  /*              */
                    }           SSCRH;                  /*              */
              union {                                   /* SSCRL        */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char MSTSSU:1;      /*    MSTSSU    */
                           unsigned char SSUMS :1;      /*    SSUMS     */
                           unsigned char SRES  :1;      /*    SRES      */
                           unsigned char SCKOS :1;      /*    SCKOS     */
                           unsigned char CSOS  :1;      /*    CSOS      */
                           }      BIT;                  /*              */
                    }           SSCRL;                  /*              */
              union {                                   /* SSMR         */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char MLS :1;        /*    MLS       */
                           unsigned char CPOS:1;        /*    CPOS      */
                           unsigned char CPHS:1;        /*    CPHS      */
                           unsigned char     :2;        /*              */
                           unsigned char CKS :3;        /*    CKS       */
                           }      BIT;                  /*              */
                    }           SSMR;                   /*              */
              union {                                   /* SSER         */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit Access  */
                           unsigned char TE   :1;       /*    TE        */
                           unsigned char RE   :1;       /*    RE        */
                           unsigned char RSSTP:1;       /*    RSSTP     */
                           unsigned char      :1;       /*              */
                           unsigned char TEIE :1;       /*    TEIE      */
                           unsigned char TIE  :1;       /*    TIE       */
                           unsigned char RIE  :1;       /*    RIE       */
                           unsigned char CEIE :1;       /*    CEIE      */
                           }      BIT;                  /*              */
                    }           SSER;                   /*              */
              union {                                   /* SSSR         */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit Access  */
                           unsigned char     :1;        /*              */
                           unsigned char ORER:1;        /*    ORER      */
                           unsigned char     :2;        /*              */
                           unsigned char TEND:1;        /*    TEND      */
                           unsigned char TDRE:1;        /*    TDRE      */
                           unsigned char RDRF:1;        /*    RDRF      */
                           unsigned char CE  :1;        /*    CE        */
                           }      BIT;                  /*              */
                    }           SSSR;                   /*              */
              unsigned char     wk1[4];                 /*              */
              unsigned char     SSRDR;                  /* SSRDR        */
              unsigned char     wk2;                    /*              */
              unsigned char     SSTDR;                  /* SSTDR        */
};                                                      /*              */
struct st_sbt {                                         /* struct SBT   */
              union {                                   /* SBTCTL       */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit Access  */
                           unsigned char PCEF  :1;      /*    PCEF      */
                           unsigned char       :2;      /*              */
                           unsigned char START :1;      /*    START     */
                           unsigned char OSCEB :1;      /*    OSCEB     */
                           unsigned char SYSCKS:1;      /*    SYSCKS    */
                           unsigned char SBTIB :1;      /*    SBTIB     */
                           unsigned char SBTUF :1;      /*    SBTUF     */
                           }      BIT;                  /*              */
                    }           SBTCTL;                 /*              */
              unsigned char     SBTDCNT;                /* SBTDCNT      */
              unsigned char     ROPCR;                  /* ROPCR        */
};                                                      /*              */
struct st_tz0 {                                         /* struct TZ0   */
              union {                                   /* TCR          */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char CCLR:3;        /*    CCLR      */
                           unsigned char CKEG:2;        /*    CKEG      */
                           unsigned char TPSC:3;        /*    TPSC      */
                           }      BIT;                  /*              */
                    }           TCR;                    /*              */
              union {                                   /* TIORA        */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char    :1;         /*              */
                           unsigned char IOB:3;         /*    IOB       */
                           unsigned char    :1;         /*              */
                           unsigned char IOA:3;         /*    IOA       */
                           }      BIT;                  /*              */
                    }           TIORA;                  /*              */
              union {                                   /* TIORC        */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char    :1;         /*              */
                           unsigned char IOD:3;         /*    IOD       */
                           unsigned char    :1;         /*              */
                           unsigned char IOC:3;         /*    IOC       */
                           }      BIT;                  /*              */
                    }           TIORC;                  /*              */
              union {                                   /* TSR          */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char     :3;        /*              */
                           unsigned char OVF :1;        /*    OVF       */
                           unsigned char IMFD:1;        /*    IMFD      */
                           unsigned char IMFC:1;        /*    IMFC      */
                           unsigned char IMFB:1;        /*    IMFB      */
                           unsigned char IMFA:1;        /*    IMFA      */
                           }      BIT;                  /*              */
                    }           TSR;                    /*              */
              union {                                   /* TIER         */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char      :3;       /*              */
                           unsigned char OVIE :1;       /*    OVIE      */
                           unsigned char IMIED:1;       /*    IMIED     */
                           unsigned char IMIEC:1;       /*    IMIEC     */
                           unsigned char IMIEB:1;       /*    IMIEB     */
                           unsigned char IMIEA:1;       /*    IMIEA     */
                           }      BIT;                  /*              */
                    }           TIER;                   /*              */
              union {                                   /* POCR         */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char     :5;        /*              */
                           unsigned char POLD:1;        /*    POLD      */
                           unsigned char POLC:1;        /*    POLC      */
                           unsigned char POLB:1;        /*    POLB      */
                           }      BIT;                  /*              */
                    }           POCR;                   /*              */
              unsigned int      TCNT;                   /* TCNT         */
              unsigned int      GRA;                    /* GRA          */
              unsigned int      GRB;                    /* GRB          */
              unsigned int      GRC;                    /* GRC          */
              unsigned int      GRD;                    /* GRD          */
};                                                      /*              */
struct st_tz1 {                                         /* struct TZ1   */
              union {                                   /* TCR          */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char CCLR:3;        /*    CCLR      */
                           unsigned char CKEG:2;        /*    CKEG      */
                           unsigned char TPSC:3;        /*    TPSC      */
                           }      BIT;                  /*              */
                    }           TCR;                    /*              */
              union {                                   /* TIORA        */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char    :1;         /*              */
                           unsigned char IOB:3;         /*    IOB       */
                           unsigned char    :1;         /*              */
                           unsigned char IOA:3;         /*    IOA       */
                           }      BIT;                  /*              */
                    }           TIORA;                  /*              */
              union {                                   /* TIORC        */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char    :1;         /*              */
                           unsigned char IOD:3;         /*    IOD       */
                           unsigned char    :1;         /*              */
                           unsigned char IOC:3;         /*    IOC       */
                           }      BIT;                  /*              */
                    }           TIORC;                  /*              */
              union {                                   /* TSR          */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char     :2;        /*              */
                           unsigned char UDF :1;        /*    UDF       */
                           unsigned char OVF :1;        /*    OVF       */
                           unsigned char IMFD:1;        /*    IMFD      */
                           unsigned char IMFC:1;        /*    IMFC      */
                           unsigned char IMFB:1;        /*    IMFB      */
                           unsigned char IMFA:1;        /*    IMFA      */
                           }      BIT;                  /*              */
                    }           TSR;                    /*              */
              union {                                   /* TIER         */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char      :3;       /*              */
                           unsigned char OVIE :1;       /*    OVIE      */
                           unsigned char IMIED:1;       /*    IMIED     */
                           unsigned char IMIEC:1;       /*    IMIEC     */
                           unsigned char IMIEB:1;       /*    IMIEB     */
                           unsigned char IMIEA:1;       /*    IMIEA     */
                           }      BIT;                  /*              */
                    }           TIER;                   /*              */
              union {                                   /* TOCR         */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char     :5;        /*              */
                           unsigned char POLD:1;        /*    POLD      */
                           unsigned char POLC:1;        /*    POLC      */
                           unsigned char POLB:1;        /*    POLB      */
                           }      BIT;                  /*              */
                    }           POCR;                   /*              */
              unsigned int      TCNT;                   /* TCNT         */
              unsigned int      GRA;                    /* GRA          */
              unsigned int      GRB;                    /* GRB          */
              unsigned int      GRC;                    /* GRC          */
              unsigned int      GRD;                    /* GRD          */
};                                                      /*              */
struct st_tz {                                          /* struct TZ    */
             union {                                    /* TSTR         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char     :6;         /*              */
                          unsigned char STR1:1;         /*    STR1      */
                          unsigned char STR0:1;         /*    STR0      */
                          }      BIT;                   /*              */
                   }            TSTR;                   /*              */
             union {                                    /* TMDR         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char BFD1:1;         /*    BFD1      */
                          unsigned char BFC1:1;         /*    BFC1      */
                          unsigned char BFD0:1;         /*    BFD0      */
                          unsigned char BFC0:1;         /*    BFC0      */
                          unsigned char     :3;         /*              */
                          unsigned char SYNC:1;         /*    SYNC      */
                          }      BIT;                   /*              */
                   }            TMDR;                   /*              */
             union {                                    /* TPMR         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char      :1;        /*              */
                          unsigned char PWMD1:1;        /*    PWMD1     */
                          unsigned char PWMC1:1;        /*    PWMC1     */
                          unsigned char PWMB1:1;        /*    PWMB1     */
                          unsigned char      :1;        /*              */
                          unsigned char PWMD0:1;        /*    PWMD0     */
                          unsigned char PWMC0:1;        /*    PWMC0     */
                          unsigned char PWMB0:1;        /*    PWMB0     */
                          }      BIT;                   /*              */
                   }            TPMR;                   /*              */
             union {                                    /* TFCR         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char      :1;        /*              */
                          unsigned char STCLK:1;        /*    STCLK     */
                          unsigned char ADEG :1;        /*    ADEG      */
                          unsigned char ADTRG:1;        /*    ADTRG     */
                          unsigned char OLS1 :1;        /*    OLS1      */
                          unsigned char OLS0 :1;        /*    OLS0      */
                          unsigned char CMD  :2;        /*    CMD       */
                          }      BIT;                   /*              */
                   }            TFCR;                   /*              */
             union {                                    /* TOER         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char ED1:1;          /*    ED1       */
                          unsigned char EC1:1;          /*    EC1       */
                          unsigned char EB1:1;          /*    EB1       */
                          unsigned char EA1:1;          /*    EA1       */
                          unsigned char ED0:1;          /*    ED0       */
                          unsigned char EC0:1;          /*    EC0       */
                          unsigned char EB0:1;          /*    EB0       */
                          unsigned char EA0:1;          /*    EA0       */
                          }      BIT;                   /*              */
                   }            TOER;                   /*              */
             union {                                    /* TOCR         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char TOD1:1;         /*    TOD1      */
                          unsigned char TOC1:1;         /*    TOC1      */
                          unsigned char TOB1:1;         /*    TOB1      */
                          unsigned char TOA1:1;         /*    TOA1      */
                          unsigned char TOD0:1;         /*    TOD0      */
                          unsigned char TOC0:1;         /*    TOC0      */
                          unsigned char TOB0:1;         /*    TOB0      */
                          unsigned char TOA0:1;         /*    TOA0      */
                          }      BIT;                   /*              */
                   }            TOCR;                   /*              */
};                                                      /*              */
struct st_lvd {                                         /* struct LVD   */
              union {                                   /* LVDCR        */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char LVDE  :1;      /*    LVDE      */
                           unsigned char       :3;      /*              */
                           unsigned char LVDSEL:1;      /*    LVDSEL    */
                           unsigned char LVDRE :1;      /*    LVDRE     */
                           unsigned char LVDDE :1;      /*    LVDDE     */
                           unsigned char LVDUE :1;      /*    LVDUE     */
                           }      BIT;                  /*              */
                    }           CR;                     /*              */
              union {                                   /* LVDSR        */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char      :6;       /*              */
                           unsigned char LVDDF:1;       /*    LVDDF     */
                           unsigned char LVDUF:1;       /*    LVDUF     */
                           }      BIT;                  /*              */
                    }           SR;                     /*              */
};                                                      /*              */
struct st_sci3 {                                        /* struct SCI3  */
               union {                                  /* SMR          */
                     unsigned char BYTE;                /*  Byte Access */
                     struct {                           /*  Bit  Access */
                            unsigned char COM :1;       /*    COM       */
                            unsigned char CHR :1;       /*    CHR       */
                            unsigned char PE  :1;       /*    PE        */
                            unsigned char PM  :1;       /*    PM        */
                            unsigned char STOP:1;       /*    STOP      */
                            unsigned char MP  :1;       /*    MP        */
                            unsigned char CKS :2;       /*    CKS       */
                            }      BIT;                 /*              */
                     }          SMR;                    /*              */
               unsigned char    BRR;                    /* BRR          */
               union {                                  /* SCR3         */
                     unsigned char BYTE;                /*  Byte Access */
                     struct {                           /*  Bit  Access */
                            unsigned char TIE :1;       /*    TIE       */
                            unsigned char RIE :1;       /*    RIE       */
                            unsigned char TE  :1;       /*    TE        */
                            unsigned char RE  :1;       /*    RE        */
                            unsigned char MPIE:1;       /*    MPIE      */
                            unsigned char TEIE:1;       /*    TEIE      */
                            unsigned char CKE :2;       /*    CKE       */
                            }      BIT;                 /*              */
                     }          SCR3;                   /*              */
               unsigned char    TDR;                    /* TDR          */
               union {                                  /* SSR          */
                     unsigned char BYTE;                /*  Byte Access */
                     struct {                           /*  Bit  Access */
                            unsigned char TDRE:1;       /*    TDRE      */
                            unsigned char RDRF:1;       /*    RDRF      */
                            unsigned char OER :1;       /*    OER       */
                            unsigned char FER :1;       /*    FER       */
                            unsigned char PER :1;       /*    PER       */
                            unsigned char TEND:1;       /*    TEND      */
                            unsigned char MPBR:1;       /*    MPBR      */
                            unsigned char MPBT:1;       /*    MPBT      */
                            }      BIT;                 /*              */
                     }          SSR;                    /*              */
               unsigned char    RDR;                    /* RDR          */
};                                                      /*              */
struct st_tb1 {                                         /* struct TB1   */
              union {                                   /* TMB1         */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char RLD:1;         /*    RLD       */
                           unsigned char    :4;         /*              */
                           unsigned char CKS:3;         /*    CKS       */
                           }      BIT;                  /*              */
                    }           TMB1;                   /*              */
              unsigned char     TCB1;                   /* TCB1         */
};                                                      /*              */
struct st_flash {                                       /* struct FLASH */
                union {                                 /* FLMCR1       */
                      unsigned char BYTE;               /*  Byte Access */
                      struct {                          /*  Bit  Access */
                             unsigned char    :1;       /*              */
                             unsigned char SWE:1;       /*    SWE       */
                             unsigned char ESU:1;       /*    ESU       */
                             unsigned char PSU:1;       /*    PSU       */
                             unsigned char EV :1;       /*    EV        */
                             unsigned char PV :1;       /*    PV        */
                             unsigned char E  :1;       /*    E         */
                             unsigned char P  :1;       /*    P         */
                             }      BIT;                /*              */
                      }         FLMCR1;                 /*              */
                union {                                 /* FLMCR2       */
                      unsigned char BYTE;               /*  Byte Access */
                      struct {                          /*  Bit  Access */
                             unsigned char FLER:1;      /*    FLER      */
                             }      BIT;                /*              */
                      }         FLMCR2;                 /*              */
                union {                                 /* FLPWCR       */
                      unsigned char BYTE;               /*  Byte Access */
                      struct {                          /*  Bit  Access */
                             unsigned char PDWND:1;     /*    PDWND     */
                             }      BIT;                /*              */
                      }         FLPWCR;                 /*              */
                union {                                 /* EBR1         */
                      unsigned char BYTE;               /*  Byte Access */
                      struct {                          /*  Bit  Access */
                             unsigned char    :1;       /*              */
                             unsigned char EB6:1;       /*    EB6       */
                             unsigned char EB5:1;       /*    EB5       */
                             unsigned char EB4:1;       /*    EB4       */
                             unsigned char EB3:1;       /*    EB3       */
                             unsigned char EB2:1;       /*    EB2       */
                             unsigned char EB1:1;       /*    EB1       */
                             unsigned char EB0:1;       /*    EB0       */
                             }      BIT;                /*              */
                      }         EBR1;                   /*              */
                char            wk[7];                  /*              */
                union {                                 /* FENR         */
                      unsigned char BYTE;               /*  Byte Access */
                      struct {                          /*  Bit  Access */
                             unsigned char FLSHE:1;     /*    FLSHE     */
                             }      BIT;                /*              */
                      }         FENR;                   /*              */
};                                                      /*              */
struct st_tv {                                          /* struct TV    */
             union {                                    /* TCRV0        */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char CMIEB:1;        /*    CMIEB     */
                          unsigned char CMIEA:1;        /*    CMIEA     */
                          unsigned char OVIE :1;        /*    OVIE      */
                          unsigned char CCLR :2;        /*    CCLR      */
                          unsigned char CKS  :3;        /*    CKS       */
                          }      BIT;                   /*              */
                   }            TCRV0;                  /*              */
             union {                                    /* TCSRV        */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char CMFB:1;         /*    CMFB      */
                          unsigned char CMFA:1;         /*    CMFA      */
                          unsigned char OVF :1;         /*    OVF       */
                          unsigned char     :1;         /*              */
                          unsigned char OS  :4;         /*    OS        */
                          }      BIT;                   /*              */
                   }            TCSRV;                  /*              */
             unsigned char      TCORA;                  /* TCORA        */
             unsigned char      TCORB;                  /* TCORB        */
             unsigned char      TCNTV;                  /* TCNT         */
             union {                                    /* TCRV1        */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char     :3;         /*              */
                          unsigned char TVEG:2;         /*    TVEG      */
                          unsigned char TRGE:1;         /*    TRGE      */
                          unsigned char     :1;         /*              */
                          unsigned char ICKS:1;         /*    ICKS      */
                          }      BIT;                   /*              */
                   }            TCRV1;                  /*              */
};                                                      /*              */
struct st_ad {                                          /* struct A/D   */
             unsigned int       ADDRA;                  /* ADDRA        */
             unsigned int       ADDRB;                  /* ADDRB        */
             unsigned int       ADDRC;                  /* ADDRC        */
             unsigned int       ADDRD;                  /* ADDRD        */
             union {                                    /* ADCSR        */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char ADF :1;         /*    ADF       */
                          unsigned char ADIE:1;         /*    ADIE      */
                          unsigned char ADST:1;         /*    ADST      */
                          unsigned char SCAN:1;         /*    SCAN      */
                          unsigned char CKS :1;         /*    CKS       */
                          unsigned char CH  :3;         /*    CH        */
                          }      BIT;                   /*              */
                   }            ADCSR;                  /*              */
             union {                                    /* ADCR         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char TRGE:1;         /*    TRGE      */
                          }      BIT;                   /*              */
                   }            ADCR;                   /*              */
};                                                      /*              */
struct st_wdt {                                         /* struct WDT   */
              union {                                   /* TCSRWD       */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char B6WI  :1;      /*    B6WI      */
                           unsigned char TCWE  :1;      /*    TCWE      */
                           unsigned char B4WI  :1;      /*    B4WI      */
                           unsigned char TCSRWE:1;      /*    TCSRWE    */
                           unsigned char B2WI  :1;      /*    B2WI      */
                           unsigned char WDON  :1;      /*    WDON      */
                           unsigned char B0WI  :1;      /*    B0WI      */
                           unsigned char WRST  :1;      /*    WRST      */
                           }      BIT;                  /*              */
                    }           TCSRWD;                 /*              */
              unsigned char     TCWD;                   /* TCWD         */
              union {                                   /* TMWD         */
                    unsigned char BYTE;                 /*  Byte Access */
                    struct {                            /*  Bit  Access */
                           unsigned char    :4;         /*              */
                           unsigned char CKS:4;         /*    CKS       */
                           }      BIT;                  /*              */
                    }           TMWD;                   /*              */
};                                                      /*              */
struct st_abrk {                                        /* struct ABRK  */
               union {                                  /* ABRKCR       */
                     unsigned char BYTE;                /*  Byte Access */
                     struct {                           /*  Bit  Access */
                            unsigned char RTINTE:1;     /*    RTINTE    */
                            unsigned char CSEL  :2;     /*    CSEL      */
                            unsigned char ACMP  :3;     /*    ACMP      */
                            unsigned char DCMP  :2;     /*    DCMP      */
                            }      BIT;                 /*              */
                     }          CR;                     /*              */
               union {                                  /* ABRKSR       */
                     unsigned char BYTE;                /*  Byte Access */
                     struct {                           /*  Bit  Access */
                            unsigned char ABIF:1;       /*    ABIF      */
                            unsigned char ABIE:1;       /*    ABIE      */
                            }      BIT;                 /*              */
                     }          SR;                     /*              */
               void            *BAR;                    /* BAR          */
               unsigned int     BDR;                    /* BDR          */
};                                                      /*              */
struct st_io {                                          /* struct IO    */
             union {                                    /* PUCR1        */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char B7:1;           /*    Bit 7     */
                          unsigned char B6:1;           /*    Bit 6     */
                          unsigned char B5:1;           /*    Bit 5     */
                          unsigned char B4:1;           /*    Bit 4     */
                          unsigned char   :1;           /*    Bit 3     */
                          unsigned char B2:1;           /*    Bit 2     */
                          unsigned char B1:1;           /*    Bit 1     */
                          unsigned char B0:1;           /*    Bit 0     */
                          }      BIT;                   /*              */
                   }            PUCR1;                  /*              */
             union {                                    /* PUCR5        */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char   :2;           /*    Bit 7,6   */
                          unsigned char B5:1;           /*    Bit 5     */
                          unsigned char B4:1;           /*    Bit 4     */
                          unsigned char B3:1;           /*    Bit 3     */
                          unsigned char B2:1;           /*    Bit 2     */
                          unsigned char B1:1;           /*    Bit 1     */
                          unsigned char B0:1;           /*    Bit 0     */
                          }      BIT;                   /*              */
                   }            PUCR5;                  /*              */
             char               wk1[2];                 /*              */
             union {                                    /* PDR1         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char B7:1;           /*    Bit 7     */
                          unsigned char B6:1;           /*    Bit 6     */
                          unsigned char B5:1;           /*    Bit 5     */
                          unsigned char B4:1;           /*    Bit 4     */
                          unsigned char   :1;           /*    Bit 3     */
                          unsigned char B2:1;           /*    Bit 2     */
                          unsigned char B1:1;           /*    Bit 1     */
                          unsigned char B0:1;           /*    Bit 0     */
                          }      BIT;                   /*              */
                   }            PDR1;                   /*              */
             union {                                    /* PDR2         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char   :3;           /*    Bit 7-5   */
                          unsigned char B4:1;           /*    Bit 4     */
                          unsigned char B3:1;           /*    Bit 3     */
                          unsigned char B2:1;           /*    Bit 2     */
                          unsigned char B1:1;           /*    Bit 1     */
                          unsigned char B0:1;           /*    Bit 0     */
                          }      BIT;                   /*              */
                   }            PDR2;                   /*              */
             char               wk2[2];                 /*              */
             union {                                    /* PDR5         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char B7:1;           /*    Bit 7     */
                          unsigned char B6:1;           /*    Bit 6     */
                          unsigned char B5:1;           /*    Bit 5     */
                          unsigned char B4:1;           /*    Bit 4     */
                          unsigned char B3:1;           /*    Bit 3     */
                          unsigned char B2:1;           /*    Bit 2     */
                          unsigned char B1:1;           /*    Bit 1     */
                          unsigned char B0:1;           /*    Bit 0     */
                          }      BIT;                   /*              */
                   }            PDR5;                   /*              */
             union {                                    /* PDR6         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char B7:1;           /*    Bit 7     */
                          unsigned char B6:1;           /*    Bit 6     */
                          unsigned char B5:1;           /*    Bit 5     */
                          unsigned char B4:1;           /*    Bit 4     */
                          unsigned char B3:1;           /*    Bit 3     */
                          unsigned char B2:1;           /*    Bit 2     */
                          unsigned char B1:1;           /*    Bit 1     */
                          unsigned char B0:1;           /*    Bit 0     */
                          }      BIT;                   /*              */
                   }            PDR6;                   /*              */
             union {                                    /* PDR7         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char   :1;           /*              */
                          unsigned char B6:1;           /*    Bit 6     */
                          unsigned char B5:1;           /*    Bit 5     */
                          unsigned char B4:1;           /*    Bit 4     */
                          unsigned char   :1;           /*    Bit 3     */
                          unsigned char B2:1;           /*    Bit 2     */
                          unsigned char B1:1;           /*    Bit 1     */
                          unsigned char B0:1;           /*    Bit 0     */
                          }      BIT;                   /*              */
                   }            PDR7;                   /*              */
             union {                                    /* PDR8         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char B7:1;           /*    Bit 7     */
                          unsigned char B6:1;           /*    Bit 6     */
                          unsigned char B5:1;           /*    Bit 5     */
                          }      BIT;                   /*              */
                   }            PDR8;                   /*              */
             union {                                    /* PDR9         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char B7:1;           /*    Bit 7     */
                          unsigned char B6:1;           /*    Bit 6     */
                          unsigned char B5:1;           /*    Bit 5     */
                          unsigned char B4:1;           /*    Bit 4     */
                          unsigned char B3:1;           /*    Bit 3     */
                          unsigned char B2:1;           /*    Bit 2     */
                          unsigned char B1:1;           /*    Bit 1     */
                          unsigned char B0:1;           /*    Bit 0     */
                          }      BIT;                   /*              */
                   }            PDR9;                   /*              */
             union {                                    /* PDRB         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char B7:1;           /*    Bit 7     */
                          unsigned char B6:1;           /*    Bit 6     */
                          unsigned char B5:1;           /*    Bit 5     */
                          unsigned char B4:1;           /*    Bit 4     */
                          unsigned char B3:1;           /*    Bit 3     */
                          unsigned char B2:1;           /*    Bit 2     */
                          unsigned char B1:1;           /*    Bit 1     */
                          unsigned char B0:1;           /*    Bit 0     */
                          }      BIT;                   /*              */
                   }            PDRB;                   /*              */
             char               wk3[2];                 /*              */
             union {                                    /* PMR1         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char IRQ3:1;         /*    IRQ3      */
                          unsigned char IRQ2:1;         /*    IRQ2      */
                          unsigned char IRQ1:1;         /*    IRQ1      */
                          unsigned char IRQ0:1;         /*    IRQ0      */
                          unsigned char     :2;         /*              */
                          unsigned char TXD :1;         /*    TXD       */
                          }      BIT;                   /*              */
                   }            PMR1;                   /*              */
             union {                                    /* PMR5         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char POF7:1;         /*    POF7      */
                          unsigned char POF6:1;         /*    POF6      */
                          unsigned char WKP5:1;         /*    WKP5      */
                          unsigned char WKP4:1;         /*    WKP4      */
                          unsigned char WKP3:1;         /*    WKP3      */
                          unsigned char WKP2:1;         /*    WKP2      */
                          unsigned char WKP1:1;         /*    WKP1      */
                          unsigned char WKP0:1;         /*    WKP0      */
                          }      BIT;                   /*              */
                   }            PMR5;                   /*              */
             union {                                    /* PMR3         */
                   unsigned char BYTE;                  /*  Byte Access */
                   struct {                             /*  Bit  Access */
                          unsigned char     :3;         /*              */
                          unsigned char POF4:1;         /*    POF7      */
                          unsigned char POF3:1;         /*    POF6      */
                          }      BIT;                   /*              */
                   }            PMR3;                   /*              */
             char               wk4;                    /*              */
             unsigned char      PCR1;                   /* PCR1         */
             unsigned char      PCR2;                   /* PCR2         */
             char               wk5[2];                 /*              */
             unsigned char      PCR5;                   /* PCR5         */
             unsigned char      PCR6;                   /* PCR6         */
             unsigned char      PCR7;                   /* PCR7         */
             unsigned char      PCR8;                   /* PCR8         */
             unsigned char      PCR9;                   /* PCR9         */
};                                                      /*              */
union un_syscr1 {                                       /* union SYSCR1 */
                unsigned char BYTE;                     /*  Byte Access */
                struct {                                /*  Bit  Access */
                       unsigned char SSBY :1;           /*    SSBY      */
                       unsigned char STS  :3;           /*    STS       */
                       }      BIT;                      /*              */
};                                                      /*              */
union un_syscr2 {                                       /* union SYSCR2 */
                unsigned char BYTE;                     /*  Byte Access */
                struct {                                /*  Bit  Access */
                       unsigned char SMSEL:1;           /*    SMSEL     */
                       unsigned char LSON :1;           /*    LSON      */
                       unsigned char DTON :1;           /*    DTON      */
                       unsigned char MA   :3;           /*    MA        */
                       unsigned char SA   :2;           /*    SA        */
                       }      BIT;                      /*              */
};                                                      /*              */
union un_iegr1 {                                        /* union IEGR1  */
               unsigned char BYTE;                      /*  Byte Access */
               struct {                                 /*  Bit  Access */
                      unsigned char NMIEG:1;            /*    NMIEG     */
                      unsigned char      :3;            /*              */
                      unsigned char IEG3 :1;            /*    IEG3      */
                      unsigned char IEG2 :1;            /*    IEG2      */
                      unsigned char IEG1 :1;            /*    IEG1      */
                      unsigned char IEG0 :1;            /*    IEG0      */
                      }      BIT;                       /*              */
};                                                      /*              */
union un_iegr2 {                                        /* union IEGR2  */
               unsigned char BYTE;                      /*  Byte Access */
               struct {                                 /*  Bit  Access */
                      unsigned char      :2;            /*              */
                      unsigned char WPEG5:1;            /*    WPEG5     */
                      unsigned char WPEG4:1;            /*    WPEG4     */
                      unsigned char WPEG3:1;            /*    WPEG3     */
                      unsigned char WPEG2:1;            /*    WPEG2     */
                      unsigned char WPEG1:1;            /*    WPEG1     */
                      unsigned char WPEG0:1;            /*    WPEG0     */
                      }      BIT;                       /*              */
};                                                      /*              */
union un_ienr1 {                                        /* union IENR1  */
               unsigned char BYTE;                      /*  Byte Access */
               struct {                                 /*  Bit  Access */
                      unsigned char IENDT:1;            /*    IENDT     */
                      unsigned char      :1;            /*              */
                      unsigned char IENWP:1;            /*    IENWP     */
                      unsigned char      :1;            /*              */
                      unsigned char IEN3 :1;            /*    IEN3      */
                      unsigned char IEN2 :1;            /*    IEN2      */
                      unsigned char IEN1 :1;            /*    IEN1      */
                      unsigned char IEN0 :1;            /*    IEN0      */
                      }      BIT;                       /*              */
};                                                      /*              */
union un_ienr2 {                                        /* union IENR2  */
               unsigned char BYTE;                      /*  Byte Access */
               struct {                                 /*  Bit  Access */
                      unsigned char       :2;           /*              */
                      unsigned char IENTB1:1;           /*    IENTB1    */
                      }      BIT;                       /*              */
};                                                      /*              */
union un_irr1 {                                         /* union IRR1   */
              unsigned char BYTE;                       /*  Byte Access */
              struct {                                  /*  Bit  Access */
                     unsigned char IRRDT:1;             /*    IRRDT     */
                     unsigned char      :3;             /*              */
                     unsigned char IRRI3:1;             /*    IRRI3     */
                     unsigned char IRRI2:1;             /*    IRRI2     */
                     unsigned char IRRI1:1;             /*    IRRI1     */
                     unsigned char IRRI0:1;             /*    IRRI0     */
                     }      BIT;                        /*              */
};                                                      /*              */
union un_irr2 {                                         /* union IRR2   */
              unsigned char BYTE;                       /*  Byte Access */
              struct {                                  /*  Bit  Access */
                     unsigned char       :2;            /*              */
                     unsigned char IRRTB1:1;            /*    IRRTB1    */
                     }      BIT;                        /*              */
};                                                      /*              */
union un_iwpr {                                         /* union IWPR   */
              unsigned char BYTE;                       /*  Byte Access */
              struct {                                  /*  Bit  Access */
                     unsigned char      :2;             /*              */
                     unsigned char IWPF5:1;             /*    IWPF5     */
                     unsigned char IWPF4:1;             /*    IWPF4     */
                     unsigned char IWPF3:1;             /*    IWPF3     */
                     unsigned char IWPF2:1;             /*    IWPF2     */
                     unsigned char IWPF1:1;             /*    IWPF1     */
                     unsigned char IWPF0:1;             /*    IWPF0     */
                     }      BIT;                        /*              */
};                                                      /*              */
union un_mstcr1 {                                       /* union MSTCR1 */
                unsigned char BYTE;                     /*  Byte Access */
                struct {                                /*  Bit  Access */
                       unsigned char      :2;           /*              */
                       unsigned char MSTS3:1;           /*    MSTS3     */
                       unsigned char MSTAD:1;           /*    MSTAD     */
                       unsigned char MSTWD:1;           /*    MSTWD     */
                       unsigned char      :1;           /*              */
                       unsigned char MSTTV:1;           /*    MSTTV     */
                       }      BIT;                      /*              */
};                                                      /*              */
union un_mstcr2 {                                       /* union MSTCR2 */
                unsigned char BYTE;                     /*  Byte Access */
                struct {                                /*  Bit  Access */
                       unsigned char        :3;         /*              */
                       unsigned char MSTTB1 :1;         /*    MSTTB1    */
                       unsigned char        :2;         /*              */
                       unsigned char MSTTZ  :1;         /*    MSTTZ     */
                       }      BIT;                      /*              */
};                                                      /*              */
#define TinyCAN (*(volatile struct st_tinycan *)0xF600) /* TinyCAN      */
#define SSU     (*(volatile struct st_ssu     *)0xF6A0) /* SSU   Address*/
#define SBT     (*(volatile struct st_sbt     *)0xF6B0) /* SBT   Address*/
#define TZ0     (*(volatile struct st_tz0     *)0xF700) /* TZ0   Address*/
#define TZ1     (*(volatile struct st_tz1     *)0xF710) /* TZ1   Address*/
#define TZ      (*(volatile struct st_tz      *)0xF720) /* TZ    Address*/
#define LVD     (*(volatile struct st_lvd     *)0xF730) /* LVD   Address*/
#define TB1     (*(volatile struct st_tb1     *)0xF760) /* TB1   Address*/
#define FLASH   (*(volatile struct st_flash   *)0xFF90) /* FLASH Address*/
#define TV      (*(volatile struct st_tv      *)0xFFA0) /* TV    Address*/
#define SCI3    (*(volatile struct st_sci3    *)0xFFA8) /* SCI3  Address*/
#define AD      (*(volatile struct st_ad      *)0xFFB0) /* A/D   Address*/
#define WDT     (*(volatile struct st_wdt     *)0xFFC0) /* WDT   Address*/
#define ABRK    (*(volatile struct st_abrk    *)0xFFC8) /* ABRK  Address*/
#define IO      (*(volatile struct st_io      *)0xFFD0) /* IO    Address*/
#define SYSCR1  (*(volatile union  un_syscr1  *)0xFFF0) /* SYSCR1Address*/
#define SYSCR2  (*(volatile union  un_syscr2  *)0xFFF1) /* SYSCR2Address*/
#define IEGR1   (*(volatile union  un_iegr1   *)0xFFF2) /* IEGR1 Address*/
#define IEGR2   (*(volatile union  un_iegr2   *)0xFFF3) /* IEGR2 Address*/
#define IENR1   (*(volatile union  un_ienr1   *)0xFFF4) /* IENR1 Address*/
#define IENR2   (*(volatile union  un_ienr2   *)0xFFF5) /* IENR2 Address*/
#define IRR1    (*(volatile union  un_irr1    *)0xFFF6) /* IRR1  Address*/
#define IRR2    (*(volatile union  un_irr2    *)0xFFF7) /* IRR2  Address*/
#define IWPR    (*(volatile union  un_iwpr    *)0xFFF8) /* IWPR  Address*/
#define MSTCR1  (*(volatile union  un_mstcr1  *)0xFFF9) /* MSTCR1Address*/
#define MSTCR2  (*(volatile union  un_mstcr2  *)0xFFFA) /* MSTCR2Address*/
#define TLB1    TCB1                            /* Change TLB1 --> TCB1 */
