#include <fstream>

#include "gamegraphics.h"
#include "utils.h"
#include "main.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb-master\stb_image_write.h"

char path_uw0[100];
char path_uw1[100];
char path_uw2[100];
char path_shock[100];

//For use in the cutscene decoder.
typedef __int32 unsigned Uint32;
typedef __int16 unsigned Uint16;
typedef __int8 unsigned Uint8;

typedef __int32 signed Sint32;
typedef __int16 signed Sint16;
typedef __int8 signed Sint8;

typedef struct {
//Borrowed from Underworld Adventures for extracting cutscenes.
	Uint32 id;    /* 4 character ID == "LPF " */
	Uint16 maxLps;    /* max # largePages allowed. 256 FOR NOW.   */
	Uint16 nLps;    /* # largePages in this file. */
	Uint32 nRecords;  /* # records in this file.  65534 is current limit plus */
	/* one for last-to-first delta for looping the animation */
	Uint16 maxRecsPerLp;  /* # records permitted in an lp. 256 FOR NOW.   */
	Uint16 lpfTableOffset;  /* Absolute Seek position of lpfTable.  1280 FOR NOW.
							The lpf Table is an array of 256 large page structures
							that is used to facilitate finding records in an anim
							file without having to seek through all of the Large
							Pages to find which one a specific record lives in. */
	Uint32 contentType;  /* 4 character ID == "ANIM" */
	Uint16 width;    /* Width of screen in pixels. */
	Uint16 height;    /* Height of screen in pixels. */
	Uint8 variant;  /*   0==ANIM. */
	Uint8 version;  /*   0==frame rate is multiple of 18 cycles/sec.
					1==frame rate is multiple of 70 cycles/sec.  */
	Uint8 hasLastDelta;  /* 1==Last record is a delta from last-to-first frame. */
	Uint8 lastDeltaValid;  /* 0==The last-to-first delta (if present) hasn't been
						   updated to match the current first&last frames,  so it
						   should be ignored. */
	Uint8 pixelType;  /* 0==256 color. */
	Uint8 CompressionType;  /* 1==(RunSkipDump) Only one used FOR NOW. */
	Uint8 otherRecsPerFrm;  /* 0 FOR NOW. */
	Uint8 bitmaptype;  /*   1==320x200, 256-color.  Only one implemented so far. */
	Uint8 recordTypes[32];  /* Not yet implemented. */
	Uint32 nFrames;   /*   In case future version adds other records at end of
					  file, we still know how many actual frames.
					  NOTE: DOES include last-to-first delta when present. */
	Uint16 framesPerSecond;  /* Number of frames to play per second. */
	Uint16 pad2[29];  /* 58 bytes of filler to round up to 128 bytes total. */
	} lpfileheader;

/* this is the format of an large page structure 
Borrowed from Underworld adventures
*/
typedef struct {
	Uint16 baseRecord;  /* Number of first record in this large page. */
	Uint16 nRecords;  /* Number of records in lp.
					  bit 15 of "nRecords" == "has continuation from previous lp".
					  bit 14 of "nRecords" == "final record continues on next lp". */
	Uint16 nBytes;    /* Total number of bytes of contents, excluding header. */
	} lp_descriptor;

void myPlayRunSkipDump(Uint8 *srcP, Uint8 *dstP);

void extractUW2Bitmaps(char filePathIn[255],char PaletteFile[255],int PaletteNo,char OutFileName[255], int useTGA)
{
	unsigned char *textureFile;          // Pointer to our buffered data (little endian format)
	int i;
	long NoOfTextures;

	FILE *file = NULL;      // File pointer
    if ((file = fopen(filePathIn, "rb")) == NULL)
       { fprintf(LOGFILE,"Could not open specified file\n"); return;}

    // Get the size of the file in bytes
    long fileSize = getFileSize(file);
    textureFile = new unsigned char[fileSize];
    fread(textureFile, fileSize, 1,file);
	fclose(file); 

	for (int p = 0; p <= 16; p++)
		{
		palette *pal;
		pal = new palette[256];
		//getPalette(PaletteFile, pal, PaletteNo);
		getPalette(PaletteFile, pal, p);
		NoOfTextures = getValAtAddress(textureFile, 0, 8);
		char OutFileNameToUse[255];
		sprintf_s(OutFileNameToUse, 255, "%d_%s_",p, OutFileName);
		for (i = 0; i <NoOfTextures; i++)
			{
			long textureOffset = getValAtAddress(textureFile, (i * 4) + 6, 32);
			if (textureOffset != 0)
				{
				int compressionFlag = getValAtAddress(textureFile, ((i * 4) + 6) + (NoOfTextures * 4), 32);
				int isCompressed = (compressionFlag >> 1) & 0x01;
				if (isCompressed == 1)
					{
					int datalen;
					unsigned char *outputImg = unpackUW2(textureFile, textureOffset, &datalen);
					if (useTGA == 1)
						{
						writeTGA(outputImg, 0, 320, 200, i, pal, OutFileNameToUse, 0);
						}
					else
						{
						writeBMP(outputImg, 0, 320, 200, i, pal, OutFileNameToUse);
						}

					}
				else
					{
					if (useTGA == 1)
						{
						writeTGA(textureFile, textureOffset, 320, 200, i, pal, OutFileNameToUse, 0);
						}
					else
						{
						writeBMP(textureFile, textureOffset, 320, 200, i, pal, OutFileNameToUse);
						}
					}
				}
			}
		}

}

void extractTextureBitmap(int ImageCount, char filePathIn[255], char PaletteFile[255], int PaletteNo, int BitmapSize, int FileType, char OutFileName[255],char auxPalPath[255],int useTGA)
{
	unsigned char *textureFile;          // Pointer to our buffered data (little endian format)
	int i;
	long NoOfTextures;
	int MaxHeight=0; int MaxWidth=0;

	FILE *file = NULL;      // File pointer
	
    if ((file = fopen(filePathIn, "rb")) == NULL)
       { fprintf(LOGFILE,"Could not open specified file\n"); return;}

    // Get the size of the file in bytes
    long fileSize = getFileSize(file);
    
	palette *pal;
	pal = new palette[256];


//Extract all palettes
	unsigned char Allpalettefile[256*16];

	for (int j = 0; j < 16; j++)
		{
		for (int i = 0; i < 256; i++)
			{//create a blank image with incremental values
			Allpalettefile[i+j*256] = i;
			}
		}


	for (int p = 0; p < 1; p++)
		{//Get the 8 palettes
		getPalette(PaletteFile, pal, p);

//Not to cycle the palettes. To keep them synced I need 28 cycles? (4 water * 7 fire frames?)
		for (int paletteFrame = 0; paletteFrame < 1; paletteFrame++)
			{
			//	CyclePalette(16, 22);//fire
			//CyclePalette(48, 51);//water
			//cyclePalette(pal, 48, 4);
			//cyclePalette(pal, 16, 7);//Reverse direction.
			char paletteOutname[256];
			sprintf(paletteOutname, "Palette_%d_frame", p);
			writeTGA(Allpalettefile, 0, 256,16, paletteFrame, pal, paletteOutname, 1);
			}

		}


		//getPalette(PaletteFile, pal, p);

	if (GREYSCALE == 1)
		{
		getPaletteIndex(PaletteFile,pal,PaletteNo);
		}
	else
		{
		getPalette(PaletteFile, pal, PaletteNo);
		}

	



    // Allocate space in the buffer for the whole file
    //BigEndBuf = new unsigned char[fileSize];
	textureFile = new unsigned char[fileSize];
    // Read the file in to the buffer
    fread(textureFile, fileSize, 1,file);
	fclose(file); 

	//long PrevAddress = 0;

	switch (FileType)
	{
		case UW_GRAPHICS_BITMAPS:	//BYT
			if (useTGA==1)
				{
				writeTGA(textureFile, 0, 320, 200, 0, pal,OutFileName,ALPHA);
				}
			else
				{
				writeBMP(textureFile, 0, 320, 200, 0, pal,OutFileName);
				}
			break;
		case UW_GRAPHICS_TEXTURES :	//.tr
			fprintf(LOGFILE,"File Type :%d\n",  textureFile[0]);
			fprintf(LOGFILE,"xy resolution:%d\n", textureFile[1]);
			fprintf(LOGFILE,"No of textures:%d\n", textureFile[3] << 8 | textureFile[2]);
			if (ImageCount == -1)	//All the images.
				{
				NoOfTextures = textureFile[3] << 8 | textureFile[2];
				NoOfTextures=256;//Count is wrong???
				}
			else
				{
				NoOfTextures = ImageCount;
				}
			for (i = 0; i <NoOfTextures; i++)
				{
				long textureOffset = getValAtAddress(textureFile, (i * 4) + 4, 32);
				if (useTGA==1)
					{
					writeTGA(textureFile, textureOffset, BitmapSize, BitmapSize, i, pal, OutFileName,0);
					}
				else
					{
					writeBMP(textureFile, textureOffset, BitmapSize, BitmapSize, i, pal, OutFileName);
					}
				}
			break;
		case UW_GRAPHICS_GR :	//.gr
		case UW_GRAPHICS_TR :
		case UW_GRAPHICS_CR :
		case UW_GRAPHICS_SR :
		case UW_GRAPHICS_AR :
			fprintf(LOGFILE,"File Type (should be %d):%d\n", FileType, textureFile[0]);
			fprintf(LOGFILE,"No of textures:%d\n", textureFile[2] << 8 | textureFile[1]);
			if (ImageCount == -1)	//All the images.
			{
				NoOfTextures = textureFile[2] << 8 | textureFile[1];
			}
			else
			{
				NoOfTextures = ImageCount;
			}
			for (i = 0; i < NoOfTextures; i++)
			{
				long textureOffset = getValAtAddress(textureFile, (i * 4) + 3, 32);
				//textureOffset = 679 + i * 1000;

				int BitMapWidth = getValAtAddress(textureFile,textureOffset+1, 8);
				int BitMapHeight = getValAtAddress(textureFile, textureOffset+2, 8);
				fprintf(LOGFILE, "\tImage %d \t %d\n", i, textureOffset);
				if (MaxHeight < BitMapHeight)
					{MaxHeight=BitMapHeight;
					}
				if (MaxWidth < BitMapWidth)
					{
					MaxWidth = BitMapWidth;
					}
				int datalen;
				palette auxpal[16];
				int auxPalIndex;
				unsigned char *imgNibbles;
				unsigned char *outputImg;
				switch (getValAtAddress(textureFile, textureOffset, 8))
					{
					case 0x4://8 bit uncompressed
						//fprintf(LOGFILE,"8 bit uncompressed\n");
						//fprintf(LOGFILE,"Width = %d\n",getValAtAddress(textureFile, textureOffset + 1, 8));
						//fprintf(LOGFILE,"Height = %d\n", getValAtAddress(textureFile, textureOffset + 2, 8));
						textureOffset = textureOffset + 5;
						if (useTGA==1)
							{
							writeTGA(textureFile, textureOffset, BitMapWidth, BitMapHeight, i, pal, OutFileName, ALPHA);
							}
						else
							{
							writeBMP(textureFile, textureOffset, BitMapWidth, BitMapHeight, i, pal, OutFileName);
							}
						break;
					case 0x8://4 bit run-length
						fprintf(LOGFILE,"4 bit run-length\n");
						//fprintf(LOGFILE,"Width = %d\n", getValAtAddress(textureFile, textureOffset + 1, 8));
						//fprintf(LOGFILE,"Height = %d\n", getValAtAddress(textureFile, textureOffset + 2, 8));
						auxPalIndex = getValAtAddress(textureFile, textureOffset + 3, 8);
						//fprintf(LOGFILE,"Aux Pal = %d\n", auxPalIndex);
						//fprintf(LOGFILE,"Data length = %d\n", getValAtAddress(textureFile, textureOffset + 4, 16));
						datalen = getValAtAddress(textureFile,textureOffset+4,16);
						imgNibbles = new unsigned char[BitMapWidth*BitMapHeight*2];

						textureOffset = textureOffset + 6;	//Start of raw data.
						copyNibbles(textureFile, imgNibbles, datalen, textureOffset);
						LoadAuxilaryPal(auxPalPath, auxpal, pal,auxPalIndex);
						outputImg = new unsigned char[BitMapWidth*BitMapHeight];
//						DecodeRLEBitmap(imgNibbles, datalen, BitMapWidth, BitMapHeight, outputImg, auxpal, i, 4,OutFileName);
						DecodeRLEBitmap(imgNibbles, datalen, BitMapWidth, BitMapHeight, outputImg,4);
						if (useTGA==1)
							{
							writeTGA(outputImg, 0, BitMapWidth, BitMapHeight, i, auxpal, OutFileName, ALPHA);
							}
						else
							{
							writeBMP(outputImg, 0, BitMapWidth, BitMapHeight, i, auxpal, OutFileName);
							}
						
						break;
					case 0xA://4 bit uncompressed
						fprintf(LOGFILE,"4 bit uncompressed\n");
						//fprintf(LOGFILE,"Width = %d\n", getValAtAddress(textureFile, textureOffset + 1, 8));
						//fprintf(LOGFILE,"Height = %d\n", getValAtAddress(textureFile, textureOffset + 2, 8));
						auxPalIndex = getValAtAddress(textureFile, textureOffset + 3, 8);
						//fprintf(LOGFILE,"Aux Pal = %d\n", auxPalIndex);
						//fprintf(LOGFILE,"Data length = %d\n", getValAtAddress(textureFile, textureOffset + 4, 16));
						datalen = getValAtAddress(textureFile, textureOffset + 4, 16);
						imgNibbles = new unsigned char[BitMapWidth*BitMapHeight * 2];

						textureOffset = textureOffset + 6;	//Start of raw data.
						copyNibbles(textureFile, imgNibbles, datalen, textureOffset);
						LoadAuxilaryPal(auxPalPath, auxpal, pal, auxPalIndex);
						//outputImg = new unsigned char[BitMapWidth*BitMapHeight];
						if (useTGA==1)
							{
							writeTGA(imgNibbles, 0, BitMapWidth, BitMapHeight, i, auxpal, OutFileName, ALPHA);
							}
						else
							{
							writeBMP(imgNibbles, 0, BitMapWidth, BitMapHeight, i, auxpal, OutFileName);
							}
						
						break;
					default:
						fprintf(LOGFILE,"Unknown file type : %d\n", getValAtAddress(textureFile, textureOffset, 8));
						break;
					}
				
			}
			break;
	}


	//NoOfTextures=0;
	//fprintf(LOGFILE,"Address of first block:%d\n",  (textureFile[7]<<16 | textureFile[6]<<32 | textureFile[5]<<8 | textureFile[4]));

	printf("Max Width: %d, Max Height: %d", MaxWidth, MaxHeight);
}

void getPalette(const char filePathPal[255], palette *pal, int paletteNo)
{
	FILE *filePal = NULL;
	unsigned char *palf;
	int j; int i;
	
	//const char *filePathPal = GRAPHICS_PAL_FILE	;	//"C:\\Games\\Ultima\\UW1\\DATA\\pals.dat"; 
	filePal = fopen(filePathPal,"rb");
	long fileSizePal = getFileSize(filePal);
	palf = new unsigned char[fileSizePal];
	fread(palf, fileSizePal, 1, filePal);
	fclose (filePal);
	i = 0;
int palAddr = paletteNo * 256;
	for (j = 0; j < 256; j++) {
		pal[i].red = palf[palAddr + 0]<<2;//getValAtAddress(palf, palAddr + 0, 8);//palf[j * 3 + 2] << 2;
		pal[i].green = palf[palAddr + 1] << 2;//getValAtAddress(palf, palAddr +1, 8);//palf[j * 3 + 0] << 2;
		pal[i].blue = palf[palAddr + 2] << 2;//getValAtAddress(palf, palAddr + 2, 8);//palf[j * 3 + 1] << 2;
		pal[i].reserved = 0;
		palAddr = palAddr +3;
		i++;
	}

	unsigned char *xfer;
	filePal = fopen("c:\\games\\uw2\\data\\xfer.dat", "rb");
	 fileSizePal = getFileSize(filePal);
	xfer = new unsigned char[fileSizePal];
	fread(xfer , fileSizePal, 1, filePal);
	unsigned char *xferOut = new unsigned char[fileSizePal*16];

	fclose(filePal);
	writeTGA(xfer, 0, fileSizePal, 1, 0, pal, "xferpal.tga", 0);
return;
}

