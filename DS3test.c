/*
 * jstest.c  Version 1.2
 *
 * Copyright (c) 1996-1999 Vojtech Pavlik
 *
 * Sponsored by SuSE
 */

/*
 * This program can be used to test all the features of the Linux
 * joystick API, including non-blocking and select() access, as
 * well as version 0.x compatibility mode. It is also intended to
 * serve as an example implementation for those who wish to learn
 * how to write their own joystick using applications.
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#define GLOBAL_DEFINE
#include "./header.h"

#include <linux/input.h>
#include <linux/joystick.h>

#define SCL 50
#define NAME_LENGTH 128
#define MAX_VALUE_STICK 32767.0
#define MIM_VALUE_STICK -32767.0
#define MAX_VALUE_ARG 100
#define WII_VALUE_HEIKOU 128
#define WII_VALUE_LEFT 156
#define WII_VALUE_RIGHT 100

#define MAX_VALUE_ARG 100
#define FACE_VALUE_HEIKOU 50
#define FACE_VALUE_LEFT 100
#define FACE_VALUE_RIGHT 0

#define CMD_GPIO02_VALUE 1

#define CMD_MOTOR_LEFT_FOWARD 32
#define CMD_MOTOR_LEFT_BACK 33
#define CMD_MOTOR_LEFT_STOP 34
#define CMD_MOTOR_RIGHT_FOWARD 35
#define CMD_MOTOR_RIGHT_BACK 36
#define CMD_MOTOR_RIGHT_STOP 37
#define CMD_HOUDAI_RIGHT 38
#define CMD_HOUDAI_LEFT 39
#define CMD_HOUDAI_STOP 40
#define CMD_GUN_FIRE 41
#define CMD_GUN_STOP 42
#define CMD_MOTOR_LEFT_ENABLE 43
#define CMD_MOTOR_RIGHT_ENABLE 44

#define DS3_BUTTON_UP 4
#define DS3_BUTTON_LEFT 5
#define DS3_BUTTON_DOWN 6
#define DS3_BUTTON_RIGHT 7
#define DS3_BUTTON_SANKAKU 12
#define DS3_BUTTON_MARU 13
#define DS3_BUTTON_BATSU 14
#define DS3_BUTTON_SIKAKU 15
#define DS3_BUTTON_L1 10
#define DS3_BUTTON_L2 8
#define DS3_BUTTON_R1 11
#define DS3_BUTTON_R2 9
#define DS3_BUTTON_SELECT 0
#define DS3_BUTTON_START 3
#define DS3_BUTTON_PS 16
#define DS3_BUTTON_PRESS 1
#define DS3_BUTTON_RELEASE 0

#define DS3_TYPE_1 1
#define DS3_TYPE_2 2

#define DS3_STICK_LEFT_X 0
#define DS3_STICK_LEFT_Y 1
#define DS3_STICK_RIGHT_X 2
#define DS3_STICK_RIGHT_Y 3

#define MUSIC_ON 45
#define MUSIC_OFF 46

void *fun_thread_enable_a_pwm(void *ptr);
void *fun_thread_enable_b_pwm(void *ptr);
void *fun_thread_ds3(void *ptr);
void *fun_thread_wii(void *ptr);
void *fun_thread_wii_sensor(void *ptr);
void *fun_thread_socket(void *ptr);

uint16_t enable_a = 0;
uint16_t enable_b = 0;
useconds_t tick_a = 0;
useconds_t tick_b = 0;
int fd_ds3;
int fd_motor;

pthread_t thread_enable_a;
pthread_t thread_enable_b;
pthread_t thread_ds3;
pthread_t thread_wii;
pthread_t thread_get_sensor;
pthread_t thread_socket;

static volatile int keep_running = 1;
struct js_event js;
unsigned int btn_data;  /* Wii のボタン情報 */
struct wii_acc acc_now; /* 計測した加速度センサの値 */

int sockfd;
int client_sockfd;

int music = 0;
int music_now = MUSIC_OFF;

void tajima_2(struct js_event js, int fd);
void tajima_3(struct js_event js, int fd);
void tajima_4(struct js_event js, int fd);
void tajima_5(struct js_event js, int fd);
void tajima_6(struct js_event js, int fd);

void intHandler(int dummy) { keep_running = 0; }

