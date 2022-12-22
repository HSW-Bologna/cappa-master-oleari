import os
from pathlib import Path
import platform

LIBRARY = "freertos-simulator.a"
INCLUDES = [".", "windows_symlinks", "linux_symlinks", "Kernel", "Kernel/include", "Demo/Common/include"]

# mingw port
MINGW_FILES = ["Kernel/portable/MSVC-MingW/port.c"] 
# posix port
GCC_FILES = ['Kernel/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c', 'Kernel/portable/ThirdParty/GCC/Posix/port.c']
COMMON_FILES = [
    'simulator_main.c',
    # Memory manager (use malloc() / free() )
    'Kernel/portable/MemMang/heap_3.c',
    # Demo library.
    'Demo/Common/Minimal/AbortDelay.c',
    'Demo/Common/Minimal/BlockQ.c',
    'Demo/Common/Minimal/blocktim.c',
    'Demo/Common/Minimal/countsem.c',
    'Demo/Common/Minimal/death.c',
    'Demo/Common/Minimal/dynamic.c',
    'Demo/Common/Minimal/EventGroupsDemo.c',
    'Demo/Common/Minimal/flop.c',
    'Demo/Common/Minimal/GenQTest.c',
    'Demo/Common/Minimal/integer.c',
    'Demo/Common/Minimal/IntSemTest.c',
    'Demo/Common/Minimal/MessageBufferAMP.c',
    'Demo/Common/Minimal/MessageBufferDemo.c',
    'Demo/Common/Minimal/PollQ.c',
    'Demo/Common/Minimal/QPeek.c',
    'Demo/Common/Minimal/QueueOverwrite.c',
    'Demo/Common/Minimal/QueueSet.c',
    'Demo/Common/Minimal/QueueSetPolling.c',
    'Demo/Common/Minimal/recmutex.c',
    'Demo/Common/Minimal/semtest.c',
    'Demo/Common/Minimal/StaticAllocation.c',
    'Demo/Common/Minimal/StreamBufferDemo.c',
    'Demo/Common/Minimal/StreamBufferInterrupt.c',
    'Demo/Common/Minimal/TaskNotify.c',
    'Demo/Common/Minimal/TimerDemo.c',
    ] 


if platform.system() == "Windows":
    files = COMMON_FILES + MINGW_FILES
    INCLUDES += ['Kernel/portable/MSVC-MingW']
else:
    files = COMMON_FILES + GCC_FILES
    INCLUDES += ['Kernel/portable/ThirdParty/GCC/Posix', 
            'Kernel/portabl/ThirdParty/GCC/Posix/utils']

Import('freertos_env')
freertos_env['CPPPATH'] += INCLUDES
sources = Glob('Kernel/*.c') + [File(f) for f in files]


lib = freertos_env.Library(LIBRARY, sources)

result = (lib, list(map(lambda x: os.path.join(os.getcwd(), x), INCLUDES)))
Return('result')