void getPaletteIndex(char filePathPal[255], palette *pal, int paletteNo)
	{
//Returns a palette comprising of just index numbers
	FILE *filePal = NULL;
	unsigned char *palf;
	short j; int i;

	//const char *filePathPal = GRAPHICS_PAL_FILE	;	//"C:\\Games\\Ultima\\UW1\\DATA\\pals.dat"; 
	filePal = fopen(filePathPal, "rb");
	long fileSizePal = getFileSize(filePal);
	palf = new unsigned char[fileSizePal];
	fread(palf, fileSizePal, 1, filePal);
	fclose(filePal);
	
	//FILE *PALOUT_FILE = NULL;
	//fopen_s(&PALOUT_FILE, "pal.txt", "w");

	i = 0;
	int palAddr = paletteNo * 256;
	for (j = 0; j < 256; j++) {
		int red;
		int green;
		int blue;
		pal[i].red = j;//<<2;//getValAtAddress(palf, palAddr + 0, 8);//palf[j * 3 + 2] << 2;
		pal[i].green = j;// << 2;//getValAtAddress(palf, palAddr +1, 8);//palf[j * 3 + 0] << 2;
		pal[i].blue = j ;//<< 2;//getValAtAddress(palf, palAddr + 2, 8);//palf[j * 3 + 1] << 2;
		pal[i].reserved = 0;

		red = palf[palAddr + 0] << 2;//getValAtAddress(palf, palAddr + 0, 8);//palf[j * 3 + 2] << 2;
		green = palf[palAddr + 1] << 2;//getValAtAddress(palf, palAddr +1, 8);//palf[j * 3 + 0] << 2;
		blue = palf[palAddr + 2] << 2;//getValAtAddress(palf, palAddr + 2, 8);//palf[j * 3 + 1] << 2;
		

		//fprintf(PALOUT_FILE,"SetPal(%d,%d,%d,%d);\n",i,red,green,blue);
		palAddr = palAddr + 3;
		i++;
		}
	//fclose(PALOUT_FILE);
	return;
	}


void writeBMP(unsigned char *bits, long Start, long SizeH, long SizeV, int index, palette *pal, const char OutFileName[255])
{
if (ACTUALLY_EXTRACT_FILES == 0)
	{
	return;
	}
	BitMapHeader bmhead;
	BitMapInfoHeader bmihead;
	
	bmhead.bfType = 19778;
	bmhead.bfReserved1 = 0;
	bmhead.bfReserved2 = 0;
	bmhead.bfOffBits = 1078;
	bmihead.biSize = 40;
	bmihead.biPlanes = 1;
	bmihead.biBitCount = 8;
	bmihead.biCompression = 0;
	bmihead.biXPelsPerMeter = 0;
	bmihead.biYPelsPerMeter = 0;
	bmihead.biClrUsed = 0;
	bmihead.biClrImportant = 0;
	
	bmhead.bfOffBits = 1078; // Set up the .bmp header info
	bmihead.biBitCount = 8;
	bmihead.biClrUsed = 0;
	bmihead.biClrImportant = 0;
	

	//unsigned char *bits = unpackdat + substart + 28;
	bmihead.biWidth = SizeH;	//bm->width;
	bmihead.biHeight =SizeV;	// bm->height;
	int imwidth = SizeH;		//bmihead.biWidth;
	imwidth += (4-(imwidth%4));
	bmihead.biSizeImage = imwidth * bmihead.biHeight;
	bmhead.bfSize = bmihead.biSizeImage + 54;
	
	char outFile[255];

	sprintf_s(outFile, 255, "%s_%04d.bmp", OutFileName, index);

	FILE *outf ;
	outf = fopen(outFile,"wb");
	
	fwrite(&bmhead.bfType,2,1,outf);
	fwrite(&bmhead.bfSize,4,1,outf);
	fwrite(&bmhead.bfReserved1,2,1,outf);
	fwrite(&bmhead.bfReserved2,2,1,outf);
	fwrite(&bmhead.bfOffBits,4,1,outf);
	fwrite(&bmihead,sizeof(BitMapInfoHeader),1,outf);
	fwrite(pal,256*4,1,outf);
	char ch = 0;
	for (int k = bmihead.biHeight-1; k >= 0; k--) {
		fwrite(Start+bits+(k*bmihead.biWidth),1,bmihead.biWidth,outf);
		if (bmihead.biWidth%4 != 0)
			for (int buf = 4; buf > bmihead.biWidth%4; buf--)
				fwrite(&ch,1,1,outf);
	}
	//fprintf(LOGFILE,".");
	fclose(outf);	
}

void writeBMPBW(unsigned char *bits, long Start, long SizeH, long SizeV, int index, palette *pal, char OutFileName[255])
	{
	if (ACTUALLY_EXTRACT_FILES == 0)
		{
		return;
		}
	BitMapHeader bmhead;
	BitMapInfoHeader bmihead;

	bmhead.bfType = 19778;
	bmhead.bfReserved1 = 0;
	bmhead.bfReserved2 = 0;
	bmhead.bfOffBits = 62;
	bmihead.biSize = 40;
	bmihead.biPlanes = 1;
	bmihead.biBitCount = 8;
	bmihead.biCompression = 0;
	bmihead.biXPelsPerMeter = 0;
	bmihead.biYPelsPerMeter = 0;
	bmihead.biClrUsed = 0;
	bmihead.biClrImportant = 0;

	bmhead.bfOffBits = 1078; // Set up the .bmp header info
	bmihead.biBitCount = 1;
	bmihead.biClrUsed = 0;
	bmihead.biClrImportant = 0;


	//unsigned char *bits = unpackdat + substart + 28;
	bmihead.biWidth = SizeH;	//bm->width;
	bmihead.biHeight = SizeV;	// bm->height;
	int imwidth = SizeH;		//bmihead.biWidth;
	imwidth += (4 - (imwidth % 4));
	bmihead.biSizeImage = imwidth * bmihead.biHeight;
	bmhead.bfSize = bmihead.biSizeImage + 54;

	char outFile[255];

	sprintf_s(outFile, 255, "%s_%04d.bmp", OutFileName, index);

	FILE *outf;
	outf = fopen(outFile, "wb");

	fwrite(&bmhead.bfType, 2, 1, outf);
	fwrite(&bmhead.bfSize, 4, 1, outf);
	fwrite(&bmhead.bfReserved1, 2, 1, outf);
	fwrite(&bmhead.bfReserved2, 2, 1, outf);
	fwrite(&bmhead.bfOffBits, 4, 1, outf);
	fwrite(&bmihead, sizeof(BitMapInfoHeader), 1, outf);
	//fwrite(pal, 256 * 4, 1, outf);
	//Black&White palette?
	short int tmpshrt = 0;
	fwrite(&tmpshrt, 2, 1, outf);
	fwrite(&tmpshrt, 2, 1, outf);
	tmpshrt = -1;
	fwrite(&tmpshrt, 2, 1, outf);
	tmpshrt = 255;
	fwrite(&tmpshrt, 2, 1, outf);

	char ch = 0;
	for (int k = bmihead.biHeight - 1; k >= 0; k--) {
		fwrite(Start + bits + (k*bmihead.biWidth), 1, bmihead.biWidth, outf);
		if (bmihead.biWidth % 4 != 0)
		for (int buf = 4; buf > bmihead.biWidth % 4; buf--)
			fwrite(&ch, 1, 1, outf);
		}



	//fprintf(LOGFILE,".");
	fclose(outf);


	}

void LoadAuxilaryPal(char auxpalPath[255], palette auxpal[16], palette gamepal[256], int PalIndex)
{
	FILE *filePal = NULL;
	unsigned char *palf;
	//const char *filePathPal = GRAPHICS_PAL_FILE	;	//"C:\\Games\\Ultima\\UW1\\DATA\\pals.dat"; 
	filePal = fopen(auxpalPath, "rb");
	long fileSizePal = getFileSize(filePal);
	palf = new unsigned char[fileSizePal];
	fread(palf, fileSizePal, 1, filePal);
	fclose(filePal);

	for (int j = 0; j < 16; j++)
	{
		//auxpal[j]. = getValAtAddress(palf,PalIndex*0xf + j,8);
		int value = getValAtAddress(palf, PalIndex * 16 + j, 8);
		
		//auxpal[j].green = ((value) & 0xF) <<1;
		//auxpal[j].blue = ((value) >> 4 & 0xF) << 1;
		//auxpal[j].red = ((value) >> 8 & 0xF) << 1;
		//auxpal[j].reserved = 0;
		auxpal[j].green = gamepal[value].green;
		auxpal[j].blue = gamepal[value].blue;
		auxpal[j].red = gamepal[value].red;
		auxpal[j].reserved = gamepal[value].reserved;

		//fprintf(LOGFILE,"%d\n", auxpal[j]);
	}
	return;

}

void DecodeRLEBitmap(unsigned char *imageData, int datalen, int imageWidth, int imageHeight, unsigned char *outputImg , int BitSize)
//, palette *auxpal, int index, int BitSize, char OutFileName[255])
{
int state=0; 
int curr_pxl=0;
int count=0;
int repeatcount=0;
char nibble;

int add_ptr=0;

while ((curr_pxl<imageWidth*imageHeight) || (add_ptr<=datalen))
{
	switch (state)
	{
		case repeat_record_start:
			{
			count = getcount(imageData, &add_ptr,BitSize);
			if (count == 1)
				{
				state = run_record;
				}
			else if (count == 2)
				{
				repeatcount = getcount(imageData, &add_ptr, BitSize) - 1;
				state = repeat_record_start;
				}
			else
				{
				state = repeat_record;
				}
				break;
			}
		case repeat_record:
			{
				//fprintf(LOGFILE,"\nRepeatRecord\n");
				//fprintf(LOGFILE,"Count is %d\n",count);
				//get nibble for the palette;
				nibble = getNibble(imageData, &add_ptr);
				//for count times copy the palette data to the image at the output pointer
				if (imageWidth*imageHeight - curr_pxl < count)
					{
						count = imageWidth*imageHeight - curr_pxl;
					}
				for (int i = 0; i < count; i++)
					{
					//fprintf(LOGFILE,"%d=%d\n", curr_pxl, nibble);
					outputImg[curr_pxl++] = nibble;
					}
				if (repeatcount == 0 )
					{					
					state = run_record;
					}
				else
					{
					state = repeat_record_start;
					repeatcount--;
					}
				break;
			}


	case 2:	//runrecord
		{
		//fprintf(LOGFILE,"\nRunRecord\n");
		count = getcount(imageData, &add_ptr, BitSize);
		if (imageWidth*imageHeight - curr_pxl < count)
			{
				count = imageWidth*imageHeight - curr_pxl;
			}
		//fprintf(LOGFILE,"Count is %d\n", count);
			//for that count copy the data / pal as it is
			for (int i = 0; i < count; i++)
			{
				//get nibble for the palette;
				nibble = getNibble(imageData, &add_ptr);
				//fprintf(LOGFILE,"%d=%d\n", curr_pxl, nibble);
				outputImg[curr_pxl++] = nibble;
			}
			state = repeat_record_start;
			break;
		}
	}
}
}

int getcount(unsigned char *nibbles, int *addr_ptr , int size)
{
int n1;
int n2;
int n3;
//int inc_ptr;
int count=0;

	n1 = getNibble(nibbles,addr_ptr);
	count = n1;

	if (count==0)
		{
		n1 = getNibble(nibbles, addr_ptr);
		n2 = getNibble(nibbles, addr_ptr);
		count = (n1 << size) | n2;
		}
	if (count == 0)
		{
		n1 = getNibble(nibbles, addr_ptr);
		n2 = getNibble(nibbles, addr_ptr);
		n3 = getNibble(nibbles, addr_ptr);
		count = (((n1 << size) | n2) << size) | n3;
		}
	return count;		
}

int getNibble(unsigned char *nibbles, int *addr_ptr)
{
	int n1 = nibbles[*addr_ptr];
	*addr_ptr = *addr_ptr + 1;
	return n1;
}

void copyNibbles(unsigned char *InputData, unsigned char *OutputData, int NoOfNibbles, int add_ptr)
{
//Split the data up into in's nibbles.
int i = 0;
NoOfNibbles=NoOfNibbles*2;

	while (NoOfNibbles > 1)
	{
		OutputData[i] = (getValAtAddress(InputData, add_ptr, 8) >> 4) & 0x0F;		//High nibble
		OutputData[i + 1] = (getValAtAddress(InputData, add_ptr, 8)) & 0xf;	//Low nibble
		//fprintf(LOGFILE,"%d,%d\n", OutputData[i], OutputData[i+1]);
		i=i+2;
		add_ptr++;
		NoOfNibbles = NoOfNibbles-2;
	}
	if (NoOfNibbles == 1)
		{	//Odd nibble out.
		OutputData[i] = (getValAtAddress(InputData, add_ptr, 8) >> 4) & 0x0F;
		}
}

void copyNibbles5Bit(unsigned char *InputData, unsigned char *OutputData, int NoOfNibbles, int add_ptr)
{
int bit_ptr=0;
int bits_needed=5;
int bits_avail=7;
int mask[6];
int i=0;
mask[0] = 0x0;
mask[1] = 0x1;
mask[2] = 0x3;
mask[3] = 0x7;
mask[4] = 0xF;
mask[5] = 0x1F;
//mask[6] = 0x3F;
//mask[7] = 0xFF;
	while (NoOfNibbles > 0)
		{
		unsigned char buf;
		fprintf(LOGFILE,"\n**%d***\n", i);
		//read in my byte and take the needed bits from it
		if (bit_ptr<=2)
			{
			bits_avail=5;
			}
		else 
			{
			bits_avail = 8 - bit_ptr;
			}
		if (bits_avail == 0)
			{
			add_ptr++;
			bit_ptr=0;
			bits_avail=5;
			}
		fprintf(LOGFILE,"\nReading in %d bits @ %d",bits_avail,bit_ptr);
		buf = (InputData[add_ptr] >> (bit_ptr)) & mask[bits_avail];
		bit_ptr=bit_ptr+bits_avail;
		bits_needed= bits_needed-bits_avail;
	
		if (bits_needed > 0)
			{//I need more bits. Read in the next byte and take whats there.
			fprintf(LOGFILE,"\nI need %d bits", bits_needed);
				add_ptr++;
				bit_ptr = 0;
				fprintf(LOGFILE,"\nReading in %d bits @ %d", bits_needed, bit_ptr);
				buf = ((InputData[add_ptr] & mask[bits_needed])<<bits_needed) | buf;
				bit_ptr=bits_needed;
			}
		OutputData[i]=buf;
		i++;
		bits_needed=5;
		NoOfNibbles--;
		}
}

void extractPanels(int ImageCount, char filePathIn[255], char PaletteFile[255], int PaletteNo, int BitmapSize, int FileType, int game, char OutFileName[255], int useTGA)
{
	//const char *filePathIn = GRAPHICS_FILE ; //"C:\\Games\\Ultima\\UW1\\DATA\\W64.tr"; 
	//    int indexNo;
	//unsigned char *BigEndBuf;          // Pointer to our buffered data (big endian format)
	unsigned char *textureFile;          // Pointer to our buffered data (little endian format)
	int i;
	long NoOfTextures;

	FILE *file = NULL;      // File pointer

	if ((file = fopen(filePathIn, "rb")) == NULL)
	{
		fprintf(LOGFILE,"Could not open specified file\n"); return;
	}

	// Get the size of the file in bytes
	long fileSize = getFileSize(file);

	palette *pal;
	pal = new palette[256];
	getPalette(PaletteFile, pal, PaletteNo);

	// Allocate space in the buffer for the whole file
	//BigEndBuf = new unsigned char[fileSize];
	textureFile = new unsigned char[fileSize];
	// Read the file in to the buffer
	fread(textureFile, fileSize, 1, file);
	fclose(file);



	switch (FileType)
	{
	case UW_GRAPHICS_GR:	//.gr
	case UW_GRAPHICS_TR:
	case UW_GRAPHICS_CR:
	case UW_GRAPHICS_SR:
	case UW_GRAPHICS_AR:
		fprintf(LOGFILE,"File Type (should be %d):%d\n", FileType, textureFile[0]);
		fprintf(LOGFILE,"No of textures:%d\n", textureFile[2] << 8 | textureFile[1]);
		if (ImageCount == -1)	//All the images.
		{
			NoOfTextures = textureFile[2] << 8 | textureFile[1];
		}
		else
		{
			NoOfTextures = ImageCount;
		}
		//NoOfTextures=1;
		for (i = 0; i < NoOfTextures; i++)
		{
			long textureOffset = getValAtAddress(textureFile, (i * 4) + 3, 32);
			int BitMapWidth = 83;  //getValAtAddress(textureFile, textureOffset + 1, 8);
			int BitMapHeight = 114; // getValAtAddress(textureFile, textureOffset + 2, 8);
			if (game == UW2)
				{
				BitMapWidth=79;
				BitMapHeight = 112;
				}
//			int datalen;
//			palette auxpal[16];
//			int auxPalIndex;
//			unsigned char *imgNibbles;
//			unsigned char *outputImg;
			//switch (getValAtAddress(textureFile, textureOffset, 8))
			//{
			//default://8 bit uncompressed
				fprintf(LOGFILE,"8 bit uncompressed\n");
				fprintf(LOGFILE,"Width = %d\n", getValAtAddress(textureFile, textureOffset + 1, 8));
				fprintf(LOGFILE,"Height = %d\n", getValAtAddress(textureFile, textureOffset + 2, 8));
				//textureOffset = 1;//textureOffset + 1;
				if (useTGA == 1)
					{
					printf("Here!");
					writeTGA(textureFile, textureOffset, BitMapWidth, BitMapHeight, i, pal, OutFileName,0);
					}
				else
					{
					writeBMP(textureFile, textureOffset, BitMapWidth, BitMapHeight, i, pal, OutFileName);
					}
				
				//break;
			//}
		}
		break;
	}


	//NoOfTextures=0;
	//fprintf(LOGFILE,"Address of first block:%d\n",  (textureFile[7]<<16 | textureFile[6]<<32 | textureFile[5]<<8 | textureFile[4]));


	return;
}

