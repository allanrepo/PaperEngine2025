#pragma once
#include <Utilities/Logger.h>
#include <Utilities/Utilities.h>
#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include <array>

namespace Win32
{
	class GDIUtility
	{
	private:
		// helper function to calculate the size of the texture required to hold all ASCII characters with the given font type, size and margin between characters
		static bool CalculateTextureSize(
			HFONT hFont, // handle to the font object
			HDC hDC,  // handle to the device context where the font is selected
			const float margin, // margin between characters in pixels
			float& textureWidth, // output parameter for the calculated texture width
			float& textureHeight // output parameter for the calculated texture height
		)
		{
			// ensure the font and device context are valid
			if (!hFont || !hDC) 
			{
				LOGERROR("Invalid font or device context.");
				return false;
			}

			// given the font object, let's load it into DC
			HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);

			// ensure we restore the old font when we're done
			utilities::OnOutOfScope cleanup([&] 
			{
				SelectObject(hDC, hOldFont);
			});

			// the target texture widths we're targeting are 256, 512, and 1024
			// these counters will be incremented for every row needed to hold all characters
			int numRowsIf256 = 1; // this will count how many rows we need if texture width is 256
			int numRowsIf512 = 1; // this will count how many rows we need if texture width is 512
			int numRowsIf1024 = 1; // this will count how many rows we need if texture width is 1024

			// the target texture height we're targeting is 256, 512, or 1024
			// these counters will be incremented for every row needed to hold all characters
			// it will reset back to 0 if moving to new row
			float width256 = 0; 
			float width512 = 0;
			float width1024 = 0;

			// reference to the maximum character height
			// initialize it to 0, and we will find the maximum height of all characters
			float maxHeight = 0;

			// iterate through all ASCII characters. value of first ASCII character is 32, and last is 127
			for (int nChar = 32; nChar <= 127; nChar++)
			{
				// Get the character size
				SIZE sizeChar;
				WCHAR wChar = static_cast<WCHAR>(nChar);
				if (!GetTextExtentPoint32(hDC, &wChar, 1, &sizeChar))
				{
					LOGERROR("Failed to get text extent for character: " << nChar);
					return false;
				}

				// add margin between characters. we add margin only if this is not the last character and is not the first character in the row
				if (width256 > 0) width256 += margin;
				if (width512 > 0) width512 += margin;
				if (width1024 > 0) width1024 += margin;

				// save max height. if current max height < this char's height, then max height = this char's height
				if (maxHeight < sizeChar.cy) maxHeight = static_cast<float>(sizeChar.cy);

				// if adding this character at the end of current row will make the row width > 256, let's move to new row
				if (width256 + sizeChar.cx + margin > 256)
				{
					numRowsIf256++;
					width256 = 0;
				}

				// if adding this character at the end of current row will make the row width > 512, let's move to new row
				if (width512 + sizeChar.cx + margin > 512)
				{
					numRowsIf512++;
					width512 = 0;
				}

				// if adding this character at the end of current row will make the row width > 1024, let's move to new row
				if (width1024 + sizeChar.cx + margin > 1024)
				{
					numRowsIf1024++;
					width1024 = 0;
				}

				// add this character to our row width for each texture size
				width256 += sizeChar.cx;
				width512 += sizeChar.cx;
				width1024 += sizeChar.cx;
			}

			// calculate the expected height of the texture for each width size
			// let's calculate the expected height of the texture for all 3 width sizes
			float heightFor256 = numRowsIf256 * maxHeight; // if texture width is 256, this will be the texture height required to hold all characters
			float heightFor512 = numRowsIf512 * maxHeight; // if texture width is 512, this will be the texture height required to hold all characters
			float heightFor1024 = numRowsIf1024 * maxHeight; // if texture width is 1024, this will be the texture height required to hold all characters

			// let's find the appropriate texture size based on the calculated height
			
			// let's try first if 256x256 texture size is good enough
			if (heightFor256 < 256)
			{
				textureWidth = 256;
				textureHeight = 256;
			}
			// or maybe 512x256
			else if (heightFor512 < 256)
			{
				textureWidth = 512;
				textureHeight = 256;
			}
			// or 512x512
			else if (heightFor512 < 512)
			{
				textureWidth = 512;
				textureHeight = 512;
			}
			// if we need to use a 1024 width texture, then we find the height starting at 256
			else
			{
				textureWidth = 1024;
				float h = 256;
				while (true)
				{
					if (heightFor1024 < h)
					{
						textureHeight = h;
						break;
					}
					h += 256;
				}
			}
			SelectObject(hDC, hOldFont);

