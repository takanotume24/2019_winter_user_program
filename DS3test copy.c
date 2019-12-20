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
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/input.h>
#include <linux/joystick.h>

#define SCL 50
#define NAME_LENGTH 128
#define MAX_VALUE_STICK 32767.0
#define MIM_VALUE_STICK -32767.0
#define MAX_VALUE_ARG 100

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

void *fun_thread_enable_a_pwm(void *ptr);
void *fun_thread_enable_b_pwm(void *ptr);

uint16_t enable_a = 0;
uint16_t enable_b = 0;
useconds_t tick_a = 0;
useconds_t tick_b = 0;
int fd_ds3;
int fd_motor;
pthread_t thread_enable_a;
pthread_t thread_enable_b;

static volatile int keep_running = 1;

void intHandler(int dummy) { keep_running = 0; }

int main(int argc, char **argv) {
  signal(SIGINT, intHandler);

  char name[NAME_LENGTH] = "Unknown";
  uint16_t js_time_old = 0;
  uint16_t arg = 0;
  struct js_event js;

  int result_pthread_create_pin_enable_a = 0;
  int result_pthread_create_pin_enable_b = 0;
  int result_pthread_pwm = 0;

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

  if (result_pthread_create_pin_enable_a != 0) {
    printf("faild create pthread pin 05\n");
  }

  if (result_pthread_create_pin_enable_b != 0) {
    printf("faild create pthread pin 06\n");
  }

  while (keep_running) {
    read(fd_ds3, &js, sizeof(struct js_event));
    /*
    if (read(fd_ds3, &js, sizeof(struct js_event)) != sizeof(struct js_event)) {
            perror("\njstest: error reading");
            return 1;
    }*/

    // printf("%d, %d, %d, %d\n", js.time, js.value, js.type, js.number);

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
            break;
        }
        break;
    }
    // printf("tick_a %d, %d\n", tick_a * SCL, ((useconds_t)100 - tick_a) *
    // SCL);
    js_time_old = js.time;

    usleep(10000);  // Choose yourself
  }
  close(fd_motor);
  close(fd_ds3);
  printf("\nexit.\n");
  exit(EXIT_SUCCESS);
}

void *fun_thread_enable_a_pwm(void *ptr) {
  int fd = *(int *)ptr;
  printf("CMD_MOTOR_LEFT_ENABLE duty: %d\n", *(int *)(ptr));
  while (1) {
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
    ioctl(fd, CMD_MOTOR_RIGHT_ENABLE, 1);
    usleep(tick_b * SCL);
    ioctl(fd, CMD_MOTOR_RIGHT_ENABLE, 0);
    usleep(((useconds_t)100 - tick_b) * SCL);
  }
}