void extractCrittersUW1(char fileAssoc[255], char fileCrit[255], char PaletteFile[255], int PaletteNo, int BitmapSize, int FileType, int game, int CritterNo, char OutFileName[255], int useTGA, int SkipFileOutput, int ItemId, int fileXX, int fileYY)
	{
	int slotIndices[128];
	int slotBase = 0;//I offset the animations slots by this number!!!! 
	palette *pal;
	int AddressPointer;
	unsigned char *critterFile;
	unsigned char auxpalval[32];
	palette auxpal[32];
	int auxPalNo = PaletteNo;

	long fileSize;
	unsigned char *assocFile;
	
	pal = new palette[256];

	if (GREYSCALE == 1)
		{
		getPaletteIndex(PaletteFile, pal, PaletteNo);
		}
	else
		{
		getPalette(PaletteFile, pal, PaletteNo);
		}
	//getPalette(PaletteFile, pal, 0);//always palette 0?

	FILE *file = NULL;      // File pointer
	if ((file = fopen(fileAssoc, "rb")) == NULL)
		{
		fprintf(LOGFILE, "//\nArchive %s not found!\n", fileAssoc);
		return;
		}
	fileSize = getFileSize(file);
	assocFile = new unsigned char[fileSize];
	fread(assocFile, fileSize, 1, file);
	fclose(file);

	if ((file = fopen(fileCrit, "rb")) == NULL)
		{
		fprintf(LOGFILE, "//\nArchive %s not found!\n", fileCrit);
		return;
		}
	fileSize = getFileSize(file);
	critterFile = new unsigned char[fileSize];
	fread(critterFile, fileSize, 1, file);
	fclose(file);

	AddressPointer = 0;
	fprintf(LOGFILE, "\n//\tFile:%s, Palette = %d", fileCrit, PaletteNo);
	slotBase = getValAtAddress(critterFile, AddressPointer, 8);
	//fprintf(LOGFILE, "\n//\tSlot base %d\n", slotBase);
	int NoOfSlots = getValAtAddress(critterFile, AddressPointer + 1, 8);
	//fprintf(LOGFILE, "\t//No of Slots %d\n", NoOfSlots);
	AddressPointer = 2;
	int k = 0;
	for (int i = 0; i < NoOfSlots; i++)
		{
		if (getValAtAddress(critterFile, AddressPointer, 8) != 255)
			{
			slotIndices[k++] = i;//getValAtAddress(critterFile, AddressPointer, 8);
			//fprintf(LOGFILE, "\n//\tIndex %d = %d", i, getValAtAddress(critterFile, AddressPointer, 8));
			}

		AddressPointer++;
		}
	int NoOfSegs = getValAtAddress(critterFile, AddressPointer, 8);
	//fprintf(LOGFILE, "\n//\tNo of anim segments=%d", NoOfSegs);
	AddressPointer++;
	for (int i = 0; i < NoOfSegs; i++)
		{
		fprintf(LOGFILE, "\n\tCreateAnimationUW(\"%d_", ItemId, slotIndices[i] + slotBase);
		PrintAnimName(game, slotIndices[i] + slotBase);
		fprintf(LOGFILE, "\",");
		int ValidCount=0;
		for (int j = 0; j < 8; j++)
			{
			if (getValAtAddress(critterFile, AddressPointer, 8) != 255)
				{
				//fprintf(LOGFILE, "\n\tAnim Frame %d is %d %s_%04d", j, getValAtAddress(critterFile, AddressPointer, 8), fileCrit, getValAtAddress(critterFile, AddressPointer, 8));
				//printf(" \"CR%02oPAGE_N%02d_%d_%04d\" ,", fileXX, fileYY, auxPalNo, getValAtAddress(critterFile, AddressPointer, 8));
				fprintf(LOGFILE, " \"CR%02oPAGE_N%02d_%d_%04d\" ,", fileXX, fileYY, auxPalNo, getValAtAddress(critterFile, AddressPointer, 8));
				ValidCount++;
				}
			else
				{
				fprintf(LOGFILE," \"\" ,");
				}
			AddressPointer++;
			}
		fprintf(LOGFILE, "%d, _RES + \"/Sprites/Critters\" , 0.2f);",ValidCount);
		}
	int NoOfPals = getValAtAddress(critterFile, AddressPointer, 8);
	AddressPointer++;
	//fprintf(LOGFILE, "\n//\tNo of Palettes %d", NoOfPals);

	for (int i = 0; i < 32; i++)
		{
		int value = getValAtAddress(critterFile, (AddressPointer)+(auxPalNo * 32) + i, 8);
		auxpalval[i] = value;
		auxpal[i].green = pal[value].green;
		auxpal[i].blue = pal[value].blue;
		auxpal[i].red = pal[value].red;
		auxpal[i].reserved = pal[value].reserved;
		}

	AddressPointer = AddressPointer + NoOfPals * 32;
	int NoOfFrames = getValAtAddress(critterFile, AddressPointer, 8);
	//fprintf(LOGFILE,"\nNo of Frames Offsets %d", NoOfFrames);
	//fprintf(LOGFILE,"\nCompression Type %d", getValAtAddress(critterFile, AddressPointer+1, 8));
	AddressPointer = AddressPointer + 2;
	int AddressPointerStart = AddressPointer;
int MaxWidth=0;
int MaxHeight=0;

int MaxHotSpotX=0;
int MaxHotSpotY=0;
	for (int pass = 0; pass <= 1; pass++)
		{
		AddressPointer=AddressPointerStart;
		if (pass == 0)
			{//get the max width and height
			for (int i = 0; i < NoOfFrames; i++)
				{
				int frameOffset = getValAtAddress(critterFile, AddressPointer + (i * 2), 16);
				//fprintf(LOGFILE,"\n%d @ %d", i, frameOffset);
				int BitMapWidth = getValAtAddress(critterFile, frameOffset + 0, 8);
				int BitMapHeight = getValAtAddress(critterFile, frameOffset + 1, 8);
				int hotspotx = getValAtAddress(critterFile, frameOffset + 2, 8);
				int hotspoty = getValAtAddress(critterFile, frameOffset + 3, 8);
				if (hotspotx>BitMapWidth) 
					{
					hotspotx = BitMapWidth;
					}
				if (hotspoty>BitMapHeight)
					{
					hotspoty = BitMapHeight;
					}

				if (BitMapWidth > MaxWidth)
					{
					MaxWidth = BitMapWidth;
					}
				if (BitMapHeight > MaxHeight)
					{
					MaxHeight = BitMapHeight;
					}

				if (hotspotx > MaxHotSpotX)
					{
					MaxHotSpotX = hotspotx;
					}
				if (hotspoty > MaxHotSpotY)
					{
					MaxHotSpotY = hotspoty;
					}
				}
			}
		else
			{//Extract
			if (MaxHotSpotX * 2 > MaxWidth)
				{//Try and center the hot spot in the image.
				MaxWidth = MaxHotSpotX * 2;
				}
			unsigned char *outputImg;
			outputImg = new unsigned char[MaxWidth*MaxHeight*2];
			for (int i = 0; i < NoOfFrames; i++)
				{
				int frameOffset = getValAtAddress(critterFile, AddressPointer + (i * 2), 16);
				//fprintf(LOGFILE,"\n%d @ %d", i, frameOffset);
				int BitMapWidth = getValAtAddress(critterFile, frameOffset + 0, 8);
				int BitMapHeight = getValAtAddress(critterFile, frameOffset + 1, 8);
				int hotspotx = getValAtAddress(critterFile, frameOffset + 2, 8);
				int hotspoty = getValAtAddress(critterFile, frameOffset + 3, 8);
				int compression = getValAtAddress(critterFile, frameOffset + 4, 8);
				int datalen = getValAtAddress(critterFile, frameOffset + 5, 16);
				//unsigned char *imgNibbles;
				
				//Adjust the hotspots from the biggest point back to the image corners
				int cornerX; int cornerY;
				cornerX= MaxHotSpotX-hotspotx;
				cornerY = MaxHotSpotY - hotspoty;
				if (cornerX<=0)
					{
					cornerX = 0;
					}
				else
					{
					cornerX = cornerX - 1;
					}
				if (cornerY<=0)
					{
					cornerY = 0;
					}

				//if (((MaxWidth > BitMapWidth) || (MaxHeight > BitMapWidth)))
				if (1)///Experimental code for keeping animation frames pinned to one spot.
					{//Merge the image into a new big image at the hotspot coordinates.;
					unsigned char *srcImg;
					//fprintf(LOGFILE, "%s = Hotspot (%d,%d)\n", critterFile,hotspotx,hotspoty);
//Offset the hotspot from the top left corner
//hotspoty=MaxHeight-hotspoty;

					srcImg = new unsigned char[BitMapWidth*BitMapHeight*2];
					ua_image_decode_rle(critterFile,srcImg, compression == 6 ? 5 : 4, datalen, BitMapWidth*BitMapHeight, frameOffset + 7, auxpalval);
					cornerY = MaxHeight-cornerY;//y is from the top left corner
					//printf("%d, %d\n",cornerX, cornerY);
					//srcImg[hotspotx + hotspoty*BitMapWidth]=200;
					
					int ColCounter = 0; int RowCounter = 0;
					bool ImgStarted = false;
					for (int y = 0; y < MaxHeight; y++)
						{
						for (int x = 0; x < MaxWidth; x++)
							{
							if ((cornerX + ColCounter == x) && (MaxHeight-cornerY + RowCounter == y) && (ColCounter<BitMapWidth) && (RowCounter<BitMapHeight))
								{//the pixel from the source image is here 
								ImgStarted=true;
								outputImg[x + (y*MaxWidth)] = srcImg[ColCounter+(RowCounter*BitMapWidth)];
								//outputImg[x + (y*MaxWidth)] = 0;
								ColCounter++;
								}
							else
								{
								outputImg[x + (y*MaxWidth)]=0;//alpha
								}
							}
						if (ImgStarted == true)
							{//New Row on the src image
							RowCounter++;
							ColCounter=0;
							}
						}
					//Set the heights for output
					BitMapWidth=MaxWidth;
					BitMapHeight = MaxHeight;
					}
				else
					{//No need to resize. Just output the image.
					outputImg = new unsigned char[BitMapWidth*BitMapHeight*2];
					ua_image_decode_rle(critterFile, outputImg, compression == 6 ? 5 : 4, datalen, BitMapWidth*BitMapHeight, frameOffset + 7, auxpalval);
					outputImg[hotspotx + hotspoty*BitMapWidth] = 200;
					}

				if (SkipFileOutput == 1)
					{
					if (useTGA == 1)
						{
						writeTGA(outputImg, 0, BitMapWidth, BitMapHeight, i, pal, OutFileName, 1);
						}
					else
						{
						writeBMP(outputImg, 0, BitMapWidth, BitMapHeight, i, pal, OutFileName);
						}
					}
				}
			}
		}
	}

	void extractCrittersUW2(char fileAssoc[255], char fileCrit[255], char PaletteFile[255], int PaletteNo, int BitmapSize, int FileType, int game, int CritterNo, char OutFileName[255], int useTGA, int SkipFileOutput)
		{//TODO:Get the max height and width
		palette *pal;
		unsigned char *critterFile;
		unsigned char auxpalval[32];
		palette auxpal[32];
		int auxPalNo = PaletteNo;
		int AddressPointer;

		pal = new palette[256];
		//getPalette(PaletteFile, pal, 0);//always palette 0?
		if (GREYSCALE == 1)
			{
			getPaletteIndex(PaletteFile, pal, 0);
			}
		else
			{
			getPalette(PaletteFile, pal, 0);
			}

		long fileSize;
		unsigned char *assocFile;

		FILE *file = NULL;      // File pointer
		if ((file = fopen(fileAssoc, "rb")) == NULL)
			{
			fprintf(LOGFILE, "\nArchive %s not found!\n", fileAssoc);
			return;
			}
		fileSize = getFileSize(file);
		assocFile = new unsigned char[fileSize];
		fread(assocFile, fileSize, 1, file);
		fclose(file);

		if ((file = fopen(fileCrit, "rb")) == NULL)
			{
			fprintf(LOGFILE, "\nArchive %s not found!\n", fileCrit);
			return;
			}
		fileSize = getFileSize(file);
		critterFile = new unsigned char[fileSize];
		fread(critterFile, fileSize, 1, file);
		fclose(file);

		//UW2 uses a different method
		//Starting at offset 0x80
		//fprintf(LOGFILE, "\n\t//%s - palette = %d", fileCrit, auxPalNo);
		//auxPalNo=2;
		AddressPointer = auxPalNo * 32;
		int i = 0;
		for (i = 0; i < 32; i++)
			{
			int value = getValAtAddress(critterFile, (AddressPointer), 8);
			auxpalval[i] = value;
			auxpal[i].green = pal[value].green;
			auxpal[i].blue = pal[value].blue;
			auxpal[i].red = pal[value].red;
			auxpal[i].reserved = pal[value].reserved;
			AddressPointer++;
			}
		i=0;
	int MaxWidth = 0;
	int MaxHeight = 0;
	int MaxHotSpotX = 0;
	int MaxHotSpotY = 0;

		for (int pass = 0; pass <= 1;pass++)
			{
			if (pass == 0)
				{//First pass is getting max image sizes
				for (int index = 128; index < 640; index = index + 2)
					{
					int frameOffset = getValAtAddress(critterFile, index, 16);
					if (frameOffset != 0)
						{
						int BitMapWidth = getValAtAddress(critterFile, frameOffset + 0, 8);
						int BitMapHeight = getValAtAddress(critterFile, frameOffset + 1, 8);
						int hotspotx = getValAtAddress(critterFile, frameOffset + 2, 8);
						int hotspoty = getValAtAddress(critterFile, frameOffset + 3, 8);
						if (hotspotx>BitMapWidth)	{hotspotx = BitMapWidth;}
						if (hotspoty>BitMapHeight)	{hotspoty = BitMapHeight;}
						if (BitMapWidth > MaxWidth){MaxWidth = BitMapWidth;}
						if (BitMapHeight > MaxHeight){MaxHeight = BitMapHeight;}
						if (hotspotx > MaxHotSpotX){MaxHotSpotX = hotspotx;}
						if (hotspoty > MaxHotSpotY){MaxHotSpotY = hotspoty;}
						}//End frameoffsetr first pass
					}//End for loop first pass
				}//End first pass
			else
				{//Extract images
				if (MaxHotSpotX * 2 > MaxWidth)
					{//Try and center the hot spot in the image.
					MaxWidth = MaxHotSpotX * 2;
					}
				unsigned char *outputImg;
				outputImg = new unsigned char[MaxWidth*MaxHeight * 2];
				for (int index = 128; index < 640; index = index + 2)
					{
					int frameOffset = getValAtAddress(critterFile, index, 16);
					if (frameOffset != 0)
						{
						int BitMapWidth = getValAtAddress(critterFile, frameOffset + 0, 8);
						int BitMapHeight = getValAtAddress(critterFile, frameOffset + 1, 8);
						int hotspotx = getValAtAddress(critterFile, frameOffset + 2, 8);
						int hotspoty = getValAtAddress(critterFile, frameOffset + 3, 8);
						int compression = getValAtAddress(critterFile, frameOffset + 4, 8);
						int datalen = getValAtAddress(critterFile, frameOffset + 5, 16);
						//Adjust the hotspots from the biggest point back to the image corners
						int cornerX; int cornerY;
						cornerX = MaxHotSpotX - hotspotx;
						cornerY = MaxHotSpotY - hotspoty;
						if (cornerX <= 0){cornerX = 0;}
						else{cornerX = cornerX - 1;}
						if (cornerY <= 0){cornerY = 0;}

						if (1)
							{
							//Merge the image into a new big image at the hotspot coordinates.;
							unsigned char *srcImg;
							//fprintf(LOGFILE, "%s = Hotspot (%d,%d)\n", critterFile,hotspotx,hotspoty);
							//Offset the hotspot from the top left corner
							//hotspoty=MaxHeight-hotspoty;

							srcImg = new unsigned char[BitMapWidth*BitMapHeight * 2];
							ua_image_decode_rle(critterFile, srcImg, compression == 6 ? 5 : 4, datalen, BitMapWidth*BitMapHeight, frameOffset + 7, auxpalval);
							cornerY = MaxHeight - cornerY;//y is from the top left corner
							//printf("%d, %d\n",cornerX, cornerY);
							//srcImg[hotspotx + hotspoty*BitMapWidth]=200;

							int ColCounter = 0; int RowCounter = 0;
							bool ImgStarted = false;
							for (int y = 0; y < MaxHeight; y++)
								{
								for (int x = 0; x < MaxWidth; x++)
									{
									if ((cornerX + ColCounter == x) && (MaxHeight - cornerY + RowCounter == y) && (ColCounter<BitMapWidth) && (RowCounter<BitMapHeight))
										{//the pixel from the source image is here 
										ImgStarted = true;
										outputImg[x + (y*MaxWidth)] = srcImg[ColCounter + (RowCounter*BitMapWidth)];
										//outputImg[x + (y*MaxWidth)] = 0;
										ColCounter++;
										}
									else
										{
										outputImg[x + (y*MaxWidth)] = 0;//alpha
										}
									}
								if (ImgStarted == true)
									{//New Row on the src image
									RowCounter++;
									ColCounter = 0;
									}
								}
							//Set the heights for output
							BitMapWidth = MaxWidth;
							BitMapHeight = MaxHeight;

							//Ouput files
							if (SkipFileOutput == 1)
								{
								if (useTGA == 1)
									{
									writeTGA(outputImg, 0, BitMapWidth, BitMapHeight, i++, pal, OutFileName, 1);
									}
								else
									{
									writeBMP(outputImg, 0, BitMapWidth, BitMapHeight, i++, pal, OutFileName);
									}
								} //skip
							}
						}//end extrac frameoffset
					}//End for loop extract
				}//End extract images
			}


		/*

		for (int index = 128; index < 640; index = index + 2)
			{
			int frameOffset = getValAtAddress(critterFile, index, 16);
			if (frameOffset != 0)
				{
				//fprintf(LOGFILE,"\n%d @ %d", i, frameOffset);
				int BitMapWidth = getValAtAddress(critterFile, frameOffset + 0, 8);
				int BitMapHeight = getValAtAddress(critterFile, frameOffset + 1, 8);
				int hotspotx = getValAtAddress(critterFile, frameOffset + 2, 8);
				int hotspoty = getValAtAddress(critterFile, frameOffset + 3, 8);
				int compression = getValAtAddress(critterFile, frameOffset + 4, 8);
				int datalen = getValAtAddress(critterFile, frameOffset + 5, 16);
				unsigned char *outputImg;
				outputImg = new unsigned char[BitMapWidth*BitMapHeight];
				ua_image_decode_rle(critterFile, outputImg, compression == 6 ? 5 : 4, datalen, BitMapWidth*BitMapHeight, frameOffset + 7, auxpalval);
				if (SkipFileOutput == 1)
					{
					if (useTGA == 1)
						{
						writeTGA(outputImg, 0, BitMapWidth, BitMapHeight, i, pal, OutFileName, 1);
						}
					else
						{
						writeBMP(outputImg, 0, BitMapWidth, BitMapHeight, i, pal, OutFileName);
						}
					}//End skip file
				i++;
				}//Endif Frame offset
			}//End for loop
*/
		}

