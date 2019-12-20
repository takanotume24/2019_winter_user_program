
#include "header.h"

/*******************************************************************************
�@�@�V���A���|�[�g[ STDIN_FILENO:�W������ ]����u���b�N�����ɁA�P������ǂݍ���
      �P�D��J�m�j�J�� �G�R�[ON
	�@�Q�DVTIME=0 VMIN=0 
********************************************************************************/
int getch(void)
{
	struct termios oldt, newt ;
	int ch ;
	char buf[10] ;

	if (!isatty( STDIN_FILENO ) ) { /* �W������ */
		perror("this problem should be run at a terminal\n") ;
		exit(1);
	}
	/* ���݂̃V���A���|�[�g�̐ݒ��Ҕ� */
	if(tcgetattr( STDIN_FILENO, &oldt ) < 0) {
		perror("save the terminal setting");
		exit(1);
	}

	/* �V���A���|�[�g�̐ݒ� */
	/* ��J�m�j�J��  VTIME=0 VMIN=0 �ŁA���L�� read() �œ��͑҂��ɂȂ�Ȃ�*/
	newt = oldt ;
	newt.c_lflag &= ~( ICANON ) ;	/* ��J�m�j�J�� ���]���Ă��邱�Ƃɒ��� */
	newt.c_cc[VTIME] = 0 ;	/* �L�����N�^�ԃ^�C�}�͖��g�p */
	newt.c_cc[VMIN] = 0 ;	/* �ǂݍ��݂͑����Ɏ��s */

	/* �V�����V���A���|�[�g�̐ݒ� */
	if(tcsetattr( STDIN_FILENO, TCSANOW, &newt ) < 0) {
		perror("set terminal");
		exit(1);
	}

	/* ���ݓǂݍ��݉\�ȕ��������C�v������������[1]���߂���� */
	/* ����read�̓u���b�N����Ȃ� */
	ch = read( STDIN_FILENO, buf, 1 ) ;
	if( ch != 0 )
		ch = (int)buf[0] ;

	/* �|�[�g�̐ݒ���v���O�����J�n���̂��̂ɖ߂� */
	if(tcsetattr(STDIN_FILENO,TCSANOW,&oldt) < 0) {
		perror("restore the termial setting");
		exit(1);
	}

	return ch;
}
