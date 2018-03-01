// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "TGALoader.h"

#include <stdio.h>
#include <string.h>
#include <GL/glew.h>

using namespace std;

namespace NS_TGALOADER
{
	// cannot directly read this struct, since it would need to be tightly packed for that to work
/*	struct TGAHeader
	{
		unsigned char  	ucIdent;		// size of ID field that follows 18 byte header (0 usually)
		unsigned char  	ucColorMapType;	// type of colour map 0=none, 1=has palette
		unsigned char  	ucImageType;    // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

		short 			colourmapstart;	// first colour map entry in palette
		short 			colourmaplength;// number of colours in palette
		unsigned char	colourmapbits;  // number of bits per palette entry 15,16,24,32

		short 			iXStart;  		// image x origin
		short 			iYStart;  		// image y origin
		short 			iWidth;   		// image width in pixels
		short 			iHeight;  		// image height in pixels
		unsigned char  	uiBits;			// image bits per pixel 8,16,24,32
		unsigned char  	uiDescriptor;	// image descriptor bits (vh flip bits)
	};*/


	IMAGE::IMAGE ()
	{
		m_iImageWidth = 0;
		m_iImageHeight = 0;
		m_iBytesPerPixel = 0;
	}

	bool IMAGE::LoadTGA (const char* szFile)
	{
		FILE* pFile = fopen (szFile, "rb");

		if (!pFile)
			return (false);

		// Read the header of the TGA, compare it with the known headers for compressed and uncompressed TGAs
		unsigned char ucHeader[18];
		fread (ucHeader, sizeof (unsigned char) * 18, 1, pFile);

		while (ucHeader[0] > 0)
		{
			--ucHeader[0];

			unsigned char temp;
			fread (&temp, sizeof (unsigned char), 1, pFile);
		}

		m_iImageWidth = ucHeader[13] * 256 + ucHeader[12];
		m_iImageHeight = ucHeader[15] * 256 + ucHeader[14];
		m_iBytesPerPixel = ucHeader[16] / 8;


		// check whether width, height an BitsPerPixel are valid
		if ((m_iImageWidth <= 0) || (m_iImageHeight <= 0) || ((m_iBytesPerPixel != 1) && (m_iBytesPerPixel != 3) && (m_iBytesPerPixel != 4)))
		{
			fclose (pFile);
			return (false);
		}

		// allocate the image-buffer
		m_Pixels.resize (m_iImageWidth * m_iImageHeight * 4);


		// call the appropriate loader-routine
		if (ucHeader[2] == 2)
		{
			LoadUncompressedTGA (pFile);
		}
		else 
		if (ucHeader[2] == 10)
		{
			LoadCompressedTGA (pFile);
		}
		else
		{
			fclose (pFile);
			return (false);
		}

		fclose (pFile);

		return (true);
	}

	void IMAGE::LoadUncompressedTGA (FILE* pFile)
	{
		unsigned char ucBuffer[4] = {255, 255, 255, 255};

		unsigned int* pIntPointer = (unsigned int*) &m_Pixels[0];
		unsigned int* pIntBuffer = (unsigned int*) &ucBuffer[0];

		const int iPixelCount	= m_iImageWidth * m_iImageHeight;

		for (int i = 0; i < iPixelCount; ++i)
		{
			fread (ucBuffer, sizeof (unsigned char) * m_iBytesPerPixel, 1, pFile);

			// if this is an 8-Bit TGA only, store the one channel in all four channels
			// if it is a 24-Bit TGA (3 channels), the fourth channel stays at 255 all the time, since the 4th value in ucBuffer is never overwritten
			if (m_iBytesPerPixel == 1)
			{
				ucBuffer[1] = ucBuffer[0];
				ucBuffer[2] = ucBuffer[0];
				ucBuffer[3] = ucBuffer[0];
			}

			// copy all four values in one operation
			(*pIntPointer) = (*pIntBuffer);
			++pIntPointer;
		}
	}

	void IMAGE::LoadCompressedTGA (FILE* pFile)
	{
		int iCurrentPixel	= 0;
		int iCurrentByte	= 0;
		unsigned char ucBuffer[4] = {255, 255, 255, 255};
		const int iPixelCount	= m_iImageWidth * m_iImageHeight;

		unsigned int* pIntPointer = (unsigned int*) &m_Pixels[0];
		unsigned int* pIntBuffer = (unsigned int*) &ucBuffer[0];

		do
		{
			unsigned char ucChunkHeader = 0;

			fread (&ucChunkHeader, sizeof (unsigned char), 1, pFile);

			if (ucChunkHeader < 128)
			{
				// If the header is < 128, it means it is the number of RAW color packets minus 1
				// that follow the header
				// add 1 to get number of following color values

				ucChunkHeader++;	

				// Read RAW color values
				for (int i = 0; i < (int) ucChunkHeader; ++i)	
				{
					fread (&ucBuffer[0], m_iBytesPerPixel, 1, pFile);

					// if this is an 8-Bit TGA only, store the one channel in all four channels
					// if it is a 24-Bit TGA (3 channels), the fourth channel stays at 255 all the time, since the 4th value in ucBuffer is never overwritten
					if (m_iBytesPerPixel == 1)
					{
						ucBuffer[1] = ucBuffer[0];
						ucBuffer[2] = ucBuffer[0];
						ucBuffer[3] = ucBuffer[0];
					}

					// copy all four values in one operation
					(*pIntPointer) = (*pIntBuffer);

					++pIntPointer;
					++iCurrentPixel;
				}
			}
			else // chunkheader > 128 RLE data, next color reapeated (chunkheader - 127) times
			{
				ucChunkHeader -= 127;	// Subteact 127 to get rid of the ID bit

				// read the current color
				fread (&ucBuffer[0], m_iBytesPerPixel, 1, pFile);

				// if this is an 8-Bit TGA only, store the one channel in all four channels
				// if it is a 24-Bit TGA (3 channels), the fourth channel stays at 255 all the time, since the 4th value in ucBuffer is never overwritten
				if (m_iBytesPerPixel == 1)
				{
					ucBuffer[1] = ucBuffer[0];
					ucBuffer[2] = ucBuffer[0];
					ucBuffer[3] = ucBuffer[0];
				}

				// copy the color into the image data as many times as dictated 
				for (int i = 0; i < (int) ucChunkHeader; ++i)
				{
					(*pIntPointer) = (*pIntBuffer);
					++pIntPointer;

					++iCurrentPixel;
				}
			}
		}
		while (iCurrentPixel < iPixelCount);
	}
}



