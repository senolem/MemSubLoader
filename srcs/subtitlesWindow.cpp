#include "MemSubLoader.hpp"

LRESULT CALLBACK subtitlesWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_PAINT:
		{
			Gdiplus::Bitmap subtitlesBitmap(SUBTITLES_WIDTH, SUBTITLES_HEIGHT, PixelFormat32bppARGB);
			HBITMAP bmp;
			Gdiplus::Graphics graphics(&subtitlesBitmap);
			HDC hdc = GetDC(hwnd);

			RectF rect(0, 0, SUBTITLES_WIDTH, SUBTITLES_HEIGHT);
			Gdiplus::StringFormat format;
			format.SetAlignment(getConfigAlignment());
			format.SetLineAlignment(Gdiplus::StringAlignment::StringAlignmentFar);
			Gdiplus::Font font(hdc, &config.subtitlesFont);
			Gdiplus::Color fontColor(GetRValue(config.subtitlesColor), GetGValue(config.subtitlesColor), GetBValue(config.subtitlesColor));
			Gdiplus::SolidBrush solidBrush(fontColor);

			graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
			graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
			graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
			graphics.Clear(Gdiplus::Color(0, 0, 0, 0));
			graphics.DrawString(textToDraw.c_str(), -1, &font, rect, &format, &solidBrush);
			subtitlesBitmap.GetHBITMAP(Color(0, 0, 0, 0), &bmp);
			
			HDC memdc = CreateCompatibleDC(hdc);
			HGDIOBJ original = SelectObject(memdc, bmp);

			BLENDFUNCTION blend = { 0 };
			blend.BlendOp = AC_SRC_OVER;
			blend.SourceConstantAlpha = 255;
			blend.AlphaFormat = AC_SRC_ALPHA;
			POINT ptLocation = { SUBTITLES_XPOS, SUBTITLES_YPOS };
			SIZE szWnd = { SUBTITLES_WIDTH, SUBTITLES_HEIGHT };
			POINT ptSrc = { 0, 0 };
			UpdateLayeredWindow(hwnd, hdc, &ptLocation, &szWnd, memdc, &ptSrc, 0, &blend, ULW_ALPHA);
			SelectObject(hdc, original);
	
			DeleteObject(bmp);
			DeleteObject(memdc);
		}
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int createSubtitlesWindow(void)
{
	const wchar_t SETTINGS_CLASS_NAME[] = L"subtitlesWindowClass";
	WNDCLASS subtitlesWindowClass = {};

	subtitlesWindowClass.lpfnWndProc = subtitlesWindowProc;
	subtitlesWindowClass.hInstance = GetModuleHandle(NULL);
	subtitlesWindowClass.lpszClassName = SETTINGS_CLASS_NAME;
	RegisterClass(&subtitlesWindowClass);

	subtitlesHWND = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT, SETTINGS_CLASS_NAME, L"Subtitles", WS_VISIBLE | WS_POPUP, SUBTITLES_XPOS, SUBTITLES_YPOS, SUBTITLES_WIDTH, SUBTITLES_HEIGHT, NULL, NULL, NULL, NULL);
	if (!IsWindow(subtitlesHWND))
	{
		return 1;
	}
	SetWindowPos(subtitlesHWND, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	return 0;
}