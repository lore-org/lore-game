#include <thread>
#include <atomic>

#if defined(_WIN32)
/* 
  @raysan5: To avoid conflicting windows.h symbols with raylib, some flags are defined
  WARNING: Those flags avoid inclusion of some Win32 headers that could be required 
  by user at some point and won't be included...
*/

/* If defined, the following flags inhibit definition of the indicated items.*/
#define NOGDICAPMASKS     // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOVIRTUALKEYCODES // VK_*
#define NOWINMESSAGES     // WM_*, EM_*, LB_*, CB_*
#define NOWINSTYLES       // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS      // SM_*
#define NOMENUS           // MF_*
#define NOICONS           // IDI_*
#define NOKEYSTATES       // MK_*
#define NOSYSCOMMANDS     // SC_*
#define NORASTEROPS       // Binary and Tertiary raster ops
#define NOSHOWWINDOW      // SW_*
#define OEMRESOURCE       // OEM Resource values
#define NOATOM            // Atom Manager routines
#define NOCLIPBOARD       // Clipboard routines
#define NOCOLOR           // Screen colors
#define NOCTLMGR          // Control and Dialog routines
#define NODRAWTEXT        // DrawText() and DT_*
#define NOGDI             // All GDI defines and routines
#define NOKERNEL          // All KERNEL defines and routines
#define NOUSER            // All USER defines and routines
#define NONLS             // All NLS defines and routines
#define NOMEMMGR          // GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE        // typedef METAFILEPICT
#define NOMSG             // typedef MSG and associated routines
#define NOOPENFILE        // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL          // SB_* and scrolling routines
#define NOSERVICE         // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND           // Sound driver routines
#define NOTEXTMETRIC      // typedef TEXTMETRIC and associated routines
#define NOWH              // SetWindowsHook and WH_*
#define NOWINOFFSETS      // GWL_*, GCL_*, associated routines
#define NOCOMM            // COMM driver routines
#define NOKANJI           // Kanji support stuff.
#define NOHELP            // Help engine interface.
#define NOPROFILER        // Profiler interface.
#define NODEFERWINDOWPOS  // DeferWindowPos routines
#define NOMCX             // Modem Configuration Extensions
#define MMNOSOUND

/* Type required before windows.h inclusion  */
typedef struct tagMSG *LPMSG;

#include <windows.h>

/* Type required by some unused function...  */
typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG  biWidth;
  LONG  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG  biXPelsPerMeter;
  LONG  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

#include <objbase.h>
#include <mmreg.h>
#include <mmsystem.h>

// Undefine near and far macros to avoid conflicts with parameter names
#ifdef near
#undef near
#endif

#ifdef far
#undef far
#endif

#endif

#include <raylib-cpp.hpp>
#include <fmt/base.h>
#include <fmt/format.h>

#include "Scheduler.hpp"
#include "Director.hpp"

#if defined(_WIN32)
#include "../crash/analyzer.hpp"
inline long WINAPI exceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo) {
    static analyzer::Analyzer analyzer;
    analyzer.analyze(ExceptionInfo);

    auto exceptionInfo = analyzer.getExceptionMessage();
    auto stackTrace = analyzer.getStackTraceMessage();
    auto registerStates = analyzer.getRegisterStateMessage();
    auto stackAllocations = analyzer.getStackAllocationsMessage();

    auto crashReport = fmt::format(
        "== Exception Information ==\n"
        "{}\n\n"
        "== Stack Trace ==\n"
        "{}\n\n"
        "== Register States ==\n"
        "{}\n\n"
        "== Stack Allocations ==\n"
        "{}",
        exceptionInfo,
        stackTrace,
        registerStates,
        stackAllocations
    );

    #define MB_OK        0x00000000L
    #define MB_ICONHAND  0x00000010L
    #define MB_ICONERROR MB_ICONHAND

    ShellMessageBoxA(nullptr, nullptr, crashReport.c_str(), "Something went wrong!", MB_ICONERROR | MB_OK);

    return EXCEPTION_CONTINUE_SEARCH;
}
#endif

inline void SetupEngine() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    raylib::InitWindow(720, 480);

    #if defined(_WIN32)
    SetUnhandledExceptionFilter(exceptionFilter);
    #endif
}

inline std::atomic<bool> stopUpdate(false);
inline void RunEngine() {

    constexpr double ticksPerSecond = 240.f;
    constexpr double secondsPerTicks = 1.f / ticksPerSecond;
    std::thread updateThread([]() {
        while (!::stopUpdate) {
            auto startTime = GetTime();

            Scheduler::sharedScheduler()->update(GetFrameTime() * ticksPerSecond);

            auto endTime = GetTime();

            if (endTime - startTime < secondsPerTicks)
                WaitTime((startTime + secondsPerTicks) - endTime);
        }
    });

    while (!WindowShouldClose()) {
        BeginDrawing();
        
        Director::sharedDirector()->draw(GetFrameTime());

        EndDrawing();
    }

    stopUpdate = true;
    updateThread.join();
    CloseWindow();
}