int main(int argc, char **argv) {
  signal(SIGINT, intHandler);

  char name[NAME_LENGTH] = "Unknown";
  uint16_t arg = 0;

  int result_pthread_create_pin_enable_a = 0;
  int result_pthread_create_pin_enable_b = 0;
  int result_pthread_ds3 = 0;
  int result_pthread_wii = 0;
  int result_pthread_socket = 0;

  fd_motor = open("/dev/raspiGpio0", O_RDWR);
  if (fd_motor < 0) {
    printf("Error opening GPIO\n");
    exit(EXIT_FAILURE);
  }

  if ((fd_ds3 = open("/dev/input/js0", O_RDONLY)) < 0) {
    printf("Can't open /dev/input/js0\n");
    exit(EXIT_FAILURE);
  }

  ioctl(fd_ds3, JSIOCGNAME(NAME_LENGTH), name);
  printf("Joystick: %s\n", name);

  fcntl(fd_ds3, F_SETFL, O_NONBLOCK);  // Choose yourself

  result_pthread_create_pin_enable_a = pthread_create(
      &thread_enable_a, NULL, fun_thread_enable_a_pwm, (void *)&fd_motor);
  result_pthread_create_pin_enable_b = pthread_create(
      &thread_enable_b, NULL, fun_thread_enable_b_pwm, (void *)&fd_motor);
  result_pthread_ds3 =
      pthread_create(&thread_ds3, NULL, fun_thread_ds3, (void *)&fd_motor);
  result_pthread_socket = pthread_create(&thread_socket, NULL,
                                         fun_thread_socket, (void *)&fd_motor);

  if (result_pthread_create_pin_enable_a != 0) {
    printf("faild create pthread pin 05\n");
  }

  if (result_pthread_create_pin_enable_b != 0) {
    printf("faild create pthread pin 06\n");
  }
  if (result_pthread_ds3 != 0) {
    printf("faild create pthread ds3\n");
  }

  while (keep_running) {
    read(fd_ds3, &js, sizeof(struct js_event));
    usleep(10000);  // Choose yourself
  }

  close(fd_motor);
  close(fd_ds3);
  wii_LED_display(0x00);

  /* プログラム終了時には必ず close する */
  close(ctl_sock);
  close(int_sock);
  close(adp_sock);
  // ソケットクローズ
  close(client_sockfd);
  close(sockfd);
  printf("\nexit.\n");
  exit(EXIT_SUCCESS);
}

void *fun_thread_enable_a_pwm(void *ptr) {
  int fd = *(int *)ptr;
  printf("CMD_MOTOR_LEFT_ENABLE duty: %d\n", *(int *)(ptr));
  while (1) {
    if (tick_a < 0) {
      tick_a = 0;
    }
    if (tick_a > 100) {
      tick_a = 100;
    }
    ioctl(fd, CMD_MOTOR_LEFT_ENABLE, 1);
    usleep(tick_a * SCL);
    ioctl(fd, CMD_MOTOR_LEFT_ENABLE, 0);
    usleep(((useconds_t)100 - tick_a) * SCL);
  }
}

void *fun_thread_enable_b_pwm(void *ptr) {
  int fd = *(int *)ptr;
  printf("CMD_MOTOR_RIGHT_ENABLE duty: %d\n", *(int *)(ptr));
  while (1) {
    if (tick_b < 0) {
      tick_b = 0;
    }
    if (tick_b > 100) {
      tick_b = 100;
    }
    ioctl(fd, CMD_MOTOR_RIGHT_ENABLE, 1);
    usleep(tick_b * SCL);
    ioctl(fd, CMD_MOTOR_RIGHT_ENABLE, 0);
    usleep(((useconds_t)100 - tick_b) * SCL);
  }
}

int check_tajima_button(struct js_event js) {
  switch (js.type) {
    case DS3_TYPE_1:
      switch (js.number) {
        case DS3_BUTTON_DOWN:
        case DS3_BUTTON_UP:
        case DS3_BUTTON_LEFT:
        case DS3_BUTTON_RIGHT:
          printf("tajima mode\n");
          return 1;
          break;
      }
      break;
  }
  return 0;
}