void ua_image_decode_rle(unsigned char *FileIn, unsigned char *pixels, unsigned int bits, unsigned int datalen, unsigned int maxpix, int addr_ptr,unsigned char *auxpal)
{
	//Code lifted from Underworld adventures.
	// bit extraction variables
	unsigned int bits_avail = 0;
	unsigned int rawbits = 0;
	unsigned int bitmask = ((1 << bits) - 1) << (8 - bits);
	unsigned int nibble;

	// rle decoding vars
	unsigned int pixcount = 0;
	unsigned int stage = 0; // we start in stage 0
	unsigned int count = 0;
	unsigned int record = 0; // we start with record 0=repeat (3=run)
	unsigned int repeatcount = 0;

	while (datalen>0 && pixcount<maxpix)
	{
		// get new bits
		if (bits_avail<bits)
		{
			// not enough bits available
			if (bits_avail>0)
			{
				nibble = ((rawbits & bitmask) >> (8 - bits_avail));
				nibble <<= (bits - bits_avail);
			}
			else
				nibble = 0;

			//rawbits = (unsigned int)fgetc(fd);
			rawbits = (unsigned int)getValAtAddress(FileIn,addr_ptr,8);
			addr_ptr++;
			if ((int)rawbits == EOF)
				return;

			//         fprintf(LOGFILE,"fgetc: %02x\n",rawbits);

			unsigned int shiftval = 8 - (bits - bits_avail);

			nibble |= (rawbits >> shiftval);

			rawbits = (rawbits << (8 - shiftval)) & 0xFF;

			bits_avail = shiftval;
		}
		else
		{
			// we still have enough bits
			nibble = (rawbits & bitmask) >> (8 - bits);
			bits_avail -= bits;
			rawbits <<= bits;
		}

		//      fprintf(LOGFILE,"nibble: %02x\n",nibble);

		// now that we have a nibble
		datalen--;

		switch (stage)
		{
		case 0: // we retrieve a new count
			if (nibble == 0)
				stage++;
			else
			{
				count = nibble;
				stage = 6;
			}
			break;
		case 1:
			count = nibble;
			stage++;
			break;

		case 2:
			count = (count << 4) | nibble;
			if (count == 0)
				stage++;
			else
				stage = 6;
			break;

		case 3:
		case 4:
		case 5:
			count = (count << 4) | nibble;
			stage++;
			break;
		}

		if (stage<6) continue;

		switch (record)
		{
		case 0:
			// repeat record stage 1
			//         fprintf(LOGFILE,"repeat: new count: %x\n",count);

			if (count == 1)
			{
				record = 3; // skip this record; a run follows
				break;
			}

			if (count == 2)
			{
				record = 2; // multiple run records
				break;
			}

			record = 1; // read next nibble; it's the color to repeat
			continue;

		case 1:
			// repeat record stage 2

		{
				  // repeat 'nibble' color 'count' times
				  for (unsigned int n = 0; n<count; n++)
				  {
					  pixels[pixcount++] = auxpal[nibble];
					  if (pixcount >= maxpix)
						  break;
				  }
		}

			//         fprintf(LOGFILE,"repeat: wrote %x times a '%x'\n",count,nibble);

			if (repeatcount == 0)
			{
				record = 3; // next one is a run record
			}
			else
			{
				repeatcount--;
				record = 0; // continue with repeat records
			}
			break;

		case 2:
			// multiple repeat stage

			// 'count' specifies the number of repeat record to appear
			//         fprintf(LOGFILE,"multiple repeat: %u\n",count);
			repeatcount = count - 1;
			record = 0;
			break;

		case 3:
			// run record stage 1
			// copy 'count' nibbles

			//         fprintf(LOGFILE,"run: count: %x\n",count);

			record = 4; // retrieve next nibble
			continue;

		case 4:
			// run record stage 2

			// now we have a nibble to write
			pixels[pixcount++] = auxpal[nibble];

			if (--count == 0)
			{
				//            fprintf(LOGFILE,"run: finished\n");
				record = 0; // next one is a repeat again
			}
			else
				continue;
			break;
		}

		stage = 0;
		// end of while loop
	}
}

bool load_cuts_anim(char filePathIn[255], const char filePathOut[255],int useTGA, bool ErrorHandling, int isAlpha)
	{
useTGA=1;
printf("%s\n", filePathIn);
//This is lifted wholesale from the Underworld Adventures implementation 
	lpfileheader lpheader;
	lp_descriptor lparray[256];
	Uint8 *outbuffer;
	Uint8 *pages;
	int curlpnum = -1; // current large page
	lp_descriptor *curlp; // current large page
	Uint8 *thepage; // current page
 
	palette pal[256];
	FILE *fd = NULL;      // File pointer
	
	fopen_s(&fd, filePathIn, "rb");
	if (fd == NULL)
		{
		fprintf(LOGFILE,"Unable to open file!");
		return 0;
		}
	printf("\nsize is %d", sizeof(lpfileheader));
	// find out file length
	fseek(fd, 0, SEEK_END);
	long filesize = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	fprintf(LOGFILE,"Reading in cutscene header\n");
	// read in anim file header
	fread(&lpheader, sizeof(lpfileheader), 1, fd);
	
	// skip color cycling structures
	fseek(fd, 128, SEEK_CUR);
	fprintf(LOGFILE,"Reading in cutscene palette\n");
	// read in color palette
	for (int i = 0; i<256; i++)
		{
		//Palette order is different here.
		pal[i].blue = fgetc(fd);
		pal[i].green = fgetc(fd);
		pal[i].red = fgetc(fd);
		pal[i].reserved = fgetc(fd);
		//palette[i][2] = fgetc(fd); // red
		//palette[i][1] = fgetc(fd); // green
		//palette[i][0] = fgetc(fd); // blue

		// extra pad byte
		//fgetc(fd);
		}

	// read in large page descriptors
	fprintf(LOGFILE,"Reading in cutscene page descriptors\n");
	fread(lparray, sizeof(lp_descriptor), 256, fd);
	printf("\nlp_d is size %d", sizeof(lp_descriptor));
	// the file pointer now points to the first large page structure

	// load remaining pages into memory
	filesize -= ftell(fd);
	if (filesize < 0){ return 0; }
	pages = new Uint8[filesize];
	fread(pages, filesize, 1, fd);
	fprintf(LOGFILE,"All pages read into memory\n");
	fclose(fd);

	// alloc memory for the outbuffer
	outbuffer = new Uint8[lpheader.width*lpheader.height + 4000];
	for (int framenumber = 0; framenumber < lpheader.nFrames; framenumber++)
		{
		if ((ErrorHandling == true) && (framenumber == 10))
			{//Special case crashes on a particular cutscene. (doors closing on avatar)
			return 0;
			}

		//draw_screen();
		int i = 0;
		for (; i<lpheader.nLps; i++)
		if (lparray[i].baseRecord <= framenumber && lparray[i].baseRecord + lparray[i].nRecords > framenumber)
			break;

		// calculate large page descriptor pointer and large page pointer
		curlp = reinterpret_cast<lp_descriptor*>(pages + 0x10000 * i);
		thepage = reinterpret_cast<Uint8*>(curlp)+sizeof(lp_descriptor)+2 ;
		//printf("\nThe page is at %d", thepage - pages);
		// page length: curlp.nBytes+(curlp.nRecords*2)
		int destframe = framenumber - curlp->baseRecord;

		Uint16 offset = 0;
		Uint16 *pagepointer = (Uint16*)thepage;
		//printf("\nthe pagepointer starts at = %d", thepage - pages);
		for (Uint16 i = 0; i < destframe; i++)
			{
			offset += pagepointer[i];
			//printf("\nPagepointer = %d", pagepointer[i]);
			}
			
		
		Uint8 *ppointer = thepage + curlp->nRecords * 2 + offset;
		//printf("\npPointer is = %d",  ppointer - pages);
		Uint16 *ppointer16 = (Uint16*)(ppointer);
		
		if (ppointer[1])
			ppointer += (4 + (ppointer16[1] + (ppointer16[1] & 1)));
		else
			ppointer += 4;
		printf("\nfinal ppointer = %d", ppointer - pages);
		// extract data to the output buffer
		//   CPlayRunSkipDump(ppointer, outbuffer);
		fprintf(LOGFILE,"Decoding frame %d of %d\n", framenumber, lpheader.nFrames);
		myPlayRunSkipDump(ppointer, outbuffer);
		fprintf(LOGFILE,"Writing frame %d of %d to file\n", framenumber, lpheader.nFrames);
		if (useTGA==1)
			{
			writeTGA(outbuffer, 0, lpheader.width, lpheader.height, framenumber,pal,filePathOut,isAlpha);//No alpha on cutscenes.
			}
		else
			{
			writeBMP(outbuffer, 0, lpheader.width, lpheader.height, framenumber,pal,filePathOut);
			}
		}

	fprintf(LOGFILE,"Cutscene decoded\n");
	return true;
	}

void myPlayRunSkipDump(Uint8 *srcP, Uint8 *dstP)
	{//From an implemtation by Underworld Adventures (hacking tools)
	int loopcount = 0;
	while (true)
		{
		loopcount++;
		if (loopcount == 15)
			{
			printf("");
			}
		Sint8 cnt = (Sint8)*srcP++;

		if (cnt>0)
			{
			// dump
			fprintf(LOGFILE, "dump\n");
			while (cnt>0)
				{
				*dstP++ = *srcP++;
				cnt--;
				}
			}
		else if (cnt == 0)
			{
			// run
			fprintf(LOGFILE, "run\n");
			Uint8 wordCnt = *srcP++;
			Uint8 pixel = *srcP++;
			while (wordCnt>0)
				{
				*dstP++ = pixel;
				wordCnt--;
				}
			}
		else
			{
			cnt &= 0x7f; // cnt -= 0x80;
			if (cnt != 0)
				{
				// shortSkip
				fprintf(LOGFILE, "shortskip\n");
				dstP += cnt;
				}
			else
				{
				// longOp
				fprintf(LOGFILE, "longop\n");
				Uint16 wordCnt = *((Uint16*)srcP);
				srcP += 2;
				
				if ((Sint16)wordCnt <= 0)
					{
					// notLongSkip
					fprintf(LOGFILE, "notlongskip\n");
					if (wordCnt == 0)
						{
						break; // end loop
						}

					wordCnt &= 0x7fff; // wordCnt -= 0x8000; // Remove sign bit.
					if (wordCnt >= 0x4000)
						{
						// longRun
						fprintf(LOGFILE, "longrun\n");
						wordCnt -= 0x4000; // Clear "longRun" bit
						Uint8 pixel = *srcP++;
						while (wordCnt>0)
							{
							*dstP++ = pixel;
							wordCnt--;
							}
						//                  dstP += wordCnt;
						}
					else
						{
						// longDump
						while (wordCnt>0)
							{
							fprintf(LOGFILE, "longdump\n");
							*dstP++ = *srcP++;
							wordCnt--;
							}

						//                  dstP += wordCnt;
						//                  srcP += wordCnt;
						}
					}
				else
					{
					// longSkip
					fprintf(LOGFILE, "longskip\n");
					dstP += wordCnt;
					}
				}
			}
		}
	}

void writeTGA(unsigned char *bits, long Start, long SizeH, long SizeV, int index, palette *pal, const char OutFileName[255], int Alpha)
	{
	if (ACTUALLY_EXTRACT_FILES == 0)
		{
		return;
		}
	//return;
	FILE *fptr;
	char outFile[255];

	sprintf_s(outFile, 255, "%s_%04d.tga", OutFileName, index);
	//stbi_write_tga(outFile, SizeH, SizeH, Alpha, bits + Start);
	//return;
	/* Write the result as a uncompressed TGA */
	if ((fptr = fopen(outFile, "wb")) == NULL) {
		fprintf(stderr, "Failed to open outputfile\n");
		exit(-1);
		}
	putc(0, fptr);
	putc(0, fptr);
	putc(2, fptr);                         /* uncompressed RGB */
	putc(0, fptr); putc(0, fptr);
	putc(0, fptr); putc(0, fptr);
	putc(0, fptr);
	putc(0, fptr); putc(0, fptr);           /* X origin */
	putc(0, fptr); putc(0, fptr);           /* y origin */
	putc((SizeH & 0x00FF), fptr);
	putc((SizeH & 0xFF00) / 256, fptr);
	putc((SizeV & 0x00FF), fptr);
	putc((SizeV & 0xFF00) / 256, fptr);
	putc(32, fptr);                        /* 32 bit bitmap */
	putc(8, fptr);

	for (int iRow = SizeV - 1; iRow >= 0; iRow--)
		{
		for (int j = (iRow *SizeH); j <(iRow*SizeH) + SizeH; j++)
			{
			int pixel = getValAtAddress(bits, Start + j, 8);
			putc(pal[pixel].blue, fptr);
			putc(pal[pixel].green, fptr);
			putc(pal[pixel].red, fptr);

			if (Alpha == 1)
				{
				if (pixel != 0)	//Alpha
					{
					fputc(255, fptr);
					}
				else
					{
					fputc(0, fptr);
					}
				}
			else
				{
				fputc(255, fptr);//No alpha
				}
			}
		}
	fclose(fptr);
	}

