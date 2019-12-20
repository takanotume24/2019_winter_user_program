
/*******************************************************************************
     wii�����R���Ő��䂷��

                                                                 By Kanno
********************************************************************************/
#ifndef	GLOBAL_DEFINE				/* �ϐ���v���g�^�C�v�Ȃǂ̐錾�Ǝ��̉� */
#define	EXTERN	extern				/*		���̃t�@�C���ɂ͐錾���邾��	*/
#else									/*		���C���t�@�C�������Ɏ��̉�����	*/
#define	EXTERN
#endif

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h> 
#include <unistd.h> 
#include <termios.h>				/* �V���A���|�[�g */
#include <sys/ioctl.h>				/* ioctl */

#include <math.h>		    		/* �O�p�֐��𗘗p���邽�߂ɕK�v                */

#include <sys/socket.h>				/* bluetooth�p  */
#include <bluetooth/bluetooth.h>	/* bluetooth�p  */
#include <bluetooth/hci.h>			/* bluetooth�p  */
#include <bluetooth/hci_lib.h>		/* bluetooth�p  */
#include <bluetooth/l2cap.h>		/* bluetooth�p  */



/*******************************************************************************
       �\���̂̒�`
********************************************************************************/
struct wii_acc{				/* Wii �̉����x���i�[����\���� */
	unsigned char x ;
	unsigned char y ;
	unsigned char z ;
};

/*******************************************************************************
       �֐��̃v���b�g�^�C�v
********************************************************************************/
EXTERN int getch( void ) ;
EXTERN int drive_finish( void ) ;
EXTERN int drive( int, int, int ) ;
EXTERN void display_menu( void ) ;

EXTERN void Bluetooth_init( void ) ;
EXTERN void wii_mode_set( void ) ;
EXTERN void wii_EEPROM_read( void ) ;
EXTERN void wii_LED_display( unsigned char ) ;
EXTERN void wii_Rumble_ON_OFF( unsigned char ) ;
EXTERN unsigned char wii_Battery_check( void ) ;

EXTERN int wii_get_data( unsigned int *, struct wii_acc * ) ;
EXTERN int wiimote_acc( struct wii_acc *, double *, double * ) ;

/*******************************************************************************
       �萔�̒�`
********************************************************************************/
/* �L�[�Ɋւ����` */
#define	ESC_KEY		0x1b	/* ESC �L�[�̃A�X�L�[�R�[�h */

/* ���C�����[�`���I���p��` */
#define	DONE		1

/* ���s�Ԃ̑��s�����Ɋւ����` */
#define DIR_STOP    0	/* ��~ */
#define DIR_FORWARD	1	/* �O�i */
#define DIR_BACK	2	/* ��� */
#define DIR_RIGHT	3	/* �E���� */
#define DIR_LEFT	4	/* ������ */
#define DIR_BRAKE	5	/* �u���[�L */
#define DIR_RIGHT_TURN	6	/* �E�^�[�� */
#define DIR_LEFT_TURN	7	/* ���^�[�� */


/* ���s�Ԃ̑��s����p���X���Ɋւ����` */
#define CONTINUE	0	/* �A�� */
#define NON_PARA	0  	/* �p�����[�^�Ȃ� */

/* ���s�Ԃ̑��s���x�Ɋւ����` */
#define MIN_SPEED	40	/* �����Œᑬ�x  */


/* ���E���[�^�̉�]�Ɋւ����` */
#define MB_OFF		0	/* ��]�����킹�Ȃ�  */
#define MB_ON		1	/* ��]�����킹��  */


/* A/D�ϊ���Ɋւ����` */
#define ADC_CH_VCC   	0	/* �d���d���Ď�A/D�ϊ���̃`�����l���ԍ� */
#define ADC_CH_SENSOR   1	/* �����Z���T��A/D�ϊ���̃`�����l���ԍ� */
#define ADC_CH_M_VDD   	3	/* ���[�^�d���Ď�A/D�ϊ���̃`�����l���ԍ� */
#define AVECNT			10	/* ���ς���� */


/* wii�����R���{�^����� */
#define WII_BTN_LEFT	0x0100
#define WII_BTN_RIGHT	0x0200
#define WII_BTN_DOWN	0x0400
#define WII_BTN_UP		0x0800
#define WII_BTN_PLUS	0x1000

#define WII_BTN_2		0x0001
#define WII_BTN_1		0x0002
#define WII_BTN_B		0x0004
#define WII_BTN_A		0x0008
#define WII_BTN_MINUS	0x0010
#define WII_BTN_HOME	0x0080


#define ACC_AVE_CNT 10			/* �����x�Z���T����̃f�[�^�𕽋ς���� */
#define PI 3.14159265358979323	/* �~���� */

/* ���[���E�s�b�`�p�̕ω��� */
#define DIFF_ROLL	0.1
#define DIFF_PITCH	0.1

/* wii�����R�� */
#define WR_NAME 		"Nintendo RVL-CNT-01"
#define WR_NAME_LEN 	sizeof(WR_NAME)

#define ON		1
#define OFF		0

/*******************************************************************************
       �O���[�o���ϐ�
********************************************************************************/

EXTERN int	ctl_sock ;				/* wii �����R������p�\�P�b�g�ԍ� */
EXTERN int	int_sock ;				/* wii �����R���f�[�^�����͗p�\�P�b�g�ԍ� */
EXTERN int	adp_sock ;				/* ARM�ɐڑ�����Bluetooth�A�_�v�^�\�P�b�g�ԍ� */

EXTERN struct wii_acc acc_zero ;	/* �����x�Z���T 0�f �̒l */
EXTERN struct wii_acc acc_one ;	/* �����x�Z���T 1�f �̒l */

EXTERN unsigned char wii_status ;

//#endif
