#include "../../low_level/clipboard.h"

#include <Windows.h>
#include <gdiplus.h>

#include "log.h"


UINT clipboard_content_type = 0;
void* clipboard_data = nullptr;


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
        GlobalLock(data);

        switch(clipboard_content_type)
        {
        case CF_TEXT:
        {
            auto* text = static_cast<const char*>(data);
            const size_t size = strlen(text) + 1;
            clipboard_data = new char[size] { 0, };
            strcpy_s(static_cast<char*>(clipboard_data), size, text);
            break;
        }

        case CF_UNICODETEXT:
        {
            auto* text = static_cast<const wchar_t*>(data);
            const size_t size = wcslen(text) + 1;
            clipboard_data = new wchar_t[size] { 0, };
            wcscpy_s(static_cast<wchar_t*>(clipboard_data), size, text);
            break;
        }

        case CF_BITMAP:
        {
            const auto hBitmap = static_cast<HBITMAP>(data);
            BITMAP bitmap;
            GetObject(hBitmap, sizeof(BITMAP), &bitmap);

            const HDC sourceHdc = CreateCompatibleDC(nullptr);
            SelectObject(sourceHdc, hBitmap);

            clipboard_data = CreateCompatibleBitmap(sourceHdc, bitmap.bmWidth, bitmap.bmHeight);
            const HDC targetHdc = CreateCompatibleDC(nullptr);
            SelectObject(targetHdc, clipboard_data);

            BitBlt(targetHdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, sourceHdc, 0, 0, SRCCOPY);

            DeleteDC(targetHdc);
            DeleteDC(sourceHdc);
            break;
        }

        // TODO: Handle other clipboard types (Do we really have to?)

        default:
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
        const size_t len = (strlen(text) + 1) * sizeof(char);
        const HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, len);
        strcpy_s(static_cast<char*>(GlobalLock(mem)), len, text);
        GlobalUnlock(mem);
        SetClipboardData(CF_TEXT, mem);
        break;
    }

    case CF_UNICODETEXT:
    {
        EmptyClipboard();

        const auto* text = static_cast<const wchar_t*>(clipboard_data);
        const size_t len = (wcslen(text) + 1) * sizeof(wchar_t);
        const HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, len);
        wcscpy_s(static_cast<wchar_t*>(GlobalLock(mem)), len, text);
        GlobalUnlock(mem);
        SetClipboardData(CF_UNICODETEXT, mem);
        break;
    }

    case CF_BITMAP:
        EmptyClipboard();

        SetClipboardData(CF_BITMAP, clipboard_data);
        break;

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
        delete[] static_cast<char*>(clipboard_data);
        break;

    case CF_UNICODETEXT:
        delete[] static_cast<wchar_t*>(clipboard_data);
        break;

    case CF_BITMAP:
        DeleteObject(clipboard_data);
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
    constexpr std::chrono::milliseconds delay{ 200 };
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


void set_clipboard_image(const std::filesystem::path& imagePath)
{
    const Gdiplus::GdiplusStartupInput startupInput;
    ULONG_PTR token;
    Gdiplus::GdiplusStartup(&token, &startupInput, nullptr);

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
                    log_last_error(L"Failed to set clipboard data:");
                }
                DeleteObject(realHBitmap);
            }
            CloseClipboard();
        }
        else
        {
            log_last_error(L"Failed to open clipboard:");
        }

        DeleteObject(hBitmap);
        delete bitmap;
    }
    else
    {
        log_last_error(L"Failed to load image from file:");
    }

    Gdiplus::GdiplusShutdown(token);
}


void set_clipboard_text(const std::wstring& text)
{
    
}
