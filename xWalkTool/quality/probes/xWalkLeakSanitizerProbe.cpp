#include <cstdlib>

volatile void* xwalkLeakProbePointer = nullptr;

int main()
{
    xwalkLeakProbePointer = std::malloc(64U);
    return xwalkLeakProbePointer == nullptr ? 1 : 0;
}
