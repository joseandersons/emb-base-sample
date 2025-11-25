tui enable
set pagination off
target remote :1234
b main.c:29
b camera_service.c:67
c
