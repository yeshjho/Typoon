#include "../../low_level/clipboard.h"

#include <Windows.h>
#include <gdiplus.h>

#include "../../low_level/tray_icon.h"
#include "log.h"


UINT clipboard_content_type = 0;
void* clipboard_data = nullptr;
size_t clipboard_data_size = 0;


constexpr UINT CLIPBOARD_FORMAT_IMAGE = 49161;


void push_current_clipboard_state()
{
    if (!OpenClipboard(nullptr))
    {
        log_last_error(L"Failed to open clipboard:");
        return;
    }

    pop_clipboard_state_without_restoring();

    if (clipboard_content_type = EnumClipboardFormats(0);
        clipboard_content_type != 0)
    {
        const HANDLE data = GetClipboardData(clipboard_content_type);

        switch(clipboard_content_type)
        {
        case CF_TEXT:
        {
            GlobalLock(data);

            auto* text = static_cast<const char*>(data);
            const size_t len = strlen(text) + 1;
            clipboard_data = new char[len] { 0, };
            strcpy_s(static_cast<char*>(clipboard_data), len, text);
            clipboard_data_size = len * sizeof(char);

            GlobalUnlock(data);
            break;
        }

        case CF_UNICODETEXT:
        {
            GlobalLock(data);

            auto* text = static_cast<const wchar_t*>(data);
            const size_t len = wcslen(text) + 1;
            clipboard_data = new wchar_t[len] { 0, };
            wcscpy_s(static_cast<wchar_t*>(clipboard_data), len, text);
            clipboard_data_size = len * sizeof(wchar_t);

            GlobalUnlock(data);
            break;
        }

        // TODO: It seems like there's no way to get the actual data of the image from the clipboard.
        /*
        case CLIPBOARD_FORMAT_IMAGE:
        {
            GlobalLock(data);
            
            clipboard_data_size = GlobalSize(data);
            clipboard_data = new char[clipboard_data_size] { 0, };
            std::memcpy(clipboard_data, data, clipboard_data_size);

            GlobalUnlock(data);
            break;
        }
        */

        // TODO: Handle other clipboard types (Do we really have to?)
        default:
            logger.Log(ELogLevel::INFO, "Unhandled clipboard format", clipboard_content_type);
            break;
        }
    }

    CloseClipboard();
}


void pop_clipboard_state()
{
    if (!OpenClipboard(nullptr))
    {
        log_last_error(L"Failed to open clipboard:");
        return;
    }

    switch (clipboard_content_type)
    {
    case CF_TEXT:
    {
        EmptyClipboard();

        const auto* text = static_cast<const char*>(clipboard_data);
        const HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, clipboard_data_size);
        strcpy_s(static_cast<char*>(GlobalLock(mem)), clipboard_data_size / sizeof(char), text);
        GlobalUnlock(mem);
        SetClipboardData(CF_TEXT, mem);
        break;
    }

    case CF_UNICODETEXT:
    {
        EmptyClipboard();

        const auto* text = static_cast<const wchar_t*>(clipboard_data);
        const HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, clipboard_data_size);
        wcscpy_s(static_cast<wchar_t*>(GlobalLock(mem)), clipboard_data_size / sizeof(wchar_t), text);
        GlobalUnlock(mem);
        SetClipboardData(CF_UNICODETEXT, mem);
        break;
    }

    /*
    case CLIPBOARD_FORMAT_IMAGE:
    {
        EmptyClipboard();

        const HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, clipboard_data_size);
        std::memcpy(GlobalLock(mem), clipboard_data, clipboard_data_size);
        GlobalUnlock(mem);
        SetClipboardData(CLIPBOARD_FORMAT_IMAGE, mem);
        break;
    }
    */

    default:
        break;
    }

    CloseClipboard();

    pop_clipboard_state_without_restoring();
}


void pop_clipboard_state_without_restoring()
{
    switch (clipboard_content_type)
    {
    case CF_TEXT:
    case CLIPBOARD_FORMAT_IMAGE:
        delete[] static_cast<char*>(clipboard_data);
        break;

    case CF_UNICODETEXT:
        delete[] static_cast<wchar_t*>(clipboard_data);
        break;

    default:
        break;
    }

    clipboard_content_type = 0;
    clipboard_data = nullptr;
}


std::thread clipboard_state_restorer;