void ExtractShockGraphics(char GraphicsFile[255], char PaletteFile[255], int PaletteChunk,  char OutFileName[255], int useTGA)
{
	palette *pal;
	pal = new palette[256];
	
	/*Palette cycled palettes*/
	palette *pal1;
	palette *pal2;
	palette *pal3;
	palette *pal4;
	unsigned char *art_ark;
	unsigned char *tmp_ark;
	int bCyclePalettes=0;
	int isCutscene=0;
	FILE *file = NULL;      // File pointer
	if ((file = fopen(GraphicsFile, "rb")) == NULL)
		{
		fprintf(LOGFILE,"\nGraphics file not found!\n");
		return;
		}
	long fileSize = getFileSize(file);
	tmp_ark = new unsigned char[fileSize];
	fread(tmp_ark, fileSize, 1,file);
	fclose(file);	
//for (int p=0; p<20;p++)
//{
//if (LoadShockPal(pal,PaletteFile,PaletteChunk)==1)
//	{
	long DirectoryAddress=getValAtAddress(tmp_ark,124,32);//Get the list of chunks in this archive.
	int NoOfChunks = getValAtAddress(tmp_ark,DirectoryAddress,16);
	long address_pointer=DirectoryAddress+6;
	for (int k=0; k< NoOfChunks; k++)
		{
		int chunkId = getValAtAddress(tmp_ark,address_pointer,16);
		short chunkContentType = getValAtAddress(tmp_ark,address_pointer+9,8);
		address_pointer=address_pointer+10;	//next chunk
		switch (chunkId)
			{//ss_xtract. However the fuck he got them!
			case 0x001e:
				isCutscene = 1;
				PaletteChunk = 0;
				break;
			case 0x001f:
			case 0x0020:
				isCutscene = 1;
				PaletteChunk = 1;
				break;
			case 0x01a9:
				isCutscene=1;
				PaletteChunk=2;
				break;
			case 0x01aa:
				isCutscene = 1;
				PaletteChunk=3;
				break;
			case 0x01ab:
				isCutscene = 1;
				PaletteChunk=4;
				break;
			case 0x01ac:
				isCutscene = 1;
				PaletteChunk=5;
				break;
			case 0x01ad:
			case 0x01ae:
				isCutscene = 1;
				PaletteChunk=6;
				break;
			case 0x01af:
			case 0x01b0:
				isCutscene = 1;
				PaletteChunk=7;
				break;
			case 0x01b1:
			case 0x01b2:
			case 0x01b3:
				isCutscene = 1;
				PaletteChunk=8;
				break;
			case 0x01b4:
				isCutscene = 1;
				PaletteChunk=9;
				break;	
			case 0x01b5:	
				isCutscene = 1;
				PaletteChunk=10;
				break;
			case 0x01b7:	
			case 0x01b8:		
				isCutscene = 1;
				PaletteChunk=11;
				break;
			case 0x01b9:
				isCutscene = 1;
				PaletteChunk=12;
				break;	
			case 0x01ba:
				isCutscene = 1;
				PaletteChunk=13;
				break;	
			case 0x01bb:		
				isCutscene = 1;
				PaletteChunk=14;
				break;	
			default:
				break;
			}
		if ((chunkId >= 1000) || (chunkId <= 1272))
			{//Textures.
			bCyclePalettes=0;//1
			}
		if (LoadShockPal(pal,PaletteFile,PaletteChunk)==1)
			{
			if (bCyclePalettes == 1)
				{
				pal1 = new palette[256];
				pal2 = new palette[256];
				pal3 = new palette[256];
				pal4 = new palette[256];
				copyPalette(pal, pal1);
				copyPalette(pal, pal2);
				copyPalette(pal, pal3);
				copyPalette(pal, pal4);
				for (int p=16; p<40;p=p+4)
					{
					cyclePalette(pal1, p, 4);
					cyclePalette(pal2, p, 4);
					cyclePalette(pal2, p, 4);
					cyclePalette(pal3, p, 4);
					cyclePalette(pal3, p, 4);
					cyclePalette(pal3, p, 4);
					cyclePalette(pal4, p, 4);
					cyclePalette(pal4, p, 4);
					cyclePalette(pal4, p, 4);
					cyclePalette(pal4, p, 4);
					}
				bCyclePalettes=2;//only need to cycle once
				}

		
			//if (((chunkContentType == 2) || (chunkContentType == 17)) && (chunkId >= 1000) && (chunkId <= 1272))	//Bitmap and sometimes audio log
			if ((chunkContentType == 2) || (chunkContentType == 17)) 	//Bitmap and sometimes audio log
			//if ((chunkContentType == 3)) 	//Bitmap and sometimes audio log
			{//load this chunk and extract
				char NewOutFileName[255];
				sprintf_s(NewOutFileName, 255, "%s_%04d", OutFileName, chunkId);
				long chunkUnpackedLength;
				long chunkType;//compression type
				long chunkPackedLength;
				int blockAddress =getShockBlockAddress(chunkId,tmp_ark,&chunkPackedLength,&chunkUnpackedLength,&chunkType); 
				if (blockAddress != -1)
					{
					fprintf(LOGFILE,"\nChunk %d, type %d", chunkId, chunkType);
					art_ark=new unsigned char[chunkUnpackedLength];
					LoadShockChunk(blockAddress, chunkType, tmp_ark, art_ark,chunkPackedLength,chunkUnpackedLength);
					if (chunkContentType == 3)
						{//Font data
						long fontAddressPointer=0;
						int isColour=0;
						printf("\nChunk No : %d, BlockAddress : %d", chunkId,blockAddress,fontAddressPointer);

						printf("\nColor Flag : %d (@ %d)", getValAtAddress(art_ark, fontAddressPointer, 16), fontAddressPointer);
						if (getValAtAddress(art_ark, fontAddressPointer, 16) != 0)
							{
							isColour =1;
							}
						fontAddressPointer = fontAddressPointer +2 + 34;
						printf("\nFirst ASCII Char : %d (@ %d)", getValAtAddress(art_ark, fontAddressPointer, 16), fontAddressPointer);
						fontAddressPointer = fontAddressPointer + 2;
						printf("\nSecond ASCII Char : %d (@ %d)", getValAtAddress(art_ark, fontAddressPointer, 16), fontAddressPointer);
						fontAddressPointer = fontAddressPointer+2+32;
						printf("\nPosition Table Offset : %d (@ %d)", getValAtAddress(art_ark, fontAddressPointer, 32), fontAddressPointer);
						fontAddressPointer = fontAddressPointer+4;
						printf("\nBitmap Data Offset : %d (@ %d)", getValAtAddress(art_ark, fontAddressPointer, 32), fontAddressPointer);
						long textureOffset = getValAtAddress(art_ark, fontAddressPointer, 32);
						fontAddressPointer = fontAddressPointer+4;
						printf("\nBitmap Width : %d (@ %d)", getValAtAddress(art_ark, fontAddressPointer, 16), fontAddressPointer);
						int width = getValAtAddress(art_ark, fontAddressPointer, 16);
						fontAddressPointer = fontAddressPointer+2;
						printf("\nBitmap Height : %d (@ %d)", getValAtAddress(art_ark, fontAddressPointer, 16), fontAddressPointer);
						int height = getValAtAddress(art_ark, fontAddressPointer, 16);
						WriteShockFont(art_ark, pal, 0, textureOffset, NewOutFileName, useTGA, width,height,isColour);
						}
					else
						{
						//Read in my chunk header
						int NoOfTextures = getValAtAddress(art_ark, 0, 16);

						//fprintf(LOGFILE,"No of texture subblocks %d\n",NoOfTextures);
						//fprintf(LOGFILE,"Offset to first subblock %d\n",getValAtAddress(art_ark,2,32));

						for (int i = 0; i<NoOfTextures; i++)
							{

							long textureOffset = getValAtAddress(art_ark, 2 + (i * 4), 32);
							//if ((bCyclePalettes == 1) || (bCyclePalettes == 2))
							//	{
							//	//char Pal0OutFileName[255];
							//	char Pal1OutFileName[255];
							//	char Pal2OutFileName[255];
							//	char Pal3OutFileName[255];
							//	char Pal4OutFileName[255];
							//	//sprintf_s(Pal0OutFileName, 255, "%s_%04d_%02d", OutFileName, chunkId, 0);
							//	sprintf_s(Pal1OutFileName, 255, "%s_%04d_PC%02d", OutFileName, chunkId, 1);
							//	sprintf_s(Pal2OutFileName, 255, "%s_%04d_PC%02d", OutFileName, chunkId, 2);
							//	sprintf_s(Pal3OutFileName, 255, "%s_%04d_PC%02d", OutFileName, chunkId, 3);
							//	sprintf_s(Pal4OutFileName, 255, "%s_%04d_PC%02d", OutFileName, chunkId, 4);
							//	WriteShockBitmaps(art_ark, pal, i, textureOffset, NewOutFileName, useTGA, isCutscene);
							//	WriteShockBitmaps(art_ark, pal1, i, textureOffset, Pal1OutFileName, useTGA, isCutscene);
							//	WriteShockBitmaps(art_ark, pal2, i, textureOffset, Pal2OutFileName, useTGA, isCutscene);
							//	WriteShockBitmaps(art_ark, pal3, i, textureOffset, Pal3OutFileName, useTGA, isCutscene);
							//	WriteShockBitmaps(art_ark, pal4, i, textureOffset, Pal4OutFileName, useTGA, isCutscene);
							//	}
							//else
							//	{
							WriteShockBitmaps(art_ark, pal, i, textureOffset, NewOutFileName, useTGA, isCutscene);
							//	}

							}
						}
					
					}
				else
					{
					fprintf(LOGFILE,"Graphics chunk %d not found in %s\n", chunkId,GraphicsFile);
					}
			}
		}	
	}
//}//endof
}

int LoadShockPal(palette *pal, char PaletteFile[255], int PaletteNo)
	{
	//Loads a system shock palette.
	unsigned char *pal_ark;
	unsigned char *tmp_ark;
	int chunkId=-1;
	long chunkUnpackedLength;
	long chunkType;//compression type
	long chunkPackedLength;
	FILE *file = NULL;      // File pointer
	if ((file = fopen(PaletteFile, "rb")) == NULL)
		{
		fprintf(LOGFILE,"\nPalette file not found!\n");
		return -1;
		}
	long fileSize = getFileSize(file);
	tmp_ark = new unsigned char[fileSize];
	fread(tmp_ark, fileSize, 1,file);
	fclose(file);	
	int MatchesFound=-1;
	long DirectoryAddress=getValAtAddress(tmp_ark,124,32);//Get the list of chunks in this archive.
	int NoOfChunks = getValAtAddress(tmp_ark,DirectoryAddress,16);
	long address_pointer=DirectoryAddress+6;
	for (int k=0; k< NoOfChunks; k++)
		{
		
		short chunkContentType = getValAtAddress(tmp_ark,address_pointer+9,8);
		
		
		if ((chunkContentType==0))	//Gamepal
			{
			MatchesFound++;
			chunkId = getValAtAddress(tmp_ark,address_pointer,16);//If I can find a match I'll just use the last one I found.
			if (MatchesFound==PaletteNo)
				{
				k=NoOfChunks+1;//break my loop.
				}
			}
		address_pointer=address_pointer+10;	//next chunk
		}
	
	if (chunkId==-1)
		{
		return -1;
		}
	
	
	int blockAddress =getShockBlockAddress(chunkId,tmp_ark,&chunkPackedLength,&chunkUnpackedLength,&chunkType); 
	if (blockAddress != -1)
		{
		pal_ark=new unsigned char[chunkUnpackedLength];
		LoadShockChunk(blockAddress, chunkType, tmp_ark, pal_ark,chunkPackedLength,chunkUnpackedLength);
		int i=0;
		int palAddr=0;
		for (int j = 0; j < 256; j++) {
			pal[i].red = pal_ark[palAddr + 0];//<<2;
			pal[i].green = pal_ark[palAddr + 1];// << 2;
			pal[i].blue = pal_ark[palAddr + 2];// << 2;
			pal[i].reserved = 0;
			palAddr = palAddr +3;
			i++;
			}
			return 1;
		}
	else
		{
		fprintf(LOGFILE,"Palette %d not found in %c", PaletteNo,PaletteFile);
		return-1;
		}
	return -1;
	}

// This one is also almost directly from Jim Cameron's code.
void UncompressBitmap(unsigned char *chunk_bits, unsigned char *bits, int numbits) {
	int j = 0;
	int i,xc;
	unsigned char *bits_end;

	bits_end = bits + numbits;

	memset(bits,0,numbits);

	while (bits < bits_end)
	{
		xc = *chunk_bits++;
		if (xc == 0)
		{
			xc = *chunk_bits++;
			for (i = 0; i < xc && bits < bits_end; ++i)
			{
				*bits++ = *chunk_bits;
			}
			++chunk_bits;
		}
		else if (xc < 0x81)
		{
			if (xc == 0x80)
			{
				xc = *chunk_bits++;
				if (xc == 0)
				{
					break;
				}
				if (*chunk_bits < 0x80)
				{
					bits += xc + (*chunk_bits << 8);
					xc = 0;
				}
				/*  	  xc = *chunk_bits++; */
				++chunk_bits;
			}
			for (i = 0; i < xc && bits < bits_end; ++i)
			{
				*bits++ = *chunk_bits++;
			}
		}
		else
		{
			bits += (xc & 0x7f);
		}
	}

}

void WriteShockBitmaps(unsigned char *art_ark, palette *pal,int index, int textureOffset, char OutFileName[255], int useTGA,int isCutscene)
{
//Process a system shock bitmap chunk
int BitMapHeaderSize=28;
int CompressionType;
int Width;
int Height;

		//First 28 bytes are header info.
		//fprintf(LOGFILE,"\nAlways 0=%d",getValAtAddress(art_ark,textureOffset+0,32));
		CompressionType=getValAtAddress(art_ark,textureOffset+4,16);
		//fprintf(LOGFILE,"\nType=%d",CompressionType);
		Width=getValAtAddress(art_ark,textureOffset+8,16);
		//fprintf(LOGFILE,"\nWidth=%d",Width);
		Height=getValAtAddress(art_ark,textureOffset+10,16);
		//Height=150;//?cutscenes???
		if (isCutscene == 1)
			{
			Width=320;
			Height=150;
			}
	fprintf(LOGFILE, "\n%s_%04d", OutFileName,index);
	fprintf(LOGFILE,"\nBitmap header\n");
	fprintf(LOGFILE,"Always %d = %d\n",0, getValAtAddress(art_ark,textureOffset+0,32));
	fprintf(LOGFILE,"Type %d = %d\n",0x4, getValAtAddress(art_ark,textureOffset+0x4,16));
	fprintf(LOGFILE,"??? %d = %d\n",0x6, getValAtAddress(art_ark,textureOffset+0x6,16));
	fprintf(LOGFILE,"Width %d = %d\n",0x8, getValAtAddress(art_ark,textureOffset+0x8,16));
	fprintf(LOGFILE,"Heigth %d = %d\n",0xA, getValAtAddress(art_ark,textureOffset+0xA,16));
	fprintf(LOGFILE,"Width in bytes %d = %d\n",0xC, getValAtAddress(art_ark,textureOffset+0xC,16));
	fprintf(LOGFILE,"log2width %d = %d\n",0xE, getValAtAddress(art_ark,textureOffset+0xE,8));
	fprintf(LOGFILE,"log2height %d = %d\n",0xF, getValAtAddress(art_ark,textureOffset+0xF,8));
	fprintf(LOGFILE,"%d = %d\n",0x10, getValAtAddress(art_ark,textureOffset+0x10,16));
	fprintf(LOGFILE,"%d = %d\n",0x12, getValAtAddress(art_ark,textureOffset+0x12,16));
	fprintf(LOGFILE,"%d = %d\n",0x14, getValAtAddress(art_ark,textureOffset+0x14,16));
	fprintf(LOGFILE,"%d = %d\n",0x16, getValAtAddress(art_ark,textureOffset+0x16,16));
	fprintf(LOGFILE,"??? %d = %d\n",0x18, getValAtAddress(art_ark,textureOffset+0x18,32));
		
		
		//fprintf(LOGFILE,"\nHeight=%d",Height);		
	if ((Width>0) && (Height >0))
		{
		//fprintf(LOGFILE,"\nAt 6 =%d",getValAtAddress(art_ark,textureOffset+6,16));
		//fprintf(LOGFILE,"\nAt E =%d",getValAtAddress(art_ark,textureOffset+0xE,8));
		//fprintf(LOGFILE,"\nAt F =%d",getValAtAddress(art_ark,textureOffset+0xF,8));
		//fprintf(LOGFILE,"\nAt 18 =%d",getValAtAddress(art_ark,textureOffset+0x18,32));
		if(CompressionType==4)
			{//compressed
			//fprintf(LOGFILE,"Compressed bmp\n");
			unsigned char *outputImg;
			outputImg = new unsigned char[Width*Height];
			UncompressBitmap(art_ark+textureOffset+BitMapHeaderSize, outputImg,Height*Width);
			if (useTGA==1)
				{
				writeTGA(outputImg,0,Width,Height,index,pal,OutFileName,1);
				return;
				}
			else
				{
				writeBMP(outputImg,0,Width,Height,index,pal,OutFileName);
				}
			}
		else
			{
			if (useTGA==1)
				{
				writeTGA(art_ark,textureOffset+BitMapHeaderSize,Width,Height,index,pal,OutFileName,1);
				}
			else
				{
				writeBMP(art_ark,textureOffset+BitMapHeaderSize,Width,Height,index,pal,OutFileName);
				}
			}			
		}
	}

