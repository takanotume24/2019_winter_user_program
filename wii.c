
#include "header.h"

/*******************************************************************************
　　wiiリモコンからボタンand 加速度情報を出力するモードへ

********************************************************************************/
void wii_mode_set( void )
{
	unsigned char str[10] ;

	str[0] = 0x52 ;
	str[1] = 0x12 ;	/* コマンド */
	str[2] = 0x00 ;
	str[3] = 0x31 ;	/* a1 31 BB BB XX YY ZZ */

	write( ctl_sock, str, 4 ) ;

}


/*******************************************************************************
　　wiiリモコンのEEPROM内容を読み出す関数
　　　ここでは加速度センサの　0.0G and 1.0G の時のデータが書かれている
　　　これらのデータは、0016から７バイト
     
********************************************************************************/
void wii_EEPROM_read( void )
{
	unsigned char str[32] ;
	int n ;
//	int i ;

	str[0] = 0x52 ;
	str[1] = 0x17 ;	/* EEPROM read コマンド */
	str[2] = 0x00 ; /* オプション */
	str[3] = 0x00 ;	/* オフセット値１  */
	str[4] = 0x00 ;	/* オフセット値２  */
	str[5] = 0x16 ;	/* オフセット値３  */
	str[6] = 0x00 ;	/* 読むバイト数１  */
	str[7] = 0x07 ;	/* 読むバイト数２  */

	write( ctl_sock, str, 8 ) ;

	n = read( int_sock, str, sizeof(str) );

	acc_zero.x = str[7] ;	/* zero point for X axis */
	acc_zero.y = str[8] ;	/* zero point for Y axis */
	acc_zero.z = str[9] ;	/* zero point for Z axis */
	acc_one.x  = str[11] ;	/* +1G point for X axis */
	acc_one.y  = str[12] ;	/* +1G point for Y axis */
	acc_one.z  = str[13] ;	/* +1G point for Z axis */

/*		
	for( i = 7; i < 14; i++ ){
		printf("%d ", str[i] ) ;
	}
	printf("\n" ) ;
*/

}

/*******************************************************************************
　　wiiリモコンへ４つのLEDへ表示する関数

　　*--- 0x10  -*-- 0x20  --*- 0x40  ---* 0x80  
********************************************************************************/
void wii_LED_display( unsigned char data )
{
	unsigned char str[5] ;

	data &= 0xF0 ;

	wii_status &= 0x0F ;
	wii_status |= ( data & 0xF0 ) ;

	str[0] = 0x52 ;
	str[1] = 0x11 ;	/* LED出力 コマンド */
	str[2] = data ; /* 表示するデータ */

	write( ctl_sock, str, 3 ) ;

}

/*******************************************************************************
　　wiiリモコンの振動モータに関する関数（？？？？？？？）

　　！！！！！これを使うとバッテリが激減するので使用しない！！！！

　 data : 1 --> ON   0 --> OFF
********************************************************************************/
void wii_Rumble_ON_OFF( unsigned char data )
{
	unsigned char str[5] ;

	if( data == ON ){
		wii_status |= 0x01 ;	/* ON */
	}else{
		wii_status &= ~0x01 ;	/* OFF */
	}

	/* 課題 wii－２　*/
	/* テキストを見て、wii へのコマンドを str[]へ格納する */



	/*  ここまで */

	write( ctl_sock, str, 3 ) ;

	usleep( 5000 ) ;	/* これを入れないと振動しない */

}

/*******************************************************************************
　　wiiリモコンのバッテリ状態を得る関数

********************************************************************************/
unsigned char wii_Battery_check( void )
{
	unsigned char str[10] ;

	/* 課題 wii－３　*/
	/* テキストを見て、wii へのコマンドを str[]へ格納する */

	/*  ここまで */

	write( ctl_sock, str, 3 ) ;

	read( int_sock, str, sizeof(str) );

	return str[7] ;

}


/*******************************************************************************
　　wiiリモコンからボタンand 加速度情報を得る関数

　戻り値：各値が変化したときに、各ビットが１になる

********************************************************************************/
int wii_get_data( unsigned int *btn, struct wii_acc *acc )
{
	unsigned char str[10] ;
	static unsigned char str_buf2 = 0, str_buf3 = 0 ;
	static unsigned char x_buf = 0, y_buf = 0, z_buf = 0 ;
	int ret ;


	ret = 0 ;

	read( int_sock, str, sizeof(str) );

	/* 加速度が以前と変化したかを判断 */
	acc->x = str[4] ;  /* 加速度Ｘ  */
	acc->y = str[5] ;  /* 加速度Ｙ  */
	acc->z = str[6] ;  /* 加速度Ｚ  */

	/* ボタン情報 */
	/* 関係するビットのみ取り出す */
	str[2] &= 0x1F ;
	str[3] &= 0x9F ;
	*btn = 0 ;
	*btn = str[2] ;
	*btn = *btn << 8 ;
	*btn += str[3] ;

	return ret ;
}


