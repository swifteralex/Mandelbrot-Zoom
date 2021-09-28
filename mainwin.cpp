#include <windows.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")
#include "basewin.h"
#include <complex>

class MainWindow : public BaseWindow<MainWindow> {
private:
	ID2D1Factory* pFactory = 0;
	ID2D1HwndRenderTarget* pRenderTarget = 0;
	ID2D1SolidColorBrush* pBrush = 0;
	HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);
	std::complex<double> top_left = std::complex<double>(-2, 1.3);
	double zoom = -2.15;
	D2D1_POINT_2F last_pos = D2D1::Point2F(0, 0);
	bool panning = false;

public:
	void DrawScene();
	std::complex<double> CanvasCoordToActual(UINT X, UINT Y);
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
			std::complex<double> z = std::complex<double>(0, 0);
			std::complex<double> c = CanvasCoordToActual(X, Y);
			bool not_in_set = false;
			for (int i = 0; i < 100; i++) {
				if (abs(z) >= 2) {
					not_in_set = true;
					break;
				}
				z = z * z + c;
			}

			UINT8* PixelData = Data + ((Y * (LONGLONG)size.width) + X) * 4;
			PixelData[0] = not_in_set * 255;
			PixelData[1] = not_in_set * 255;
			PixelData[2] = not_in_set * 255;
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

std::complex<double> MainWindow::CanvasCoordToActual(UINT X, UINT Y) {
	std::complex<double> res = std::complex<double>((double)X * pow(10, zoom), -((double)Y * pow(10, zoom)));
	res += top_left;
	return res;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_LBUTTONDOWN: {
			panning = true;
			return 0;
		}

		case WM_LBUTTONUP: {
			panning = false;
			return 0;
		}

		case WM_MOUSEWHEEL: {
			GET_WHEEL_DELTA_WPARAM(wParam);
			if ((int)wParam > 0) {
				double real = (last_pos.x) * pow(10, zoom) / 4.9;
				double imag = -(last_pos.y) * pow(10, zoom) / 4.9;
				top_left += std::complex<double>(real, imag);
				zoom -= 0.1;
			} else {
				double real = (last_pos.x) * pow(10, zoom) / 3.9;
				double imag = -(last_pos.y) * pow(10, zoom) / 3.9;
				top_left -= std::complex<double>(real, imag);
				zoom += 0.1;
			}
			DrawScene();
			return 0;
		}

		case WM_MOUSEMOVE: {
			float x = LOWORD(lParam);
			float y = HIWORD(lParam);
			if (panning) {
				float x_diff = (last_pos.x - x) * pow(10, zoom);
				float y_diff = (y - last_pos.y) * pow(10, zoom);
				std::complex<double> c = std::complex<double>((double)x_diff, (double)y_diff);
				top_left += c;
				DrawScene();
			}
			last_pos = D2D1::Point2F(x, y);
			return 0;
		}

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