void WriteShockFont(unsigned char *art_ark, palette *pal, int index, int textureOffset, char OutFileName[255], int useTGA, int Width, int Height ,int isColour)
	{
	//Process a system shock bitmap chunk
	int BitMapHeaderSize = 0; //28;
	if ((Width>0) && (Height >0))
		{
		if (useTGA == 1)
			{
			writeTGA(art_ark, textureOffset + BitMapHeaderSize, Width, Height, index, pal, OutFileName, 1);
			}
		else
			{
			//writeBMP(art_ark, textureOffset + BitMapHeaderSize, Width, Height, index, pal, OutFileName);
			if (isColour == 1)
				{
				writeBMP(art_ark, textureOffset + BitMapHeaderSize, Width, Height, index, pal, OutFileName);
				}
			else
				{
				writeBMPBW(art_ark, textureOffset + BitMapHeaderSize, Width, Height, index, pal, OutFileName);
				}
			}
		}
	}

void ExtractShockCutscenes(char GraphicsFile[255], char PaletteFile[255], int PaletteChunk,  char OutFileName[255], int useTGA)
{
PaletteChunk=2;
	palette *pal;
	pal = new palette[256];
	
	unsigned char *art_ark;
	unsigned char *tmp_ark;

	FILE *file = NULL;      // File pointer
	if ((file = fopen(GraphicsFile, "rb")) == NULL)
		{
		fprintf(LOGFILE,"\nGraphics file not found!\n");
		return;
		}
	long fileSize = getFileSize(file);
	tmp_ark = new unsigned char[fileSize];
	fread(tmp_ark, fileSize, 1,file);
	fclose(file);	
//for (int p=0; p<20;p++)
//{

	long DirectoryAddress=getValAtAddress(tmp_ark,124,32);//Get the list of chunks in this archive.
	int NoOfChunks = getValAtAddress(tmp_ark,DirectoryAddress,16);
	long address_pointer=DirectoryAddress+6;
	for (int k=0; k< NoOfChunks; k++)
		{
		int chunkId = getValAtAddress(tmp_ark,address_pointer,16);
		short chunkContentType = getValAtAddress(tmp_ark,address_pointer+9,8);
		address_pointer=address_pointer+10;	//next chunk
		switch (chunkId)
			{//ss_xtract. However the fuck he got them!
			case 0x001E:
				PaletteChunk=0;
					break;
			case 0x001F:
			case 0x0020:
				PaletteChunk = 1;
				break;
			case 0x01a9:
				PaletteChunk=2;
				break;
			case 0x01aa:
				PaletteChunk=3;
				break;
			case 0x01ab:
				PaletteChunk=4;
				break;
			case 0x01ac:
				PaletteChunk=5;
				break;
			case 0x01ad:
			case 0x01ae:
				PaletteChunk=6;
				break;
			case 0x01af:
			case 0x01b0:
				PaletteChunk=7;
				break;
			case 0x01b1:
			case 0x01b2:
			case 0x01b3:
				PaletteChunk=8;
				break;
			case 0x01b4:
				PaletteChunk=9;
				break;	
			case 0x01b5:		
				PaletteChunk=10;
				break;
			case 0x01b7:	
			case 0x01b8:			
				PaletteChunk=11;
				break;
			case 0x01b9:
				PaletteChunk=12;
				break;	
			case 0x01ba:
				PaletteChunk=13;
				break;	
			case 0x01bb:						
				PaletteChunk=14;
				break;	
			default:
				break;
			}
		if (LoadShockPal(pal,PaletteFile,PaletteChunk)==1)
			{
			if ((chunkContentType==2) || (chunkContentType==17))	//Bitmap and sometimes audio log
				{//load this chunk and extract
					char NewOutFileName[255];
					sprintf_s(NewOutFileName, 255, "%s_%04d", OutFileName, chunkId);
					long chunkUnpackedLength;
					long chunkType;//compression type
					long chunkPackedLength;
					int blockAddress =getShockBlockAddress(chunkId,tmp_ark,&chunkPackedLength,&chunkUnpackedLength,&chunkType); 
					if (blockAddress != -1)
						{
						fprintf(LOGFILE,"\nChunk %d, type %d", chunkId, chunkType);
						art_ark=new unsigned char[chunkUnpackedLength];
						LoadShockChunk(blockAddress, chunkType, tmp_ark, art_ark,chunkPackedLength,chunkUnpackedLength);
						
						//Read in my chunk header
						int NoOfTextures=getValAtAddress(art_ark,0,16);
						
						//fprintf(LOGFILE,"No of texture subblocks %d\n",NoOfTextures);
						//fprintf(LOGFILE,"Offset to first subblock %d\n",getValAtAddress(art_ark,2,32));
						unsigned char *keyframe=new unsigned char[320*150];
						for (int i =0; i<NoOfTextures; i++)
							{
							long textureOffset = getValAtAddress(art_ark,2+(i*4),32);
							WriteShockCutsceneBitmaps(keyframe,art_ark,pal,i,textureOffset, NewOutFileName,useTGA);
							}
						}
					else
						{
						fprintf(LOGFILE,"Graphics chunk %d not found in %s\n", chunkId,GraphicsFile);
						}
				}
		}
	}	
//}//endof
}

void WriteShockCutsceneBitmaps(unsigned char KeyFrame[48000], unsigned char *art_ark, palette *pal,int index, int textureOffset, char OutFileName[255], int useTGA)
{
//Process a system shock bitmap chunk
int BitMapHeaderSize=28;
int CompressionType;
int Width;
int Height;

		//First 28 bytes are header info.
		//fprintf(LOGFILE,"\nAlways 0=%d",getValAtAddress(art_ark,textureOffset+0,32));
		CompressionType=getValAtAddress(art_ark,textureOffset+4,16);
		//fprintf(LOGFILE,"\nType=%d",CompressionType);
		Width=getValAtAddress(art_ark,textureOffset+8,16);
		//fprintf(LOGFILE,"\nWidth=%d",Width);
		Height=getValAtAddress(art_ark,textureOffset+10,16);
		Height=150;//?cutscenes???
		
		
		
		//fprintf(LOGFILE,"\nHeight=%d",Height);		
	if ((Width>0) && (Height >0))
		{
		//fprintf(LOGFILE,"\nAt 6 =%d",getValAtAddress(art_ark,textureOffset+6,16));
		//fprintf(LOGFILE,"\nAt E =%d",getValAtAddress(art_ark,textureOffset+0xE,8));
		//fprintf(LOGFILE,"\nAt F =%d",getValAtAddress(art_ark,textureOffset+0xF,8));
		//fprintf(LOGFILE,"\nAt 18 =%d",getValAtAddress(art_ark,textureOffset+0x18,32));
		if(CompressionType==4)
			{//compressed
			//fprintf(LOGFILE,"Compressed bmp\n");
			unsigned char *outputImg;
			outputImg = new unsigned char[Width*Height];
			if (index==0)
				{//Keyframe
				//KeyFrame = new unsigned char[Width*Height];
				UncompressBitmap(art_ark+textureOffset+BitMapHeaderSize, KeyFrame,Height*Width);
				//copy keyframe to outputimg
				for (int z=0;z<Height*Width;z++)
					{
					outputImg[z]=KeyFrame[z];
					}
				}
			else
				{
				UncompressBitmap(art_ark+textureOffset+BitMapHeaderSize, outputImg,Height*Width);
				//ApplyKeyFrame(KeyFrame,outputImg,Height*Width);
				for (int z=0;z<Height*Width;z++)
					{
					if (getValAtAddress(outputImg,z,8)==0)
						{
						outputImg[z]=KeyFrame[z];
						//outputImg[z]=KeyFrame[z]^outputImg[z];
						}
					}
				}
			if (useTGA==1)
				{
				writeTGA(outputImg,0,Width,Height,index,pal,OutFileName,0);//Doesn't appear to work properly?
				}
			else
				{
				writeBMP(outputImg,0,Width,Height,index,pal,OutFileName);
				}
			////copy output img to key frame.
			//	for (int z=0;z<Height*Width;z++)
			//		{
			//		KeyFrame[z]=outputImg[z];
			//		}
			}
		else
			{
			if (useTGA==1)
				{
				writeTGA(art_ark,textureOffset+BitMapHeaderSize,Width,Height,index,pal,OutFileName,1);
				}
			else
				{
				writeBMP(art_ark,textureOffset+BitMapHeaderSize,Width,Height,index,pal,OutFileName);
				}
			}			
		}
	}
//	
//void ApplyKeyFrame(unsigned char *keyframe,unsigned char *output,int BitMapSize)
//{
////Merge animation frames into the key frame.
//for (int i=0;i<BitMapSize;i++)
//	{
//	if (output[i]==0)
//		{
//		output[i]=keyframe[i];
//		}
//	}
//}

void cyclePalette(palette *pal, int Start, int length )
	{
	/*Shifts the palette values around between the start and start+length. Used for texture animations and special effects*/


		unsigned char firstRed = pal[Start].red;
		unsigned char firstGreen = pal[Start].green;
		unsigned char firstBlue = pal[Start].blue;
		for (int i = Start; i < Start + length - 1; i++)
			{
			pal[i].red = pal[i + 1].red;
			pal[i].green = pal[i + 1].green;
			pal[i].blue = pal[i + 1].blue;
			}
		pal[Start + length - 1].red = firstRed;
		pal[Start + length - 1].green = firstGreen;
		pal[Start + length - 1].blue = firstBlue;


	}

void copyPalette(palette *inPal, palette *outPal)
	{
	/*Copies one palette to another.*/
	for (int i = 0; i < 256; i++)
		{
		outPal[i].red = inPal[i].red;
		outPal[i].green = inPal[i].green;
		outPal[i].blue = inPal[i].blue;
		outPal[i].reserved = inPal[i].reserved;
		}
	}

void extractAllCrittersUW1(char fileAssoc[255], char CritPath[255], char PaletteFile[255], int game, int useTGA)
	{
	char OutFileName[255];
	long fileSize;
	unsigned char *assocFile;
	int AssocPtr = 0;
	//Read in the assoc.anim file
	FILE *file = NULL;      // File pointer
	if ((file = fopen(fileAssoc, "rb")) == NULL)
		{
		fprintf(LOGFILE, "\n//ExtractAllCrittersUW1 : Archive not found!\n");
		return;
		}
	fileSize = getFileSize(file);
	assocFile = new unsigned char[fileSize];
	fread(assocFile, fileSize, 1, file);
	fclose(file);
	AssocPtr = 256;
	for (int i = 0; i < 64; i++)
		{
		char fileCrit[255];
		int auxPalNo = getValAtAddress(assocFile, AssocPtr + 1, 8);
		int CritterID = getValAtAddress(assocFile, AssocPtr + 0, 8);

		fprintf(LOGFILE, "\n //Anim ID: %d - which is %s", i, objectMasters[i + 0x40].desc);
		//fprintf(LOGFILE, "//\t Palette is %d", auxPalNo);
		for (int pageNo = 0; pageNo <= 1; pageNo++)
			{
			sprintf_s(fileCrit, 255, "%s\CR%02oPAGE.N%02d", CritPath, CritterID, pageNo);//page 1
			sprintf_s(OutFileName, 255, "CR%02oPAGE_N%02d_%d", CritterID, pageNo, auxPalNo);
			extractCrittersUW1(fileAssoc, fileCrit, PaletteFile, auxPalNo, 64, UW_GRAPHICS_GR, game, 0, OutFileName, useTGA, ACTUALLY_EXTRACT_FILES, i + 0x40, CritterID, pageNo);
			}
		AssocPtr = AssocPtr + 2;
		}
	}


void extractAllCrittersUW2(char fileAssoc[255], char CritPath[255], char PaletteFile[255], int game, int useTGA)
	{
	/*Extracts all UW2 critters and dumps their animation frames
	Animations are first looked up in the as.am file with associates the creatures item id-0x40 with the XX portion of a CRXX.YY file.
	Then a look up of the CR.AN file gives a listing of the animation frames for each anim and angle as detailed in Abysmals uw-formats.txt
	From there a lookup is made to PG.MP for each frame. PG.MP lists the final frame of each YY file in order. So to get YY for a specific animation
	you just get which position in PG.MP the animation frame is in.
	Additionally since my tools extract every file in the crxx.yy without frame numbers files I subtract the value in pg.mp from the frame no to match up.
	*/
	char OutFileName[255];
	//	palette auxpal[32];
	int frameIndices[6];
	int frameFiles[8];
	long fileSize;
	unsigned char *assocFile;
	int AssocPtr = 0;
	int PageNo;
	//Read in the assoc.anim file
	FILE *file = NULL;      // File pointer
	if ((file = fopen(fileAssoc, "rb")) == NULL)
		{
		fprintf(LOGFILE, "\nExtractAllCrittersUW2 : Archive not found!\n");
		return;
		}
	fileSize = getFileSize(file);
	assocFile = new unsigned char[fileSize];
	fread(assocFile, fileSize, 1, file);
	fclose(file);
	int NoOfCritters = fileSize / 2;//Two bytes per critter. Each critter corresponds with an item id in the object list.

	//Load the paging file pg.mp
	char filePGMP[255];
	sprintf_s(filePGMP, 255, "%s\\%s", path_uw2, "Crit\\PG.MP");
	unsigned char *PGMP;
	if ((file = fopen(filePGMP, "rb")) == NULL)
		{
		fprintf(LOGFILE, "\nArchive %s not found!\n", filePGMP);
		return;
		}
	fileSize = getFileSize(file);
	PGMP = new unsigned char[fileSize];
	fread(PGMP, fileSize, 1, file);
	fclose(file);

	//Load the animation frame listings in cr.an
	char fileCRAN[255];
	sprintf_s(fileCRAN, 255, "%s\\%s", path_uw2, "Crit\\CR.AN");
	unsigned char *CRAN;
	if ((file = fopen(fileCRAN, "rb")) == NULL)
		{
		fprintf(LOGFILE, "\nArchive %s not found!\n", fileCRAN);
		return;
		}
	fileSize = getFileSize(file);
	CRAN = new unsigned char[fileSize];
	fread(CRAN, fileSize, 1, file);
	fclose(file);

	for (int i = 0; i < NoOfCritters; i++)
		{
		char fileCrit[255];
		int auxPalNo = getValAtAddress(assocFile, AssocPtr + 1, 8);//Which palette is used in the graphics for this critter. The palettes are stored in the animation file.
		int CritterID = getValAtAddress(assocFile, AssocPtr + 0, 8);//The XX portion of the file name.
		fprintf(LOGFILE, "\n//Anim ID: %d - which is %s", i, objectMasters[i + 0x40].desc);
		AssocPtr = AssocPtr + 2;
		if (CritterID != 255)
			{
			//Extract the graphics in order.
			//Go through the pg.mp file to get the page files
			int ExtractPageNo = 0;
			for (int i = 0; i < 8; i++)
				{
				if (getValAtAddress(PGMP, CritterID * 8 + i, 8) != 255)//Checks if a exists at this index in the page file.
					{
					sprintf_s(fileCrit, 255, "%s\\CR%02o.%02d", CritPath, CritterID, ExtractPageNo);//Create a file name to extract from
					sprintf_s(OutFileName, 255, "CR%02o_%02d_%d", CritterID, ExtractPageNo, auxPalNo);//Get a name to extract to
					extractCrittersUW2(fileAssoc, fileCrit, PaletteFile, auxPalNo, 64, UW_GRAPHICS_GR, game, 0, OutFileName, useTGA, ACTUALLY_EXTRACT_FILES);
					ExtractPageNo++;
					}
				}

			for (int i = 0; i < 8; i++)
				{//Store the page file last frame in .YY file values 
				frameFiles[i] = getValAtAddress(PGMP, i + (8 * CritterID), 8);
				}
			int cranAdd = (CritterID * 512);//Address when the anim info starts
			for (int Animation = 0; Animation<8; Animation++)//The the animation slot
				{
				fprintf(LOGFILE, "//Animation #%d ", Animation);
				bool NoAngle=false;
				bool PrintFrames=true;
				int NoOfValid=0;
				if (PrintAnimName(game, Animation))//Prints out what the animation is.
					{
					NoAngle = true;
					}
				else
					{
					//fprintf(LOGFILE, "_");
					NoAngle = false;					
					}
				for (int Angle = 0; Angle<8; Angle++)//Each animation has every possible angle.
					{
					//fprintf(LOGFILE, "\n\t\tAngle is ", Angle);
							
					if (NoAngle==false)
						{
						fprintf(LOGFILE, "\nCreateAnimationUW(\"%d_",i+64);
						PrintAnimName(game, Animation);
						fprintf(LOGFILE, "_");
						PrintCritAngle(Angle); //Prints out the cardinal direction (front,left,right,rear etc) for the animation
						fprintf(LOGFILE, "\"");
						PrintFrames=true;
						}
					else
						{//only print the front version of it
						if (Angle == 4)
							{
						fprintf(LOGFILE, "\nCreateAnimationUW(\"%d_", i + 64);
							PrintAnimName(game, Animation);
							fprintf(LOGFILE, "\"");
							PrintFrames=true;
							}
						else
							{
							PrintFrames=false;
							}
						}
					int ValidEntries = getValAtAddress(CRAN, cranAdd + (Animation * 64) + (Angle * 8) + (7), 8);//Get how many valid frames are in the animation
					NoOfValid = 0;
					//fprintf(LOGFILE, " Valid is %d", ValidEntries);
					for (int FrameNo = 0; FrameNo < 6; FrameNo++)//Each animation has up to 6 frames. We keep the last valid one for the page lookup
						{
						frameIndices[FrameNo] = 255;//reset the value
						int currFrame = getValAtAddress(CRAN, cranAdd + (Animation * 64) + (Angle * 8) + (FrameNo), 8);
						frameIndices[FrameNo] = currFrame;
						if (FrameNo>ValidEntries)
							{//I don't store any frames after the valid value.
							frameIndices[FrameNo] = 255;
							}
						}

					//Calculate which page the anim is part of
					for (int x = 0; x < 6; x++)
						{
						if (frameIndices[x] != 255)
							{
							PageNo = 0;//default
							for (int i = 0; i < 6; i++)
								{
								if (i == 0)
									{
									if (frameIndices[x] <= frameFiles[0])
										{//Frame is in the CRXX.00 file.
										PageNo = 0;
										}
									}
								else
									{
									if ((frameIndices[x] > frameFiles[i - 1]) && (frameIndices[x] <= frameFiles[i]))//In between 
										{//The frame is in the CRXX.i file.
										PageNo = i;
										}
									}
								}
							sprintf_s(fileCrit, 255, "CR%02o_%02d_%d", CritterID, PageNo, auxPalNo);
							if (PrintFrames == true)
								{
								fprintf(LOGFILE, ",\"%s", fileCrit);
								NoOfValid++;
								if (PageNo == 0)
									{
									fprintf(LOGFILE, "_%04d\"", frameIndices[x]);
									}
								else
									{
									fprintf(LOGFILE, "_%04d\"", frameIndices[x] - frameFiles[PageNo - 1] - 1);//Shift my frame values down to match the extracted graphics files.
									}
								}
							}
						else
							{
							if (PrintFrames == true)
								{
								fprintf(LOGFILE, ",\"\"");
								}
							}
						}
					if (PrintFrames == true)
						{
						fprintf(LOGFILE, ", %d, _RES + \"/Sprites/Critters\" , 0.2f);\n", NoOfValid);						
						}
					}
				}//h
			}
		}
	}

