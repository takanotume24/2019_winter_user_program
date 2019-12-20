
#include "header.h"

/*******************************************************************************
�@�@wii�����R������{�^��and �����x�����o�͂��郂�[�h��

********************************************************************************/
void wii_mode_set( void )
{
	unsigned char str[10] ;

	str[0] = 0x52 ;
	str[1] = 0x12 ;	/* �R�}���h */
	str[2] = 0x00 ;
	str[3] = 0x31 ;	/* a1 31 BB BB XX YY ZZ */

	write( ctl_sock, str, 4 ) ;

}


/*******************************************************************************
�@�@wii�����R����EEPROM���e��ǂݏo���֐�
�@�@�@�����ł͉����x�Z���T�́@0.0G and 1.0G �̎��̃f�[�^��������Ă���
�@�@�@�����̃f�[�^�́A0016����V�o�C�g
     
********************************************************************************/
void wii_EEPROM_read( void )
{
	unsigned char str[32] ;
	int n ;
//	int i ;

	str[0] = 0x52 ;
	str[1] = 0x17 ;	/* EEPROM read �R�}���h */
	str[2] = 0x00 ; /* �I�v�V���� */
	str[3] = 0x00 ;	/* �I�t�Z�b�g�l�P  */
	str[4] = 0x00 ;	/* �I�t�Z�b�g�l�Q  */
	str[5] = 0x16 ;	/* �I�t�Z�b�g�l�R  */
	str[6] = 0x00 ;	/* �ǂރo�C�g���P  */
	str[7] = 0x07 ;	/* �ǂރo�C�g���Q  */

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
�@�@wii�����R���ւS��LED�֕\������֐�

�@�@*--- 0x10  -*-- 0x20  --*- 0x40  ---* 0x80  
********************************************************************************/
void wii_LED_display( unsigned char data )
{
	unsigned char str[5] ;

	data &= 0xF0 ;

	wii_status &= 0x0F ;
	wii_status |= ( data & 0xF0 ) ;

	str[0] = 0x52 ;
	str[1] = 0x11 ;	/* LED�o�� �R�}���h */
	str[2] = data ; /* �\������f�[�^ */

	write( ctl_sock, str, 3 ) ;

}

/*******************************************************************************
�@�@wii�����R���̐U�����[�^�Ɋւ���֐��i�H�H�H�H�H�H�H�j

�@�@�I�I�I�I�I������g���ƃo�b�e������������̂Ŏg�p���Ȃ��I�I�I�I

�@ data : 1 --> ON   0 --> OFF
********************************************************************************/
void wii_Rumble_ON_OFF( unsigned char data )
{
	unsigned char str[5] ;

	if( data == ON ){
		wii_status |= 0x01 ;	/* ON */
	}else{
		wii_status &= ~0x01 ;	/* OFF */
	}

	/* �ۑ� wii�|�Q�@*/
	/* �e�L�X�g�����āAwii �ւ̃R�}���h�� str[]�֊i�[���� */



	/*  �����܂� */

	write( ctl_sock, str, 3 ) ;

	usleep( 5000 ) ;	/* ��������Ȃ��ƐU�����Ȃ� */

}

/*******************************************************************************
�@�@wii�����R���̃o�b�e����Ԃ𓾂�֐�

********************************************************************************/
unsigned char wii_Battery_check( void )
{
	unsigned char str[10] ;

	/* �ۑ� wii�|�R�@*/
	/* �e�L�X�g�����āAwii �ւ̃R�}���h�� str[]�֊i�[���� */

	/*  �����܂� */

	write( ctl_sock, str, 3 ) ;

	read( int_sock, str, sizeof(str) );

	return str[7] ;

}


/*******************************************************************************
�@�@wii�����R������{�^��and �����x���𓾂�֐�

�@�߂�l�F�e�l���ω������Ƃ��ɁA�e�r�b�g���P�ɂȂ�

********************************************************************************/
int wii_get_data( unsigned int *btn, struct wii_acc *acc )
{
	unsigned char str[10] ;
	static unsigned char str_buf2 = 0, str_buf3 = 0 ;
	static unsigned char x_buf = 0, y_buf = 0, z_buf = 0 ;
	int ret ;


	ret = 0 ;

	read( int_sock, str, sizeof(str) );

	/* �����x���ȑO�ƕω��������𔻒f */
	acc->x = str[4] ;  /* �����x�w  */
	acc->y = str[5] ;  /* �����x�x  */
	acc->z = str[6] ;  /* �����x�y  */

	/* �{�^����� */
	/* �֌W����r�b�g�̂ݎ��o�� */
	str[2] &= 0x1F ;
	str[3] &= 0x9F ;
	*btn = 0 ;
	*btn = str[2] ;
	*btn = *btn << 8 ;
	*btn += str[3] ;

	return ret ;
}


