#pragma once

#if defined(_WIN32)
    #define NOGDI
    #define NOUSER
    #define MMNOSOUND

    #include <Windows.h>

    typedef struct tagMSG {
        HWND   hwnd;
        UINT   message;
        WPARAM wParam;
        LPARAM lParam;
        DWORD  time;
        POINT  pt;
        DWORD  lPrivate;
    } MSG, *PMSG, *NPMSG, *LPMSG;
#endif

#include <raylib.h>

#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/printf.h>

#include <discord-rpc.hpp>

#include <nlohmann/json.hpp>

#include "config.hpp" // IWYU pragma: keep
#include "Geometry.hpp" // IWYU pragma: keep