bool PrintAnimName(int game, int animNo)
	{//Returns true if the anim name is the full name (no angle). For UW2.
		switch (game)
			{
			case UWDEMO:
			case UW1:
				{
				switch (animNo)
					{
						case 0x0:fprintf(LOGFILE, "idle_combat"); return true; break;
						case 0x1:fprintf(LOGFILE, "attack_bash"); return true; break;
						case 0x2:fprintf(LOGFILE, "attack_slash"); return true; break;
						case 0x3:fprintf(LOGFILE, "attack_thrust"); return true; break;
						case 0x4:fprintf(LOGFILE, "attack_unk4"); return true; break;
						case 0x5:fprintf(LOGFILE, "attack_secondary"); return true; break;
						case 0x6:fprintf(LOGFILE, "attack_unk6"); return true; break;
						case 0x7:fprintf(LOGFILE, "walking_towards"); return true; break;
						case 0xc:fprintf(LOGFILE, "death"); return true; break;
						case 0xd:fprintf(LOGFILE, "begin_combat"); return true; break;
						case 0x20:fprintf(LOGFILE, "idle_rear"); break;
						case 0x21:fprintf(LOGFILE, "idle_rear_right"); break;
						case 0x22:fprintf(LOGFILE, "idle_right"); break;
						case 0x23:fprintf(LOGFILE, "idle_front_right"); break;
						case 0x24:fprintf(LOGFILE, "idle_front"); break;
						case 0x25:fprintf(LOGFILE, "idle_front_left"); break;
						case 0x26:fprintf(LOGFILE, "idle_left"); break;
						case 0x27:fprintf(LOGFILE, "idle_rear_left"); break;
						case 0x80:fprintf(LOGFILE, "walking_rear"); break;
						case 0x81:fprintf(LOGFILE, "walking_rear_right"); break;
						case 0x82:fprintf(LOGFILE, "walking_right"); break;
						case 0x83:fprintf(LOGFILE, "walking_front_right"); break;
						case 0x84:fprintf(LOGFILE, "walking_front"); break;
						case 0x85:fprintf(LOGFILE, "walking_front_left"); break;
						case 0x86:fprintf(LOGFILE, "walking_left"); break;
						case 0x87:fprintf(LOGFILE, "walking_rear_left"); break;
						default:fprintf(LOGFILE, "unknown_anim"); break;
					}
				break;
				}
			case UW2:
				{
					switch (animNo)
						{
						case 0x0:fprintf(LOGFILE, "idle"); break;
						case 0x1:fprintf(LOGFILE, "walking"); break;
						case 0x2:fprintf(LOGFILE, "attack_bash"); return true; break;
						case 0x3:fprintf(LOGFILE, "attack_slash"); return true; break;
						case 0x4:fprintf(LOGFILE, "attack_thrust"); return true; break;
						case 0x5:fprintf(LOGFILE, "attack_secondary"); return true; break;
						case 0x6:fprintf(LOGFILE, "attack_unk"); return true; break;
						case 0x7:fprintf(LOGFILE, "death"); return true;break;
						case 0xd:fprintf(LOGFILE, "begin_combat"); return true; break;
						case 0x20:fprintf(LOGFILE, "idle"); break;
						case 0x21:fprintf(LOGFILE, "idle"); break;
						case 0x22:fprintf(LOGFILE, "idle"); break;
						case 0x23:fprintf(LOGFILE, "idle"); break;
						case 0x24:fprintf(LOGFILE, "idle"); break;
						case 0x25:fprintf(LOGFILE, "idle"); break;
						case 0x26:fprintf(LOGFILE, "idle"); break;
						case 0x27:fprintf(LOGFILE, "idle"); break;
						case 0x80:fprintf(LOGFILE, "walking"); break;
						case 0x81:fprintf(LOGFILE, "walking"); break;
						case 0x82:fprintf(LOGFILE, "walking"); break;
						case 0x83:fprintf(LOGFILE, "walking"); break;
						case 0x84:fprintf(LOGFILE, "walking"); break;
						case 0x85:fprintf(LOGFILE, "walking"); break;
						case 0x86:fprintf(LOGFILE, "walking"); break;
						case 0x87:fprintf(LOGFILE, "walking"); break;
						default:fprintf(LOGFILE, "unknown_anim"); break;
					break;
					}
				break;
				}
			case SHOCK:
				{
				switch (animNo)
					{
						case 0: fprintf(LOGFILE, "Standing"); break;
						case 1: fprintf(LOGFILE, "Walking"); break;
						case 2: fprintf(LOGFILE, "Combat"); break;
						case 3: fprintf(LOGFILE, "Attack 1"); break;
						case 4: fprintf(LOGFILE, "Attack 2"); break;
						case 5: fprintf(LOGFILE, "Attack 3"); break;
						case 6: fprintf(LOGFILE, "Attack 4"); break;
						case 7: fprintf(LOGFILE, "Dying"); break;
						default: fprintf(LOGFILE, "Unknown"); break;
					}
				break;
				}
			}
	return false;
	}

void PrintCritAngle(int angle)
	{
	switch (angle)
		{
			case 0: fprintf(LOGFILE, "rear"); break;
			case 1: fprintf(LOGFILE, "rear_right"); break;
			case 2: fprintf(LOGFILE, "right"); break;
			case 3: fprintf(LOGFILE, "front_right"); break;
			case 4: fprintf(LOGFILE, "front"); break;
			case 5: fprintf(LOGFILE, "front_left"); break;
			case 6: fprintf(LOGFILE, "left"); break;
			case 7: fprintf(LOGFILE, "rear_left"); break;
		}
	}

void ExtractShockFont(char GraphicsFile[255], char PaletteFile[255], int PaletteChunk, char OutFileName[255], int useTGA)
{

int Alpha=1;
palette *pal;
pal = new palette[256];

unsigned char *art_ark;
unsigned char *tmp_ark;

FILE *file = NULL;      // File pointer
FILE *outf;
if ((file = fopen(GraphicsFile, "rb")) == NULL)
	{
	fprintf(LOGFILE, "\nGraphics file not found!\n");
	return;
	}
long fileSize = getFileSize(file);
tmp_ark = new unsigned char[fileSize];
fread(tmp_ark, fileSize, 1, file);
fclose(file);

if (LoadShockPal(pal, PaletteFile, PaletteChunk) != 1)
	{
	fprintf(LOGFILE, "\nPalette not loaded\n");
	}

long DirectoryAddress = getValAtAddress(tmp_ark, 124, 32);//Get the list of chunks in this archive.
int NoOfChunks = getValAtAddress(tmp_ark, DirectoryAddress, 16);
long address_pointer = DirectoryAddress + 6;

for (int k = 0; k< NoOfChunks; k++)
	{
	int chunkId = getValAtAddress(tmp_ark, address_pointer, 16);
	short chunkContentType = getValAtAddress(tmp_ark, address_pointer + 9, 8);
	address_pointer = address_pointer + 10;	//next chunk
		if ((chunkContentType == 3)) 	//Font
			{//load this chunk and extract
			char NewOutFileName[255];
			sprintf_s(NewOutFileName, 255, "%s_%04d", OutFileName, chunkId);
			long chunkUnpackedLength;
			long chunkType;//compression type
			long chunkPackedLength;
			int blockAddress = getShockBlockAddress(chunkId, tmp_ark, &chunkPackedLength, &chunkUnpackedLength, &chunkType);
			if (blockAddress != -1)
				{
				fprintf(LOGFILE, "\nChunk %d, type %d", chunkId, chunkType);
				art_ark = new unsigned char[chunkUnpackedLength];
				LoadShockChunk(blockAddress, chunkType, tmp_ark, art_ark, chunkPackedLength, chunkUnpackedLength);
				if (chunkContentType == 3)
					{//Font data
					long fontAddressPointer = 0;
					int isColour = 0;
					//printf("\nChunk No : %d, BlockAddress : %d", chunkId, blockAddress, fontAddressPointer);

					//printf("\nColor Flag : %d (@ %d)", getValAtAddress(art_ark, fontAddressPointer, 16), fontAddressPointer);
					if (getValAtAddress(art_ark, fontAddressPointer, 16) != 0)
						{
						isColour = 1;
						}
					fontAddressPointer = fontAddressPointer + 2 + 34;
					//printf("\nFirst ASCII Char : %d (@ %d)", getValAtAddress(art_ark, fontAddressPointer, 16), fontAddressPointer);
					int firstChar = getValAtAddress(art_ark, fontAddressPointer, 16);
					fontAddressPointer = fontAddressPointer + 2;
					//printf("\nLast ASCII Char : %d (@ %d)", getValAtAddress(art_ark, fontAddressPointer, 16), fontAddressPointer);
					int LastChar = getValAtAddress(art_ark, fontAddressPointer, 16);
					int positionTableSize = LastChar-firstChar + 1;
					fontAddressPointer = fontAddressPointer + 2 + 32;
					//printf("\nPosition Table Offset : %d (@ %d)", getValAtAddress(art_ark, fontAddressPointer, 32), fontAddressPointer);
					long PositionTable = getValAtAddress(art_ark, fontAddressPointer, 32);
					fontAddressPointer = fontAddressPointer + 4;
					//printf("\nBitmap Data Offset : %d (@ %d)", getValAtAddress(art_ark, fontAddressPointer, 32), fontAddressPointer);
					long textureOffset = getValAtAddress(art_ark, fontAddressPointer, 32);
					fontAddressPointer = fontAddressPointer + 4;
					//printf("\nBitmap Width : %d (@ %d)", getValAtAddress(art_ark, fontAddressPointer, 16), fontAddressPointer);
					int width = getValAtAddress(art_ark, fontAddressPointer, 16);
					fontAddressPointer = fontAddressPointer + 2;
					//printf("\nBitmap Height : %d (@ %d)", getValAtAddress(art_ark, fontAddressPointer, 16), fontAddressPointer);
					int SizeV = getValAtAddress(art_ark, fontAddressPointer, 16);
					//WriteShockFont(art_ark, pal, 0, textureOffset, NewOutFileName, useTGA, width, SizeV, isColour);
					//Output the position chart.
					int pos_ptr = 0;
					for (long index = 0; index < positionTableSize; index++)
						{
						//int SizeH =  getValAtAddress(art_ark, PositionTable + pos_ptr + 2, 16) - getValAtAddress(art_ark, PositionTable + pos_ptr, 16);
						int GlyphOffset = getValAtAddress(art_ark, PositionTable + pos_ptr, 16);
						int GlyphOffsetNext = getValAtAddress(art_ark, PositionTable + pos_ptr +2 , 16);
						int SizeH = GlyphOffsetNext-GlyphOffset;
						unsigned char *bits;
						if (isColour==1)
							bits = art_ark + textureOffset + GlyphOffset;
						else
							{
							bits = art_ark + textureOffset + ((GlyphOffset - (GlyphOffset % 8)) / 8);
							}
							

						pos_ptr = pos_ptr+2;
//						unsigned char *bits;
						//bits = art_ark + textureOffset + ((GlyphOffset - (GlyphOffset % 8)) / 8);

						//printf("\nWidth%d", getValAtAddress(art_ark, PositionTable + pos_ptr + 2, 16) - getValAtAddress(art_ark, PositionTable + pos_ptr, 16));
						//WriteShockFont(art_ark,pal,k+1,GlyphAddress,NewOutFileName, useTGA, GlyphWidth,height,isColour);
						if (useTGA == 0)
							{
							BitMapHeader bmhead;
							BitMapInfoHeader bmihead;
							bmhead.bfType = 19778;
							bmhead.bfReserved1 = 0;
							bmhead.bfReserved2 = 0;
							bmhead.bfOffBits = 1078;
							bmihead.biSize = 40;
							bmihead.biPlanes = 1;
							bmihead.biBitCount = 8;
							bmihead.biCompression = 0;
							bmihead.biXPelsPerMeter = 0;
							bmihead.biYPelsPerMeter = 0;
							bmihead.biClrUsed = 0;
							bmihead.biClrImportant = 0;
							bmhead.bfOffBits = 1078; // Set up the .bmp header info
							bmihead.biBitCount = 8;
							bmihead.biClrUsed = 0;
							bmihead.biClrImportant = 0;
							bmihead.biWidth = SizeH;	//bm->width;
							bmihead.biHeight = SizeV;	// bm->height;
							int imwidth = SizeH;		//bmihead.biWidth;
							imwidth += (4 - (imwidth % 4));
							bmihead.biSizeImage = imwidth * bmihead.biHeight;
							bmhead.bfSize = bmihead.biSizeImage + 54;
							
							if (isColour == 0)
								{//Black & White bitmap 
								bmhead.bfOffBits = 62;
								bmihead.biBitCount = 1;
								bmihead.biClrUsed = 0;
								bmihead.biClrImportant = 0;
								}
							char outFile[255];
							sprintf_s(outFile, 255, "%s_%04d_%04d.bmp", OutFileName, chunkId, index);
							
							outf = fopen(outFile, "wb");

							fwrite(&bmhead.bfType, 2, 1, outf);
							fwrite(&bmhead.bfSize, 4, 1, outf);
							fwrite(&bmhead.bfReserved1, 2, 1, outf);
							fwrite(&bmhead.bfReserved2, 2, 1, outf);
							fwrite(&bmhead.bfOffBits, 4, 1, outf);
							fwrite(&bmihead, sizeof(BitMapInfoHeader), 1, outf);
							char ch = 0;
							if (isColour == 1)
								{//COLOUR BITMAP
								fwrite(pal, 256 * 4, 1, outf); // write full game palette
								char ch = 0;
								for (int k = bmihead.biHeight - 1; k >= 0; k--) {
									fwrite(bits + (k*width), 1, bmihead.biWidth, outf);
									if (bmihead.biWidth % 4 != 0)
									for (int buf = 4; buf > bmihead.biWidth % 4; buf--)
										fwrite(&ch, 1, 1, outf);
									}
								fclose(outf);
								}
							else
								{
								//B&W bitmap
								//unsigned char *bits;
								//bits = art_ark + textureOffset + ((GlyphOffset - (GlyphOffset % 8)) / 8);
								
								short int tmpshrt = 0;
								fwrite(&tmpshrt, 2, 1, outf);
								fwrite(&tmpshrt, 2, 1, outf);
								tmpshrt = -1;
								fwrite(&tmpshrt, 2, 1, outf);
								tmpshrt = 255;
								fwrite(&tmpshrt, 2, 1, outf);

								unsigned char outc;
								unsigned char tempc1;
								unsigned char tempc2;
								unsigned short int shift = GlyphOffset % 8;
								//Black & White Bitmap
								for (int k = bmihead.biHeight - 1; k >= 0; k--) 
									{
									for (int q = 0; q <= (bmihead.biWidth - bmihead.biWidth % 8) / 8; q++) 
										{
										tempc1 = *(bits + (k*width) + q);
										tempc2 = *(bits + (k*width) + q + 1);
										outc = (tempc1 << shift) + (tempc2 >> (8 - shift));
										fwrite(&outc, 1, 1, outf);
										}
									if ((((bmihead.biWidth - bmihead.biWidth % 8) / 8) + 1) % 4 != 0)
										{
										for (int buf = 4; buf > (((bmihead.biWidth - bmihead.biWidth % 8) / 8) + 1) % 4; buf--)
											{
											fwrite(&ch, 1, 1, outf);
											}
										}
									}
								fclose(outf);
								}
							}
						else
							{//TGA
							if (isColour == 1)
								{//COLOUR TGA
								char outFile[255];
								sprintf_s(outFile, 255, "%s_%04d_%04d.tga", OutFileName, chunkId, firstChar+index);
								/* Write the result as a uncompressed TGA */
								if ((outf = fopen(outFile, "wb")) == NULL) {
									fprintf(stderr, "Failed to open outputfile\n");
									exit(-1);
									}
								putc(0, outf);
								putc(0, outf);
								putc(2, outf);/* uncompressed RGB */
								putc(0, outf); putc(0, outf);
								putc(0, outf); putc(0, outf);
								putc(0, outf);
								putc(0, outf); putc(0, outf);           /* X origin */
								putc(0, outf); putc(0, outf);           /* y origin */
								putc((SizeH & 0x00FF), outf);
								putc((SizeH & 0xFF00) / 256, outf);
								putc((SizeV & 0x00FF), outf);
								putc((SizeV & 0xFF00) / 256, outf);
								putc(32, outf);                        /* 32 bit bitmap */
								putc(8, outf);
								for (int iRow = SizeV - 1; iRow >= 0; iRow--)
									{
									int rowStart = iRow*width;
									for (int j = 0; j <SizeH; j++)
										{
										int pixel = getValAtAddress(bits, rowStart+j , 8);
										putc(pal[pixel].blue, outf);
										putc(pal[pixel].green, outf);
										putc(pal[pixel].red, outf);
										if (Alpha == 1)
											{
											if (pixel != 0)	//Alpha
												{
												fputc(255, outf);
												}
											else
												{
												fputc(0, outf);
												}
											}
										else
											{
											fputc(255, outf);//No alpha
											}
										}
									}
								}
							else
								{
								//COLOUR TGA.

								//Generate a tmp file of bitmap data
								BitMapHeader bmhead;
								BitMapInfoHeader bmihead;
								bmihead.biWidth = SizeH;	//bm->width;
								bmihead.biHeight = SizeV;	// bm->height;
								char ch = 0;
								unsigned char outc;
								unsigned char tempc1;
								unsigned char tempc2;
								unsigned short int shift = GlyphOffset % 8;

								char outFile[255];
								sprintf_s(outFile, 255, "%s_%04d_%04d.tga", OutFileName, chunkId, firstChar + index);
								/* Write the result as a uncompressed TGA */
								if ((outf = fopen(outFile, "wb")) == NULL) {
									fprintf(stderr, "Failed to open outputfile\n");
									exit(-1);
									}
								putc(0, outf);
								putc(0, outf);
								putc(2, outf);/* uncompressed RGB */
								putc(0, outf); putc(0, outf);
								putc(0, outf); putc(0, outf);
								putc(0, outf);
								putc(0, outf); putc(0, outf);           /* X origin */
								putc(0, outf); putc(0, outf);           /* y origin */
								putc((SizeH & 0x00FF), outf);
								putc((SizeH & 0xFF00) / 256, outf);
								putc((SizeV & 0x00FF), outf);
								putc((SizeV & 0xFF00) / 256, outf);
								putc(32, outf);                        /* 32 bit bitmap */
								putc(8, outf);
								//printf("\nBitmap %d\n", index);
								for (int k = bmihead.biHeight - 1; k >= 0; k--)
									{
									int BitsRemaining=SizeH;
									for (int q = 0; q <= (bmihead.biWidth - bmihead.biWidth % 8) / 8; q++)
										{
										int NextBit = 7;
										tempc1 = *(bits + (k*width) + q);
										tempc2 = *(bits + (k*width) + q + 1);
										outc = (tempc1 << shift) + (tempc2 >> (8 - shift));
										//fwrite(&outc, 1, 1, outf);
										int pixel =outc;
										while ((NextBit >= 0) && (BitsRemaining >0))
											{
											int bit = (pixel >> NextBit) & 0x1;
											if (bit == 1)
												{
												fputc(255, outf);
												fputc(255, outf);
												fputc(255, outf);
												fputc(255, outf);
												}
											else
												{
												fputc(0, outf);
												fputc(0, outf);
												fputc(0, outf);
												fputc(0, outf);
												}
												//printf("%d",bit);
											NextBit--; 
											BitsRemaining--;
											}
											//printf("\n");
										//Convert the bits in outc in to pixels
										}
									}
								fclose(outf);
								}
							}
						fclose(outf);
						}
					}
				}
			}
		else
			{
			fprintf(LOGFILE, "Graphics chunk %d not found in %s\n", chunkId, GraphicsFile);
			}
		}
	}


