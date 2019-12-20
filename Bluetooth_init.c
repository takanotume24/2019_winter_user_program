
#include "header.h"

/*******************************************************************************
　　Bluetoothディバイスの初期化
　　　Bluezライブラリを使用
********************************************************************************/
void Bluetooth_init(void) {
  int adp_id;         /* ARMのBluetoothアダプタＩＤ */
  int dev_num;        /* 検出したBluetoothの数 */
  int max_rsp = 10;   /* 探索するBluetoothの最大数 */
  int len = 8;        /* 探索する時間 */
  int timeout = 5000; /* 名前を探索する時間 */

  //	char btaddr[20]="E8:4E:CE:97:49:D5" ;		/* 04 */
  char btaddr[20] = "00:27:09:3E:6B:DF"; /* 18 */
  char btname[64];                       /* 検出したBluetoothの名前 */
  inquiry_info *dev_list = NULL;
  struct sockaddr_l2 ctl_wraddr, int_wraddr;
  int i;

  /* ARM Bluetooth アダプタの探索 */
  if ((adp_id = hci_get_route(NULL)) == -1) {
    printf("Not found adapter \n");
    exit(1);
  } else
    printf("Use available bluetooth adapter id %d \n", adp_id);

  /* ARMソケットのオープン */
  if ((adp_sock = hci_open_dev(adp_id)) == -1) {
    printf("Adapter open error \n");
    exit(1);
  } else
    printf("Open adapter socket id %d\n", adp_sock);

  /* 周囲にあるBluetooth端末の物理アドレスを取得する
  printf("Searching device...\n") ;
  dev_list = (inquiry_info*)malloc( max_rsp * sizeof(inquiry_info) ) ;
  if( ( dev_num = hci_inquiry( adp_id, len, max_rsp, NULL, &dev_list,
  IREQ_CACHE_FLUSH ) ) < 0 ){ printf("Error on device query \n") ; close(
  adp_sock ) ; exit(1) ;
  }
  printf("Find %d device\n\n", dev_num ) ;


  for( i = 0; i < dev_num; i++ ){
          if( hci_read_remote_name( adp_sock, &dev_list[i].bdaddr, WR_NAME_LEN,
  btname, timeout ) ){ printf("Error read name device. \n" ) ; }else{ if(
  strncmp( btname, WR_NAME, WR_NAME_LEN ) == 0 ){ ba2str( &dev_list[i].bdaddr,
  btaddr ) ; printf(" Set wii remote controler addres %s.\n", btaddr ) ; break;
                  }
          }
  }

  free( dev_list ) ;
*/

  /* Wiiリモコンとの接続 */
  /* ctl_sock : コントローラの制御に使用 */
  memset(&ctl_wraddr, 0, sizeof(struct sockaddr_l2));
  ctl_wraddr.l2_family = AF_BLUETOOTH;
  str2ba(btaddr, &ctl_wraddr.l2_bdaddr);
  ctl_wraddr.l2_psm = htobs(17);

  /* int_sock : コントローラからボタン・加速度情報の取得に使用 */
  memset(&int_wraddr, 0, sizeof(struct sockaddr_l2));
  int_wraddr.l2_family = AF_BLUETOOTH;
  str2ba(btaddr, &int_wraddr.l2_bdaddr);
  int_wraddr.l2_psm = htobs(19);

  ctl_sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
  int_sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

  if (connect(ctl_sock, (struct sockaddr *)&ctl_wraddr,
              sizeof(struct sockaddr_l2))) {
    printf("connect control port 17 failed \n");
    exit(1);
  }
  if (connect(int_sock, (struct sockaddr *)&int_wraddr,
              sizeof(struct sockaddr_l2))) {
    printf("connect interupt port 19 failed \n");
    exit(1);
  }

  /* 何も押されていない状態をいちど読み込む必要がある */
  read(int_sock, btaddr, sizeof(btaddr));
}
