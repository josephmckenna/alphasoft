//==============================================================================
// File:        PDrawPostscriptFile.h
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
/*
** Postscript file drawable - PH 09/22/99
*/
#ifndef __PDrawPostscriptFile_h__
#define __PDrawPostscriptFile_h__

#include <stdio.h>
#include "PDrawable.h"

const short kMaxPSNameLen       = 256;

class PDrawPostscriptFile : public PDrawable
{
public:
    PDrawPostscriptFile(char *filename, int landscape=0);
    virtual ~PDrawPostscriptFile();
    
    virtual int     BeginDrawing(int width,int height);
    virtual void    EndDrawing();
    
    virtual void    SetForeground(int col_num, int alpha=0xffff);
    virtual int     EqualColours(int col1, int col2);
    virtual void    SetLineWidth(float width);
    virtual void    SetLineType(ELineType type);
    virtual void    SetFont(XFontStruct *font);
    virtual int     GetTextWidth(char *str);
    virtual void    Comment(char *str);
    virtual void    DrawSegments(XSegment *segments, int num, int smooth=1);
    virtual void    DrawLine(int x1,int y1,int x2,int y2);
    virtual void    FillRectangle(int x,int y,int w,int h);
    virtual void    FillPolygon(XPoint *point, int num);
    virtual void    DrawString(int x, int y, const char *str, ETextAlign_q align);
    virtual void    DrawArc(int cx,int cy,int rx,int ry,float ang1,float ang2);
    virtual void    FillArc(int cx,int cy,int rx,int ry,float ang1,float ang2);
    virtual void    Comment(const char */*str*/){ printf("PDrawPostscriptFile::Comment() not implemented\n"); };
    virtual void    SetForegroundPixel(Pixel /*pixel*/, int /*alpha=0xffff*/){ printf("PDrawPostscriptFile::SetForegroundPixel() not implemented\n"); };
    virtual void    DrawPoint(int /*x*/, int /*y*/){ printf("PDrawPostscriptFile::DrawPoint() not implemented\n"); };
    virtual void    DrawRectangle(int /*x*/,int /*y*/,int /*w*/,int /*h*/){ printf("PDrawPostscriptFile::DrawRectangle() not implemented\n"); };
    virtual void    PutImage(XImage */*image*/, int /*dest_x*/, int /*dest_y*/){ printf("PDrawPostscriptFile::PutImage() not implemented\n"); };

    virtual EDevice GetDeviceType()     { return kDevicePrinter; }

private:
    char            mFilename[kMaxPSNameLen];
    char            mBoundingBoxStr[256];
    FILE          * mFile;
    XColor        * mColours;
    int             mIsEPS;
    int             mIsLandscape;
};


#endif // __PDrawPostscriptFile_h__
