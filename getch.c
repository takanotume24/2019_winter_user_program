
#include "header.h"

/*******************************************************************************
　　シリアルポート[ STDIN_FILENO:標準入力 ]からブロックせずに、１文字を読み込む
      １．非カノニカル エコーON
	　２．VTIME=0 VMIN=0 
********************************************************************************/
int getch(void)
{
	struct termios oldt, newt ;
	int ch ;
	char buf[10] ;

	if (!isatty( STDIN_FILENO ) ) { /* 標準入力 */
		perror("this problem should be run at a terminal\n") ;
		exit(1);
	}
	/* 現在のシリアルポートの設定を待避 */
	if(tcgetattr( STDIN_FILENO, &oldt ) < 0) {
		perror("save the terminal setting");
		exit(1);
	}

	/* シリアルポートの設定 */
	/* 非カノニカル  VTIME=0 VMIN=0 で、下記の read() で入力待ちにならない*/
	newt = oldt ;
	newt.c_lflag &= ~( ICANON ) ;	/* 非カノニカル 反転していることに注意 */
	newt.c_cc[VTIME] = 0 ;	/* キャラクタ間タイマは未使用 */
	newt.c_cc[VMIN] = 0 ;	/* 読み込みは即座に実行 */

	/* 新しいシリアルポートの設定 */
	if(tcsetattr( STDIN_FILENO, TCSANOW, &newt ) < 0) {
		perror("set terminal");
		exit(1);
	}

	/* 現在読み込み可能な文字数か，要求した文字数[1]が戻される */
	/* このreadはブロックされない */
	ch = read( STDIN_FILENO, buf, 1 ) ;
	if( ch != 0 )
		ch = (int)buf[0] ;

	/* ポートの設定をプログラム開始時のものに戻す */
	if(tcsetattr(STDIN_FILENO,TCSANOW,&oldt) < 0) {
		perror("restore the termial setting");
		exit(1);
	}

	return ch;
}
