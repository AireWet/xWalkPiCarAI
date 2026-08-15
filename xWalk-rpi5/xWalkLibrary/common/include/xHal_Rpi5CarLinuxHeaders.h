/******************************************************************************
 * @file        xHal_Rpi5CarLinuxHeaders.h
 * @brief       Aggregates Linux headers required by hardware backends.
 *
 * @details
 * Isolates Linux file, I2C, SPI, GPIO, process, network, user, ioctl, and
 * descriptor APIs from the hardware-independent xWalk HAL implementation.
 *
 * @project     xWalk Firmware
 * @module      xWalkLibraryCommon
 *
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_LINUX_HEADERS_H
#define XHAL_RPI5CAR_LINUX_HEADERS_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#ifdef __linux__
    #include <arpa/inet.h>
    #include <cerrno>
    #include <dirent.h>
    #include <dlfcn.h>
    #include <fcntl.h>
    #include <grp.h>
    #include <ifaddrs.h>
    #include <linux/gpio.h>
    #include <linux/i2c-dev.h>
    #include <linux/i2c.h>
    #include <linux/spi/spidev.h>
    #include <netinet/in.h>
    #include <poll.h>
    #include <pwd.h>
    #include <signal.h>
    #include <spawn.h>
    #include <sys/ioctl.h>
    #include <sys/socket.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

#endif /* XHAL_RPI5CAR_LINUX_HEADERS_H */