void ExtractWeaponAnimations(int ImageCount, char filePathIn[255], char PaletteFile[255], int PaletteNo, int BitmapSize, int FileType, char OutFileName[255], char auxPalPath[255], char animFile[255], int useTGA , int game)
	{
//todo tomorrow
	unsigned char *AnimData;
	unsigned char *textureFile;         
	int i;
	long NoOfTextures;
	int alpha=0;

	FILE *file = NULL;      // File pointer

	if ((file = fopen(filePathIn, "rb")) == NULL)
		{
		fprintf(LOGFILE, "Could not open specified file\n"); return;
		}

	// Get the size of the file in bytes
	long fileSize = getFileSize(file);

	palette *pal;
	pal = new palette[256];
	getPalette(PaletteFile, pal, PaletteNo);
	textureFile = new unsigned char[fileSize];
	fread(textureFile, fileSize, 1, file);
	fclose(file);

	if ((file = fopen(animFile, "rb")) == NULL)
		{
		fprintf(LOGFILE, "Could not open specified file\n"); return;
		}
	fileSize = getFileSize(file);
	AnimData = new unsigned char[fileSize];
	fread(AnimData, fileSize, 1, file);
	int AnimX[390];
	int AnimY[390];
	int AnimXY[776];//For UW2
int offset=0;
int add_ptr=0;


/*
for (int i = 0; i < fileSize; i++)
	{
	if (i % 2 == 0)
		{
		AnimX[offset] = getValAtAddress(AnimData, i, 8);
		}
	else
		{
		AnimY[offset++] = getValAtAddress(AnimData, i, 8);
		}
	}
*/
offset=0;
int GroupSize=28; //28 for uw1
	if (game != UW2)
	{
		GroupSize = 28;
	
		for (int i = 0; i<8; i++)
			{
			for (int j = 0; j<GroupSize; j++)
				{
				AnimX[j + offset] = getValAtAddress(AnimData, add_ptr++, 8);
				}
			for (int j = 0; j<GroupSize; j++)
				{
				AnimY[j + offset] = getValAtAddress(AnimData, add_ptr++, 8);
				}
			offset = offset + GroupSize;
			}
	}
	else
		{//In UW2 I just read the values into one array
		for (int i = 0; i < fileSize; i++)
			{
			AnimXY[i] = getValAtAddress(AnimData, add_ptr++, 8);
			}
		}


add_ptr=0;
	for (int i = 0; i<fileSize; i++)
		{
		fprintf(LOGFILE, "%d\n", getValAtAddress(AnimData, add_ptr++, 8));
		}
		fprintf(LOGFILE, "File Type (should be %d):%d\n", FileType, textureFile[0]);
		fprintf(LOGFILE, "No of textures:%d\n", textureFile[2] << 8 | textureFile[1]);
		if (ImageCount == -1)	//All the images.
			{
			NoOfTextures = textureFile[2] << 8 | textureFile[1];
			}
		else
			{
			NoOfTextures = ImageCount;
			}

//Offsets into weap.dat for UW2 for the various weapon frames.
		int UW2_X[232] = { 35, 36, 37, -1, 39, 40, 41, 42, -1, 44, 45, 46, -1, 48, 49, 50, 51, -1, -1, -1, -1, -1, 57, 58, 59, -1, -1, 62, 63, 64, -1, 132, 133, 134, -1, 136, 137, 138, 139, -1, 141, 142, 143, -1, 145, 146, 147, 148, -1, -1, -1, -1, -1, 154, 155, 156, -1, -1, 159, 160, 161, -1, 229, 230, 231, -1, 233, 234, 235, 236, -1, 238, 239, 240, -1, 242, 243, 244, 245, -1, -1, -1, -1, -1, 251, 252, 253, -1, -1, 256, 257, 258, -1, -1, -1, -1, -1, 330, 331, 332, -1, -1, 335, -1, 337, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 423, 424, 425, -1, 427, 428, 429, 430, -1, 432, 433, 434, -1, 436, 437, 438, 439, -1, -1, -1, -1, -1, 446, 447, 448, -1, -1, -1, 450, 451, 452, 520, 521, 522, -1, 524, 525, 526, 527, -1, 529, 530, 531, -1, 533, 534, 535, 536, -1, -1, -1, -1, -1, 542, 543, 544, -1, -1, 547, 548, 549, -1, 617, 618, 619, -1, 621, 622, 623, 624, -1, 626, 627, 628, -1, 630, 631, 632, 633, -1, -1, -1, -1, -1, 639, 640, 641, -1, -1, 644, 645, 646, -1, -1, -1, -1, -1, 718, 719, 720, -1, 723, -1, 725, -1, -1 };
		int UW2_Y[232] = { 66, 67, 68, -1, 70, 71, 72, 73, -1, 75, 76, 77, -1, 79, 80, 81, 82, -1, -1, -1, -1, -1, 88, 89, 90, -1, -1, 93, 94, 95, -1, 163, 164, 165, -1, 167, 168, 169, 170, -1, 172, 173, 174, -1, 176, 177, 178, 179, -1, -1, -1, -1, -1, 185, 186, 187, -1, -1, 190, 191, 192, -1, 260, 261, 262, -1, 264, 265, 266, 267, -1, 269, 270, 271, -1, 273, 274, 275, 276, -1, -1, -1, -1, -1, 282, 283, 284, -1, -1, 287, 288, 289, -1, -1, -1, -1, -1, 361, 362, 363, -1, -1, 366, -1, 368, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 454, 455, 456, -1, 458, 459, 460, 461, -1, 463, 464, 465, -1, 467, 468, 469, 470, -1, -1, -1, -1, -1, 476, 477, 478, -1, -1, -1, 481, 482, 483, 551, 552, 553, -1, 555, 556, 557, 558, -1, 560, 561, 562, -1, 564, 565, 566, 567, -1, -1, -1, -1, -1, 573, 574, 575, -1, -1, 578, 579, 580, -1, 648, 649, 650, -1, 652, 653, 654, 655, -1, 657, 658, 659, -1, 661, 662, 663, 664, -1, -1, -1, -1, -1, 670, 671, 672, -1, -1, 675, 676, 677, -1, -1, -1, -1, -1, 749, 750, 751, -1, 754, -1, 756, -1, -1 };

		for (i = 0; i < NoOfTextures; i++)
			{
			int MaxHeight = 112;
			int MaxWidth = 172;
			if (game == UW2)
				{
				MaxHeight = 128;
				MaxWidth = 208;
				}
			if (i>220)
				{
				printf("H");
				}
			long textureOffset = getValAtAddress(textureFile, (i * 4) + 3, 32);
			int BitMapWidth = getValAtAddress(textureFile, textureOffset + 1, 8);
			int BitMapHeight = getValAtAddress(textureFile, textureOffset + 2, 8);
			int datalen;
			palette auxpal[16];
			int auxPalIndex;
			unsigned char *imgNibbles;
			unsigned char *outputImg;
			unsigned char *srcImg;
			//fprintf(LOGFILE, "4 bit run-length\n");
			//auxPalIndex = getValAtAddress(textureFile, textureOffset + 3, 8);
			auxPalIndex = 1;
			datalen = getValAtAddress(textureFile, textureOffset + 4, 16);
			imgNibbles = new unsigned char[BitMapWidth*BitMapHeight * 2];
			textureOffset = textureOffset + 6;	//Start of raw data.
			copyNibbles(textureFile, imgNibbles, datalen, textureOffset);
			LoadAuxilaryPal(auxPalPath, auxpal, pal, auxPalIndex);
			srcImg = new unsigned char[BitMapWidth*BitMapHeight];
			outputImg = new unsigned char[MaxWidth*MaxHeight];
			DecodeRLEBitmap(imgNibbles, datalen, BitMapWidth, BitMapHeight, srcImg, 4);

			//Paste source image into output image.
			int ColCounter = 0; int RowCounter = 0;
			int cornerX;// = AnimX[i];
			int cornerY;// = AnimY[i];
			if (game != UW2)
				{
				cornerX = AnimX[i];
				cornerY = AnimY[i];
				}
			else
				{//Read from XY
				if (UW2_X[i] != -1)
					{
					cornerX = AnimXY[UW2_X[i]];
					cornerY = AnimXY[UW2_Y[i]];
					}
				else
					{
					cornerX=0;
					cornerY=BitMapHeight;
					}
				}

			if ((game == UW1) || ((game == UW2) && (UW2_X[i] != -1)))//Only create if UW1 image or a valid uw2 one
			{
			fprintf(LOGFILE, "Image %d, Corner X = %d Corner Y = %d\n",i,cornerX,cornerY);
			bool ImgStarted = false;
			for (int y = 0; y < MaxHeight; y++)
				{
				for (int x = 0; x < MaxWidth; x++)
					{
					if ((cornerX + ColCounter == x) && (MaxHeight - cornerY + RowCounter == y) && (ColCounter<BitMapWidth) && (RowCounter<BitMapHeight))
						{//the pixel from the source image is here 
						ImgStarted = true;
						outputImg[x + (y*MaxWidth)] = srcImg[ColCounter + (RowCounter*BitMapWidth)];
						ColCounter++;
						}
					else
						{
						if ((game == UW2) && (UW2_X[i] == -1))
							{
							alpha=50;
							}
						else
							{
							alpha=0;//0
							}
						outputImg[x + (y*MaxWidth)] = alpha;
						}
					}
				if (ImgStarted == true)
					{//New Row on the src image
					RowCounter++;
					ColCounter = 0;
					}
				}
			char ImageName[255];

			sprintf(ImageName, "%s_%d",OutFileName,auxPalIndex);
			if (useTGA == 1)
				{
				writeTGA(outputImg, 0, MaxWidth, MaxHeight, i, auxpal, ImageName, ALPHA);
				}
			else
				{
				writeBMP(outputImg, 0, MaxWidth, MaxHeight, i, auxpal, ImageName);
				}
			}
	}
}