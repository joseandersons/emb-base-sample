cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(emb_base_sample)

target_sources(app PRIVATE
  src/main.c
  src/time_channel.c
  src/wifi.c
  src/sntp_task.c
  src/logger_task.c
  src/app_task.c
)

target_include_directories(app PRIVATE include)