void *fun_thread_ds3(void *ptr) {
  int fd_motor = *(int *)ptr;
  uint16_t js_time_old = 0;

  while (keep_running) {
    tajima_2(js, fd_motor);
    tajima_3(js, fd_motor);
    tajima_4(js, fd_motor);
    tajima_5(js, fd_motor);
    tajima_6(js, fd_motor);

    if (check_tajima_button(js)) {
      continue;
    }

    if (js_time_old == js.time) {
      ioctl(fd_motor, CMD_HOUDAI_STOP, 0);
      continue;
    }

    switch (js.type) {
      case DS3_TYPE_1:
        switch (js.number) {
          case DS3_BUTTON_BATSU:
            ioctl(fd_motor, CMD_MOTOR_LEFT_STOP, 0);
            ioctl(fd_motor, CMD_MOTOR_RIGHT_STOP, 0);
            break;

          case DS3_BUTTON_R1:
            printf("R1 %d\n", js.value);
            switch (js.value) {
              case DS3_BUTTON_PRESS:
                ioctl(fd_motor, CMD_HOUDAI_RIGHT, 0);
                break;

              case DS3_BUTTON_RELEASE:
                ioctl(fd_motor, CMD_HOUDAI_STOP, 0);
                break;
            }
            break;

          case DS3_BUTTON_L1:
            printf("R2 %d\n", js.value);
            switch (js.value) {
              case DS3_BUTTON_PRESS:
                ioctl(fd_motor, CMD_HOUDAI_LEFT, 0);
                break;

              case DS3_BUTTON_RELEASE:
                ioctl(fd_motor, CMD_HOUDAI_STOP, 0);
                break;
            }
            break;

          case DS3_BUTTON_R2:
            switch (js.value) {
              case DS3_BUTTON_PRESS:
                ioctl(fd_motor, CMD_GUN_FIRE, 0);
                break;

              case DS3_BUTTON_RELEASE:
                ioctl(fd_motor, CMD_GUN_STOP, 0);
                break;
            }
            break;
        }
        break;

      case DS3_TYPE_2:
        switch (js.number) {
          case DS3_STICK_LEFT_Y:
            if (js.value < 0) {
              ioctl(fd_motor, CMD_MOTOR_LEFT_FOWARD, 0);
              tick_a = (abs(js.value) / MAX_VALUE_STICK) * MAX_VALUE_ARG;
            } else {
              ioctl(fd_motor, CMD_MOTOR_LEFT_BACK, 0);
              tick_a = (abs(js.value) / MAX_VALUE_STICK) * MAX_VALUE_ARG;
            }
            break;

          case DS3_STICK_RIGHT_Y:
            if (js.value < 0) {
              ioctl(fd_motor, CMD_MOTOR_RIGHT_FOWARD, 0);
              tick_b = (abs(js.value) / MAX_VALUE_STICK) * MAX_VALUE_ARG;
            } else {
              ioctl(fd_motor, CMD_MOTOR_RIGHT_BACK, 0);
              tick_b = (abs(js.value) / MAX_VALUE_STICK) * MAX_VALUE_ARG;
            }
            printf("right %d\n", js.value);
            break;
        }
        break;
    }

    js_time_old = js.time;

    usleep(10000);  // Choose yourself
  }
}

void *fun_thread_wii(void *ptr) {
  int fd = *(int *)ptr;

  double dif_vcc, vdd_m; /* ARMの電源電圧　モータの電源電圧  */

  int wii_data_flg; /* リモコンのボタン、加速度値の変化情報 */

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
  unsigned int btn_data_old = 0;
  while (1) {
    wii_get_data(&btn_data, &acc_now);

    /* リモコンのボタン、加速度値を読む */
    if (btn_data_old == btn_data) continue;

    if (btn_data & WII_BTN_1) {
      ioctl(fd, CMD_MOTOR_LEFT_FOWARD, 0);
      ioctl(fd, CMD_MOTOR_RIGHT_FOWARD, 0);
    }
    if (btn_data & WII_BTN_2) {
      ioctl(fd, CMD_MOTOR_LEFT_BACK, 0);
      ioctl(fd, CMD_MOTOR_RIGHT_BACK, 0);
    }
    if (btn_data & WII_BTN_UP) {
      ioctl(fd, CMD_HOUDAI_LEFT, 0);
    }
    if (btn_data & WII_BTN_DOWN) {
      ioctl(fd, CMD_HOUDAI_RIGHT, 0);
    }
    if (!(btn_data & (WII_BTN_UP | WII_BTN_DOWN))) {
      ioctl(fd, CMD_HOUDAI_STOP, 0);
    }
    if (btn_data & (WII_BTN_B)) {
      ioctl(fd, CMD_GUN_FIRE, 0);
    } else {
      ioctl(fd, CMD_GUN_STOP, 0);
    }
    if (btn_data == 0) {
      ioctl(fd, CMD_MOTOR_LEFT_STOP, 0);
      ioctl(fd, CMD_MOTOR_RIGHT_STOP, 0);
      ioctl(fd, CMD_HOUDAI_STOP, 0);
    }
    if (btn_data & WII_BTN_HOME) { /* Home */
      break;
    }
    usleep(10000);
    btn_data_old = btn_data;
  }
}

void *fun_thread_wii_sensor(void *ptr) {
  while (1) {
    if (!(btn_data & (WII_BTN_1 | WII_BTN_2))) continue;
    if (acc_now.y > WII_VALUE_HEIKOU) {
      tick_a = (abs(WII_VALUE_LEFT - acc_now.y) /
                (float)(WII_VALUE_LEFT - WII_VALUE_HEIKOU)) *
               100;
      tick_b = 100;
    } else {
      tick_a = 100;
      tick_b = (abs(acc_now.y - WII_VALUE_RIGHT) /
                (float)(WII_VALUE_LEFT - WII_VALUE_HEIKOU)) *
               100;
    }
    usleep(10000);
  }
}

