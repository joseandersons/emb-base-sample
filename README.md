# Camera service

The camera service is activated by the Zephyr extra modules feature.

## Enabling the service

1. Add the extra module to your project folder. Only the `camera_service`.
1. Add the module folder to the CMakeLists.txt right before the
   `find_package()` command.

    ```CMakeLists
    cmake_minimum_required(VERSION 3.20.0)

    set(ZEPHYR_EXTRA_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/camera_service/")

    find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
    project(camera_service)

    target_sources(app PRIVATE src/main.c)
    ```

1. To use that, you need to include the service `#include "camera_service.h"`.

The compilation is normal. That should work fine.

### Camera Service dependencies

The camera service directly affects the following symbols:

1. CONFIG_ZBUS=y
1. CONFIG_ZBUS_MSG_SUBSCRIBER=y
1. CONFIG_TEST_RANDOM_GENERATOR=y
1. CONFIG_QEMU_ICOUNT=n

For more details, please see the `Kconfig.camera_service` file.
