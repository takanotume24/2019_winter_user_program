
#define GLOBAL_DEFINE
#include "header.h"

/*******************************************************************************
　　メイン関数
********************************************************************************/
int main(int argc, char **argv) {
  /* プログラムの終了を判定する変数  */
  double dif_vcc, vdd_m; /* ARMの電源電圧　モータの電源電圧  */

  int wii_data_flg; /* リモコンのボタン、加速度値の変化情報 */
  unsigned int btn_data;      /* Wii のボタン情報 */
  struct wii_acc acc_now;     /* 計測した加速度センサの値 */
  double wii_roll, wii_pitch; /* Wii のロール、ピッチ */

  int balance_flg = 0; /* 左右のモータの回転数のバランスの設定 */
  int SW_status = 0; /* リミットＳＷの状態  */

  wii_status = 0;

  printf("ラズパイ３の場合はオンボードのbluetoothとUSBカメラが干渉する。\n");
  printf("ラズパイ３の場合はUSBのbluetoothあるか確認！\n");
  printf("ラズパイ３でbluetooth接続がうまくいかない場合は、、、\n");
  printf("USBカメラを抜いてreboot(すなわちカメラをあきらめる)か、\n");
  printf("hciconfig でUARTの方のhciデバイスをダウンさせる、、\n");

  printf("\n************************\n");
  printf("  Wii Remot  push 1 & 2 \n");
  printf("************************\n");
  printf("WiiリモコンのHOMEボタンで終了します\n");

  sleep(2); /* この間に１＆２ボタンを押す */

  /* Bluetoothの初期化 */
  Bluetooth_init();
  wii_EEPROM_read();     /* Wii のEEPROM情報を読む */
  wii_LED_display(0x00); /* Wii のLEDを消す */

  wii_Rumble_ON_OFF(ON);
  sleep(1); /* １秒間振動させて、接続が完了したことを知らせる */
  wii_Rumble_ON_OFF(OFF);

  wii_mode_set(); /* ボタン＆加速度情報を出力モードへ */

  while (1) {
    /* リモコンのボタン、加速度値を読む */
    wii_data_flg = wii_get_data(&btn_data, &acc_now);
    printf("BTN=%04x, ACC: x=%d, y=%d, z=%d\n", btn_data, acc_now.x, acc_now.y,
           acc_now.z);

    if (btn_data & WII_BTN_HOME) { /* Home */
      break;
    }
  }

  wii_LED_display(0x00);

  /* プログラム終了時には必ず close する */
  close(ctl_sock);
  close(int_sock);
  close(adp_sock);

  return 0;
}
