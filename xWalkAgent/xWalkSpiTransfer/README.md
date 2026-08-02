# xWalkSpiTransfer

`xWalkSpiTransfer` is a small hardware-independent Agent coordinator for one
caller-owned `XWalkSpi`. It forwards bounded full-duplex requests and owns no
device node, chip select, configuration, or Linux descriptor.

## Host verification

```bash
cmake -S xWalkAgent/xWalkSpiTransfer -B xWalkAgent/xWalkSpiTransfer/build-host -DXWALK_SPI_TRANSFER_BUILD_HOST_TESTS=ON
cmake --build xWalkAgent/xWalkSpiTransfer/build-host --parallel
ctest --test-dir xWalkAgent/xWalkSpiTransfer/build-host --output-on-failure
```

The application creates the Linux backend first, then `XWalkSpi`, and finally
the Agent. Destruction occurs in reverse order.
