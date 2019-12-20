
/*******************************************************************************
     wiiリモコンで制御する

                                                                 By Kanno
********************************************************************************/
#ifndef	GLOBAL_DEFINE				/* 変数やプロトタイプなどの宣言と実体化 */
#define	EXTERN	extern				/*		他のファイルには宣言するだけ	*/
#else									/*		メインファイルだけに実体化する	*/
#define	EXTERN
#endif

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h> 
#include <unistd.h> 
#include <termios.h>				/* シリアルポート */
#include <sys/ioctl.h>				/* ioctl */

#include <math.h>		    		/* 三角関数を利用するために必要                */

#include <sys/socket.h>				/* bluetooth用  */
#include <bluetooth/bluetooth.h>	/* bluetooth用  */
#include <bluetooth/hci.h>			/* bluetooth用  */
#include <bluetooth/hci_lib.h>		/* bluetooth用  */
#include <bluetooth/l2cap.h>		/* bluetooth用  */



/*******************************************************************************
       構造体の定義
********************************************************************************/
struct wii_acc{				/* Wii の加速度を格納する構造体 */
	unsigned char x ;
	unsigned char y ;
	unsigned char z ;
};

/*******************************************************************************
       関数のプロットタイプ
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
       定数の定義
********************************************************************************/
/* キーに関する定義 */
#define	ESC_KEY		0x1b	/* ESC キーのアスキーコード */

/* メインルーチン終了用定義 */
#define	DONE		1

/* 走行車の走行方向に関する定義 */
#define DIR_STOP    0	/* 停止 */
#define DIR_FORWARD	1	/* 前進 */
#define DIR_BACK	2	/* 後退 */
#define DIR_RIGHT	3	/* 右方向 */
#define DIR_LEFT	4	/* 左方向 */
#define DIR_BRAKE	5	/* ブレーキ */
#define DIR_RIGHT_TURN	6	/* 右ターン */
#define DIR_LEFT_TURN	7	/* 左ターン */


/* 走行車の走行するパルス数に関する定義 */
#define CONTINUE	0	/* 連続 */
#define NON_PARA	0  	/* パラメータなし */

/* 走行車の走行速度に関する定義 */
#define MIN_SPEED	40	/* 動く最低速度  */


/* 左右モータの回転に関する定義 */
#define MB_OFF		0	/* 回転を合わせない  */
#define MB_ON		1	/* 回転を合わせる  */


/* A/D変換器に関する定義 */
#define ADC_CH_VCC   	0	/* 電源電圧監視A/D変換器のチャンネル番号 */
#define ADC_CH_SENSOR   1	/* 距離センサのA/D変換器のチャンネル番号 */
#define ADC_CH_M_VDD   	3	/* モータ電圧監視A/D変換器のチャンネル番号 */
#define AVECNT			10	/* 平均する回数 */


/* wiiリモコンボタン情報 */
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


#define ACC_AVE_CNT 10			/* 加速度センサからのデータを平均する回数 */
#define PI 3.14159265358979323	/* 円周率 */

/* ロール・ピッチ角の変化量 */
#define DIFF_ROLL	0.1
#define DIFF_PITCH	0.1

/* wiiリモコン */
#define WR_NAME 		"Nintendo RVL-CNT-01"
#define WR_NAME_LEN 	sizeof(WR_NAME)

#define ON		1
#define OFF		0

/*******************************************************************************
       グローバル変数
********************************************************************************/

EXTERN int	ctl_sock ;				/* wii リモコン制御用ソケット番号 */
EXTERN int	int_sock ;				/* wii リモコンデータい入力用ソケット番号 */
EXTERN int	adp_sock ;				/* ARMに接続したBluetoothアダプタソケット番号 */

EXTERN struct wii_acc acc_zero ;	/* 加速度センサ 0Ｇ の値 */
EXTERN struct wii_acc acc_one ;	/* 加速度センサ 1Ｇ の値 */

EXTERN unsigned char wii_status ;

//#endif
