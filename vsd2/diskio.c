/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "msd.h"

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
/* Note that Tiny-FatFs supports only single drive and always            */
/* accesses drive number 0.                                              */

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
){
/*
	DSTATUS stat;
	int result;

	switch (drv) {
	case ATA :
		result = ATA_disk_initialize();
		// translate the reslut code here

		return stat;

	case MMC :
		result = MMC_disk_initialize();
		// translate the reslut code here

		return stat;

	case USB :
		result = USB_disk_initialize();
		// translate the reslut code here

		return stat;
	}
*/
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
){
/*
	DSTATUS stat;
	int result;
	
	switch (drv) {
	case ATA :
		result = ATA_disk_status();
		// translate the reslut code here

		return stat;

	case MMC :
		result = MMC_disk_status();
		// translate the reslut code here

		return stat;

	case USB :
		result = USB_disk_status();
		// translate the reslut code here

		return stat;
	}
*/
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
){
	MSD_ReadBlock( buff, sector, count * 512 );
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
	MSD_WriteBlock(( u8 *)buff, sector, count * 512 );
	return RES_OK;
}
#endif /* _READONLY */

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
){
	DRESULT res;
	sMSD_CSD MSD_csd;
	
	if (drv) return RES_PARERR;
	//if (Stat & STA_NOINIT) return RES_NOTRDY;
	
	res = RES_ERROR;
	switch (ctrl) {
		case CTRL_SYNC :		/* Make sure that no pending write process */
			//SELECT();
			//if (wait_ready() == 0xFF)
				res = RES_OK;
			break;
			
		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
			
			if( MSD_GetCSDRegister( &MSD_csd ) == MSD_RESPONSE_NO_ERROR ){
				*( DWORD* )buff =
					(( DWORD )MSD_csd.DeviceSize + 1 ) <<
					( MSD_csd.RdBlockLen + MSD_csd.DeviceSizeMul + 2 - 9 );
				res = RES_OK;
			}
			break;
			
		case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
			*( WORD *)buff = 512;
			res = RES_OK;
			break;
			
		default:
			res = RES_PARERR;
	}
	
	return res;
}

DWORD get_fattime( void ){
	return 0;
}
