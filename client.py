# coding: UTF-8
import socket

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    # サーバを指定
    s.connect(('127.0.0.1', 1234))
    # サーバにメッセージを送る
    i = 0
    while 1:
        string = "hello" + str(i)
        s.sendall(string.encode())
        # ネットワークのバッファサイズは1024。サーバからの文字列を取得する
        data = s.recv(1024)
        #
        print(repr(data))