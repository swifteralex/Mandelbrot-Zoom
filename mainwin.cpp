#include <windows.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")
#include "basewin.h"
#include <vector>

class MainWindow : public BaseWindow<MainWindow> {
private:
	ID2D1Factory* pFactory = 0;
	ID2D1HwndRenderTarget* pRenderTarget = 0;
	ID2D1SolidColorBrush* pBrush = 0;
	HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);

public:
	void DrawScene();
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	PCWSTR ClassName() const { return L"Basic Window Class"; }
};

void MainWindow::DrawScene() {
	RECT rc;
	GetClientRect(m_hwnd, &rc);
	D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
	pRenderTarget->Resize(size);
	
	ID2D1Bitmap* scene;
	UINT8* Data = (UINT8*)malloc((LONGLONG)size.height * size.width * 4);
	for (UINT Y = 0; Y < size.height; Y++) {
		for (UINT X = 0; X < size.width; X++) {
			UINT8* PixelData = Data + ((Y * (LONGLONG)size.width) + X) * 4;
			PixelData[0] = 0;
			PixelData[1] = (X + Y) % 255;
			PixelData[2] = (int)(X * 0.05 + Y * 0.05) % 255;
			PixelData[3] = 255;
		}
	}
	pRenderTarget->CreateBitmap(
		size,
		Data,
		size.width * 4,
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
		&scene
	);
	free(Data);
	pRenderTarget->BeginDraw();
	pRenderTarget->DrawBitmap(scene, D2D1::RectF(0, 0, (FLOAT)size.width, (FLOAT)size.height));
	scene->Release();
	pRenderTarget->EndDraw();
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_SIZE: {
			if (wParam != SIZE_MINIMIZED) {
				DrawScene();
			}
			return 0;
		}

		case WM_SETCURSOR: {
			if (LOWORD(lParam) == HTCLIENT) {
				SetCursor(hCursor);
				return TRUE;
			} else {
				return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
			}
		}

		case WM_GETMINMAXINFO: {
			MINMAXINFO* info = (MINMAXINFO*)lParam;
			info->ptMinTrackSize.x = 300;
			info->ptMinTrackSize.y = 300;
			return 0;
		}

		case WM_CREATE: {
			D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
			RECT rc;
			GetClientRect(m_hwnd, &rc);
			D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
			pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(m_hwnd, size), &pRenderTarget);
			pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0F, 0, 0), &pBrush);
			return 0;
		}

		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}

		default: {
			return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
		}
	}
}

// Starting part for the program; creates the main window and gets messages for it
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {
	MainWindow win;

	if (!win.Create(hInstance, L"Mandelbrot Zoom", WS_TILEDWINDOW)) {
		return 0;
	}
	ShowWindow(win.Window(), nCmdShow);

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}