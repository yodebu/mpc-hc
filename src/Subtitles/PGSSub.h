/*
 * (C) 2006-2014 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "RLECodedSubtitle.h"
#include "CompositionObject.h"

class CGolombBuffer;

class CPGSSub : public CRLECodedSubtitle
{
public:
    CPGSSub(CCritSec* pLock, const CString& name, LCID lcid);
    ~CPGSSub();

    // ISubPicProvider
    STDMETHODIMP_(POSITION)       GetStartPosition(REFERENCE_TIME rt, double fps);
    STDMETHODIMP_(POSITION)       GetNext(POSITION pos);
    STDMETHODIMP_(REFERENCE_TIME) GetStart(POSITION pos, double fps);
    STDMETHODIMP_(REFERENCE_TIME) GetStop(POSITION pos, double fps);
    STDMETHODIMP_(bool)           IsAnimated(POSITION pos);
    STDMETHODIMP                  Render(SubPicDesc& spd, REFERENCE_TIME rt, double fps, RECT& bbox);
    STDMETHODIMP                  GetTextureSize(POSITION pos, SIZE& MaxTextureSize, SIZE& VirtualSize, POINT& VirtualTopLeft);

    virtual HRESULT ParseSample(IMediaSample* pSample);
    virtual void    EndOfStream() { /* Nothing to do */ };
    virtual void    Reset();

private:
    enum HDMV_SEGMENT_TYPE {
        NO_SEGMENT       = 0xFFFF,
        PALETTE          = 0x14,
        OBJECT           = 0x15,
        PRESENTATION_SEG = 0x16,
        WINDOW_DEF       = 0x17,
        INTERACTIVE_SEG  = 0x18,
        END_OF_DISPLAY   = 0x80,
        HDMV_SUB1        = 0x81,
        HDMV_SUB2        = 0x82
    };


    struct VIDEO_DESCRIPTOR {
        int  nVideoWidth;
        int  nVideoHeight;
        BYTE bFrameRate;
    };

    struct COMPOSITION_DESCRIPTOR {
        short nNumber;
        BYTE  bState;
    };

    struct SEQUENCE_DESCRIPTOR {
        BYTE bFirstIn  : 1;
        BYTE bLastIn   : 1;
        BYTE bReserved : 6;
    };

    struct HDMV_CLUT {
        BYTE id;
        BYTE version_number;
        BYTE size;

        HDMV_PALETTE palette[256];

        HDMV_CLUT()
            : id(0)
            , version_number(0)
            , size(0) {
            ZeroMemory(palette, sizeof(palette));
        }
    };

    struct HDMV_PRESENTATION_SEGMENT {
        REFERENCE_TIME rtStart;
        REFERENCE_TIME rtStop;

        VIDEO_DESCRIPTOR video_descriptor;
        COMPOSITION_DESCRIPTOR composition_descriptor;

        byte palette_update_flag;
        HDMV_CLUT CLUT;

        int objectCount;

        CAutoPtrList<CompositionObject> objects;
    };

    HDMV_SEGMENT_TYPE m_nCurSegment;
    BYTE*             m_pSegBuffer;
    int               m_nTotalSegBuffer;
    int               m_nSegBufferPos;
    int               m_nSegSize;

    CAutoPtr<HDMV_PRESENTATION_SEGMENT>     m_pCurrentPresentationSegment;
    CAutoPtrList<HDMV_PRESENTATION_SEGMENT> m_pPresentationSegments;

    HDMV_CLUT m_CLUTs[256];
    CompositionObject m_compositionObjects[64];

    void AllocSegment(int nSize);
    int  ParsePresentationSegment(REFERENCE_TIME rt, CGolombBuffer* pGBuffer);
    void EnqueuePresentationSegment();
    void UpdateTimeStamp(REFERENCE_TIME rtStop);

    void ParsePalette(CGolombBuffer* pGBuffer, unsigned short nSize);
    void ParseObject(CGolombBuffer* pGBuffer, unsigned short nUnitSize);
    void ParseVideoDescriptor(CGolombBuffer* pGBuffer, VIDEO_DESCRIPTOR* pVideoDescriptor);
    void ParseCompositionDescriptor(CGolombBuffer* pGBuffer, COMPOSITION_DESCRIPTOR* pCompositionDescriptor);
    void ParseCompositionObject(CGolombBuffer* pGBuffer, const CAutoPtr<CompositionObject>& pCompositionObject);

    POSITION FindPresentationSegment(REFERENCE_TIME rt) const;

    void RemoveOldSegments(REFERENCE_TIME rt);
};