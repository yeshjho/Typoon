#include "../../low_level/clipboard.h"

#include <Windows.h>
#include <gdiplus.h>


void push_current_clipboard_state()
{

}


void pop_current_clipboard_state()
{
    
}


void set_clipboard_image(const std::filesystem::path& imagePath)
{
    const Gdiplus::GdiplusStartupInput startupInput;
    ULONG_PTR token;
    Gdiplus::GdiplusStartup(&token, &startupInput, nullptr);

    if (Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(imagePath.c_str()))
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
                SetClipboardData(CF_BITMAP, realHBitmap);
                DeleteObject(realHBitmap);
            }
            CloseClipboard();
        }

        DeleteObject(hBitmap);
        delete bitmap;
    }

    Gdiplus::GdiplusShutdown(token);
}


void set_clipboard_text(const std::wstring& text)
{
    
}