void pop_clipboard_state_with_delay(std::function<bool()> predicate)
{
    // We never know how long does the simulated paste take. This is the best we can do.
    constexpr std::chrono::milliseconds delay{ 500 };
    if (clipboard_state_restorer.joinable())
    {
        clipboard_state_restorer.join();
    }
    clipboard_state_restorer = std::thread{
        [predicate = std::move(predicate)]()
        {
            std::this_thread::sleep_for(delay);
            if (predicate())
            {
                pop_clipboard_state();
            }
        }
    };
}


bool set_clipboard_image(const std::filesystem::path& imagePath)
{
    const Gdiplus::GdiplusStartupInput startupInput;
    ULONG_PTR token;
    Gdiplus::GdiplusStartup(&token, &startupInput, nullptr);

    bool toReturn = true;
    const auto lambdaOnError = [&toReturn](const std::wstring& msg, LogLevel logLevel = ELogLevel::ERROR)
        {
            toReturn = false;
            const std::wstring errorStr = get_last_error_string();
            logger.Log(logLevel, msg, errorStr);
            show_notification(msg, errorStr);
        };

    if (Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(imagePath.c_str());
        bitmap->GetLastStatus() == Gdiplus::Status::Ok)
    {
        const unsigned int propSize = bitmap->GetPropertyItemSize(PropertyTagOrientation);
        auto* propertyItem = static_cast<Gdiplus::PropertyItem*>(operator new(propSize));
        if (bitmap->GetPropertyItem(PropertyTagOrientation, propSize, propertyItem) == Gdiplus::Ok)
        {
            switch (*static_cast<UINT16*>(propertyItem->value))
            {
            // case 1: No rotation

            case 2:
                bitmap->RotateFlip(Gdiplus::RotateNoneFlipX);
                break;

            case 3:
                bitmap->RotateFlip(Gdiplus::Rotate180FlipNone);
                break;

            case 4:
                bitmap->RotateFlip(Gdiplus::RotateNoneFlipY);
                break;

            case 5:
                bitmap->RotateFlip(Gdiplus::Rotate90FlipX);
                break;

            case 6:
                bitmap->RotateFlip(Gdiplus::Rotate90FlipNone);
                break;

            case 7:
                bitmap->RotateFlip(Gdiplus::Rotate90FlipY);
                break;

            case 8:
                bitmap->RotateFlip(Gdiplus::Rotate270FlipNone);
                break;

            default:
                break;
            }
        }
        delete propertyItem;

        /// Most of this code is from espanso (https://github.com/espanso/espanso/blob/dev/espanso-clipboard/src/win32/native.cpp)
        HBITMAP hBitmap;
        bitmap->GetHBITMAP(0, &hBitmap);
        if (OpenClipboard(nullptr))
        {
            EmptyClipboard();
            DIBSECTION ds;
            if (GetObject(hBitmap, sizeof(DIBSECTION), &ds))
            {
                const HDC hdc = GetDC(HWND_DESKTOP);
                const HBITMAP realHBitmap = CreateDIBitmap(hdc, &ds.dsBmih, CBM_INIT,
                    ds.dsBm.bmBits, reinterpret_cast<BITMAPINFO*>(&ds.dsBmih), DIB_RGB_COLORS);
                ReleaseDC(HWND_DESKTOP, hdc);
                if (!SetClipboardData(CF_BITMAP, realHBitmap))
                {
                    lambdaOnError(L"Failed to set clipboard data");
                }
                DeleteObject(realHBitmap);
            }
            CloseClipboard();
        }
        else
        {
            lambdaOnError(L"Failed to open clipboard");
        }

        DeleteObject(hBitmap);
        delete bitmap;
    }
    else
    {
        lambdaOnError(L"Failed to load image from file");
    }

    Gdiplus::GdiplusShutdown(token);
    return toReturn;
}


void set_clipboard_text(const std::wstring& text)
{
    if (!OpenClipboard(nullptr))
    {
        log_last_error(L"Failed to open clipboard:");
        return;
    }

    EmptyClipboard();

    const size_t len = text.size() + 1;
    const HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, len * sizeof(wchar_t));
    wcscpy_s(static_cast<wchar_t*>(GlobalLock(mem)), len, text.c_str());
    GlobalUnlock(mem);
    SetClipboardData(CF_UNICODETEXT, mem);

    CloseClipboard();
}


void end_clipboard_storer()
{
    pop_clipboard_state_without_restoring();

    if (clipboard_state_restorer.joinable())
    {
        clipboard_state_restorer.join();
    }
}