			return true;
		}

	public:
		static bool GenerateFontAtlas(
			unsigned int** ppData, // pointer to the font data, which will be allocated by this function. caller is responsible for freeing this memory
			unsigned int& width, // is this total width of the font resource if stored in texture??? TODO
			unsigned int& height, // is this total height of the font resource if stored in texture??? TODO
			std::vector<std::array<float, 4>>& TextNormalizedCoords, // array of normalized coordinates for each character in the font, where each array contains {left, top, right, bottom} coordinates in normalized texture coordinates (0.0 to 1.0) float??? TODO
			const std::string& name = "Arial", // font name, e.g. "Arial". we're not sure if this font name is supported. it is best to query for it first
			const unsigned int size = 12, // font size in ??? TODO
			const bool italic = false, // if true, the font will be italicized
			const bool bold = false, // if true, the font will be bold
			const bool underline = false, // if true, the font will be underlined
			const bool strike = false // if true, the font will be struck through
			)
		{
#pragma region // create GDI font object
			//Create a DC for GDI instructions. 
			HDC hDC = CreateCompatibleDC(NULL);

			// ensure HDC is cleaned up when it goes out of scope
			utilities::OnOutOfScope cleanupHDC([=]
				{
					if(hDC) DeleteDC(hDC);
				});

			if (!hDC)
			{
				LOGERROR("Failed to create compatible DC for GDI font creation.");
				return false;
			}

			//Setup the DC for text drawing
			SetTextColor(hDC, RGB(255, 255, 255));
			SetBkColor(hDC, 0x00000000);
			SetTextAlign(hDC, TA_TOP);
			SetMapMode(hDC, MM_TEXT);

			// calculate actual height of font
			int nFontHeight = MulDiv(size, (int)GetDeviceCaps(hDC, LOGPIXELSY), 72);

			// convert to wide string char. GDI's CreateFont takes wide string for its name parameter
			std::wstring wname(name.begin(), name.end());

			// create the font via GDI instructions
			HFONT hFont = ::CreateFont(nFontHeight, 0, 0, 0, bold ? 600 : 0, italic, underline, strike, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, VARIABLE_PITCH, wname.c_str());

			// ensure HFONT is cleaned up when it goes out of scope
			utilities::OnOutOfScope cleanupHFONT([=]
				{
					if (hFont) DeleteObject(hFont);
				});

			// if failed to create font...
			if (!hFont)
			{
				LOGERROR("Failed to create GDI font: " << name);
				return false;
			}
#pragma endregion

#pragma region // calculate texture size required to hold all ASCII characters
			float textureWidth, textureHeight;
			float charMargin = 20;
			if (!CalculateTextureSize(hFont, hDC, charMargin, textureWidth, textureHeight))
			{
				LOGERROR("Failed to calculate texture size for GDI font: " << name);
				return false;
			}
#pragma endregion

#pragma region create GDI bitmap resource 
			// Select the font into the DC so it will use that font
			HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);
			utilities::OnOutOfScope restoreHDC([=]
				{
					if(hOldFont)SelectObject(hDC, hOldFont);
				});

			// Figure out how big the bitmap and texture should be to hold the font.
			int nTextureWidth = static_cast<int>(textureWidth);
			int nTextureHeight = static_cast<int>(textureHeight);

			width = nTextureWidth;
			height = nTextureHeight;

			//First, create a bitmap of the characters using GDI calls. The 
			//height is negated because of the way bitmaps are encoded.
			unsigned long* pBitmapBits = nullptr;
			BITMAPINFO BitmapInfo = {};
			ZeroMemory(&BitmapInfo.bmiHeader, sizeof(BITMAPINFOHEADER));
			BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			BitmapInfo.bmiHeader.biWidth = (int)nTextureWidth;
			BitmapInfo.bmiHeader.biHeight = -(int)nTextureHeight;
			BitmapInfo.bmiHeader.biPlanes = 1;
			BitmapInfo.bmiHeader.biCompression = BI_RGB;
			BitmapInfo.bmiHeader.biBitCount = 32;
			HBITMAP hBitmap = CreateDIBSection(hDC, &BitmapInfo, DIB_RGB_COLORS, (void**)&pBitmapBits, NULL, 0);