void *fun_thread_socket(void *ptr) {
  int fd = *(int *)ptr;

  int sockfd;
  struct sockaddr_in addr;

  // ソケット生成
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
  }

  // 送信先アドレス・ポート番号設定
  addr.sin_family = AF_INET;
  addr.sin_port = htons(1234);
  addr.sin_addr.s_addr = inet_addr("192.168.22.5");

  // サーバ接続
  connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

  // データ送信
  char send_str[10] = "ok";
  char receive_str[10] = {0};
  int i = 0;
  int old_motor = 0;

  while (1) {
    read(sockfd, receive_str, sizeof(char) * 10);
    write(sockfd, "ok", sizeof(char) * 10);
    int motor = atoi(receive_str);
    printf("receive:%d\n", motor);

    if (old_motor == motor) {
      continue;
    } else {
      if (motor == 0) {
        tick_a = 0;
        tick_b = 0;
      }
    }
    old_motor = motor;
    if (motor > 320 || motor <= 0) {
      continue;
    }
    if (motor > 160) {
      ioctl(fd, CMD_MOTOR_LEFT_BACK, 0);
      ioctl(fd, CMD_MOTOR_RIGHT_FOWARD, 0);
    } else {
      ioctl(fd, CMD_MOTOR_LEFT_FOWARD, 0);
      ioctl(fd, CMD_MOTOR_RIGHT_BACK, 0);
    }
    int tick = (abs(motor - 160) / 160.0) * 100 + 20;
    if (tick > 100) {
      tick = 100;
    }
    tick_a = tick;
    tick_b = tick;
    printf("%d, %d\n", tick_a, tick_b);

    // usleep(10 * 1000);
  }

  // ソケットクローズ
  close(sockfd);

  return 0;
}

void tajima_2(struct js_event js, int fd) {
  if (js.type == 1 && js.number == 4) {  //　　↑
    if (js.value == 1) {
      ioctl(fd, CMD_MOTOR_LEFT_FOWARD, 0);
      ioctl(fd, CMD_MOTOR_RIGHT_FOWARD, 0);
    }
    if (js.value == 0) {
      ioctl(fd, CMD_MOTOR_LEFT_STOP, 0);
      ioctl(fd, CMD_MOTOR_RIGHT_STOP, 0);
    }
    tick_a = 100;
    tick_b = 100;
  }
}
void tajima_3(struct js_event js, int fd) {
  if (js.type == 1 && js.number == 6) {  //　　↓
    if (js.value == 1) {
      ioctl(fd, CMD_MOTOR_LEFT_BACK, 0);
      ioctl(fd, CMD_MOTOR_RIGHT_BACK, 0);
    }
    if (js.value == 0) {
      ioctl(fd, CMD_MOTOR_LEFT_STOP, 0);
      ioctl(fd, CMD_MOTOR_RIGHT_STOP, 0);
    }
    tick_a = 100;
    tick_b = 100;
  }
}
void tajima_4(struct js_event js, int fd) {
  if (js.type == 1 && js.number == 7) {  //　　←
    if (js.value == 1) {
      ioctl(fd, CMD_MOTOR_LEFT_BACK, 0);
      ioctl(fd, CMD_MOTOR_RIGHT_FOWARD, 0);
    }
    if (js.value == 0) {
      ioctl(fd, CMD_MOTOR_LEFT_STOP, 0);
      ioctl(fd, CMD_MOTOR_RIGHT_STOP, 0);
    }
    tick_a = 100;
    tick_b = 100;
  }
}

void tajima_5(struct js_event js, int fd) {
  if (js.type == 1 && js.number == 5) {  //　　→
    if (js.value == 1) {
      ioctl(fd, CMD_MOTOR_LEFT_FOWARD, 0);
      ioctl(fd, CMD_MOTOR_RIGHT_BACK, 0);
    }
    if (js.value == 0) {
      ioctl(fd, CMD_MOTOR_LEFT_STOP, 0);
      ioctl(fd, CMD_MOTOR_RIGHT_STOP, 0);
    }
    tick_a = 100;
    tick_b = 100;
  }
}
void tajima_6(struct js_event js, int fd) {
  if (js.type == 1 && js.number == 0) {  // SELECT_BUTTON
    if (js.value == 1) {
      ioctl(fd, CMD_GPIO02_VALUE, 1);  // LED点灯させる
    } else if (js.value == 0) {
      ioctl(fd, CMD_GPIO02_VALUE, 0);  // LEDを消灯させる
    }
  }
}