			// ensure HBITMAP is cleaned up when it goes out of scope
			utilities::OnOutOfScope cleanupHBITMAP([=]
				{
					if (hBitmap) DeleteObject(hBitmap);
				});

			// if failed to create bitmap...
			if (!hBitmap)
			{
				LOGERROR("Failed to create GDI bitmap for font: " << name);
				return false;
			}
#pragma endregion

#pragma region // write all ASCII characters into resource and save its normalized texture coordinates
			// Select the new bitmap into the DC.
			HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDC, hBitmap);

			utilities::OnOutOfScope restoreHBITMAP([=]
				{
					if(hOldBitmap) SelectObject(hDC, hOldBitmap);
				});

			// Now, loop through all the printable ASCII characters (32 - 127) and draw them to the bitmap. 
			// As they are being drawn, record their positions in our array of texture coordinates.
			long x = 0;
			long y = 0;
			SIZE sizeChar;
			int nCharIndex;
			int maxHeight = 0;

			for (int nChar = 32; nChar <= 127; nChar++)
			{
				//Get the character size
				GetTextExtentPoint32(hDC, (LPCWSTR)&nChar, 1, &sizeChar);

				if (maxHeight < sizeChar.cy) maxHeight = sizeChar.cy;

				//Move to the next line if need be.
				if (x + sizeChar.cx + charMargin > (int)nTextureWidth)
				{
					x = 0;
					y += maxHeight;// nFontHeight;
				}

				// check if our texture size is enough to be filled with all characters
				if (y + sizeChar.cy > (int)nTextureHeight)
				{
					LOGERROR("Texture size is not enough to hold all characters. Increase the texture size or reduce the font size.");
					return false;
				}

				//Draw the character
				ExtTextOut(hDC, x, y, ETO_OPAQUE, NULL, (LPCTSTR)&nChar, 1, NULL);

				// we started counting at 32, but we begin storing our coordinates at index 0 of the array
				nCharIndex = nChar - 32;

				// normalize the texture coordinates. value is a ratio [0:1] with respect to texture size.
				std::array<float, 4> texCoord = {};
				texCoord[0] = (float)(x) / (float)nTextureWidth; // left
				texCoord[1] = (float)(y) / (float)nTextureHeight; // top
				texCoord[2] = (float)(x + sizeChar.cx) / (float)nTextureWidth; // right
				texCoord[3] = (float)(y + sizeChar.cy) / (float)nTextureHeight; // bottom
				TextNormalizedCoords.push_back(texCoord);

				// Update the x position, but make sure there's enough space so that characters don't overlap.
				// set to 10 as minimum. if lower, some font types may get messed up. try papyrus at 1.
				// DEBUG:	even when set to 10, if font is papyrus size 96, some part of 'j' is rendered when rendering 'i'. 
				//			setting to 20 fixes it. note that changing this does not change the width of characters
				//			it's only effect is it changes distance between characters in font texture.
				x += sizeChar.cx + static_cast<int>(charMargin);
			}
#pragma endregion

#pragma region store pixel data from bitma into pointer array
			* ppData = new unsigned int[nTextureWidth * nTextureHeight];
			memset(*ppData, 0, sizeof(unsigned int)* nTextureWidth* nTextureHeight);
			for (int y = 0; y < (int)nTextureHeight; y++)
			{
				for (int x = 0; x < (int)nTextureWidth; x++)
				{
					// do this if you want to fill specific color to all pixels. useful for debugging
					//colorBuffer[x + y * nTextureSize] = 0xff0000ff;

					unsigned long value = *(pBitmapBits + (y * nTextureWidth + x));

					// we know the text color is white and background is black. if pixel is for text, change its alpha to 0xFF. else, to 0x00
					unsigned long color = value & 0x00ffffff;
					if (color)
					{
						value |= 0xff000000;
					}
					(*ppData)[x + y * nTextureWidth] = value;
				}
			}
#pragma endregion

			return true;
		}
	};
}
