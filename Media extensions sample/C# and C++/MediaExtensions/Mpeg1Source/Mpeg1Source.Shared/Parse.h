//////////////////////////////////////////////////////////////////////////
//
// Parse.h
// MPEG-1 parsing code.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once


// Note: The structs, enums, and constants defined in this header are not taken from
// Media Foundation or DirectShow headers. The parser code is written to be API-agnostic.
// The only exceptions are:
//    - Use of the MFRatio structure to describe ratios.
//    - The MPEG1AudioFlags enum defined here maps directly to the equivalent DirectShow flags.

// Sizes
const DWORD MPEG1_MAX_PACKET_SIZE = 65535 + 6;          // Maximum packet size.
const DWORD MPEG1_PACK_HEADER_SIZE = 12;                // Pack header.

const DWORD MPEG1_SYSTEM_HEADER_MIN_SIZE = 12;          // System header, excluding the stream info.
const DWORD MPEG1_SYSTEM_HEADER_PREFIX = 6;             // This value + header length = total size of the system header.
const DWORD MPEG1_SYSTEM_HEADER_STREAM = 3;             // Size of each stream info in the system header.

const DWORD MPEG1_PACKET_HEADER_MIN_SIZE = 6;           // Minimum amount to read in the packet header. (Up to the variable-sized padding bytes)
const DWORD MPEG1_PACKET_HEADER_MAX_STUFFING_BYTE = 16; // Maximum number of stuffing bytes in a packet header.
const DWORD MPEG1_PACKET_HEADER_MAX_SIZE = 34;          // Maximum size of a packet header.

const DWORD MPEG1_VIDEO_SEQ_HEADER_MIN_SIZE = 12;       // Minimum length of the video sequence header.
const DWORD MPEG1_VIDEO_SEQ_HEADER_MAX_SIZE = 140;      // Maximum length of the video sequence header.

const DWORD MPEG1_AUDIO_FRAME_HEADER_SIZE = 4;


// Codes
const DWORD MPEG1_START_CODE_PREFIX     = 0x00000100;
const DWORD MPEG1_PACK_START_CODE       = 0x000001BA;
const DWORD MPEG1_SYSTEM_HEADER_CODE    = 0x000001BB;
const DWORD MPEG1_SEQUENCE_HEADER_CODE  = 0x000001B3;
const DWORD MPEG1_STOP_CODE             = 0x000001B9;

// Stream ID codes
const BYTE MPEG1_STREAMTYPE_ALL_AUDIO = 0xB8;
const BYTE MPEG1_STREAMTYPE_ALL_VIDEO = 0xB9;
const BYTE MPEG1_STREAMTYPE_RESERVED = 0xBC;
const BYTE MPEG1_STREAMTYPE_PRIVATE1 = 0xBD;
const BYTE MPEG1_STREAMTYPE_PADDING = 0xBE;
const BYTE MPEG1_STREAMTYPE_PRIVATE2 = 0xBF;
const BYTE MPEG1_STREAMTYPE_AUDIO_MASK = 0xC0;
const BYTE MPEG1_STREAMTYPE_VIDEO_MASK = 0xE0;
const BYTE MPEG1_STREAMTYPE_DATA_MASK = 0xF0;



// Systems layer

enum StreamType
{
    StreamType_Unknown,
    StreamType_AllAudio,
    StreamType_AllVideo,
    StreamType_Reserved,
    StreamType_Private1,
    StreamType_Padding,
    StreamType_Private2,
    StreamType_Audio,   // ISO/IEC 11172-3
    StreamType_Video,   // ISO/IEC 11172-2
    StreamType_Data
};

struct MPEG1StreamHeader
{
    BYTE        stream_id;  // Raw stream_id field.
    StreamType  type;       // Stream type (audio, video, etc)
    BYTE        number;     // Index within the stream type (audio 0, audio 1, etc)
    DWORD       sizeBound;
};

// MPEG1SystemHeader
// Holds information from the system header. This structure is variable
// length, because the last field is an array of stream headers.
struct MPEG1SystemHeader
{
    DWORD   cbSize;     // Size of this structure, including the streams array.
    DWORD   rateBound;
    BYTE    cAudioBound;
    bool    bFixed;
    bool    bCSPS;
    bool    bAudioLock;
    bool    bVideoLock;
    BYTE    cVideoBound;
    DWORD   cStreams;
    MPEG1StreamHeader streams[1];   // Array of 1 or more stream headers.
};

struct MPEG1PacketHeader
{
    BYTE        stream_id;      // Raw stream_id field.
    StreamType  type;           // Stream type (audio, video, etc)
    BYTE        number;         // Index within the stream type (audio 0, audio 1, etc)
    DWORD       cbPacketSize;   // Size of the entire packet (header + payload).
    DWORD       cbPayload;      // Size of the packet payload (packet size - header size).
    bool        bHasPTS;        // Did the packet header contain a Presentation Time Stamp (PTS)?
    LONGLONG    PTS;            // Presentation Time Stamp (in 90 kHz clock)
};

// Video

struct MPEG1VideoSeqHeader
{
    WORD        width;
    WORD        height;
    MFRatio     pixelAspectRatio;
    MFRatio     frameRate;
    DWORD       bitRate;
    WORD        cbVBV_Buffer;
    bool        bConstrained;
    DWORD       cbHeader;
    BYTE        header[MPEG1_VIDEO_SEQ_HEADER_MAX_SIZE];    // Raw header.
};

// Audio

enum MPEG1AudioLayer
{
    MPEG1_Audio_Layer1 = 0,
    MPEG1_Audio_Layer2,
    MPEG1_Audio_Layer3
};

enum MPEG1AudioMode
{
    MPEG1_Audio_Stereo = 0,
    MPEG1_Audio_JointStereo,
    MPEG1_Audio_DualChannel,
    MPEG1_Audio_SingleChannel
};


// Various bit flags used in the audio frame header.
// (Note: These enum values are not the actual values in the audio frame header.)
enum MPEG1AudioFlags
{
    MPEG1_AUDIO_PRIVATE_BIT    = 0x01,  // = ACM_MPEG_PRIVATEBIT
    MPEG1_AUDIO_COPYRIGHT_BIT  = 0x02,  // = ACM_MPEG_COPYRIGHT
    MPEG1_AUDIO_ORIGINAL_BIT   = 0x04,  // = ACM_MPEG_ORIGINALHOME
    MPEG1_AUDIO_PROTECTION_BIT = 0x08,  // = ACM_MPEG_PROTECTIONBIT
};

struct  MPEG1AudioFrameHeader
{
    MPEG1AudioLayer     layer;
    DWORD               dwBitRate;        // Bit rate in Kbits / sec
    DWORD               dwSamplesPerSec;
    WORD                nBlockAlign;
    WORD                nChannels;
    MPEG1AudioMode      mode;
    BYTE                modeExtension;
    BYTE                emphasis;
    WORD                wFlags;    // bitwise OR of MPEG1AudioFlags
};

// ExpandableStruct class:
// Class which wraps a structure which can be expanded by additional data.
template<typename T>
ref class ExpandableStruct sealed
{
internal:
    ExpandableStruct(unsigned int size)
        : m_array(ref new Array<BYTE>(size))
    {
        ZeroMemory(m_array->Data, m_array->Length);
    }

    ExpandableStruct(const ExpandableStruct<T> ^src)
        : m_array(ref new Array<BYTE>(src->Size))
    {
        CopyFrom(src);
    }
    
    property unsigned int Size {unsigned int get() const { return m_array->Length; }}
    T *Get() const { return reinterpret_cast<T*>(m_array->Data); }
    void CopyFrom(const ExpandableStruct<T> ^src)
    {
        if (src->Size != Size)
        {
            throw ref new InvalidArgumentException();
        }

        CopyMemory(Get(), src->Get(), Size);
    }

private:
    Array<BYTE> ^m_array;
};


// Buffer class:
// Resizable buffer used to hold the MPEG-1 data.

ref class Buffer sealed
{
internal:
    Buffer(DWORD cbSize);
    HRESULT Initalize(DWORD cbSize);

    property BYTE *DataPtr { BYTE *get(); }
    property DWORD DataSize { DWORD get() const; }

    // Reserve: Reserves cb bytes of free data in the buffer.
    // The reserved bytes start at DataPtr() + DataSize().
    void Reserve(DWORD cb);

    // MoveStart: Moves the front of the buffer.
    // Call this method after consuming data from the buffer.
    void MoveStart(DWORD cb);

    // MoveEnd: Moves the end of the buffer.
    // Call this method after reading data into the buffer.
    void MoveEnd(DWORD cb);

private:
    property BYTE *Ptr { BYTE *get() { return m_array->Data; } }

    void SetSize(DWORD count);
    void Allocate(DWORD alloc);

    property DWORD  CurrentFreeSize { DWORD get() const; }

private:

    Array<BYTE> ^m_array;
    DWORD m_count;        // Nominal count.
    DWORD m_allocated;    // Actual allocation size.

    DWORD m_begin;
    DWORD m_end;  // 1 past the last element
};


// Parser class:
// Parses an MPEG-1 systems-layer stream.
ref class Parser sealed
{
internal:
    Parser();

    bool ParseBytes(const BYTE *pData, DWORD cbLen, DWORD *pAte);

    property bool HasSystemHeader{bool get() const { return m_header != nullptr; }}
    ExpandableStruct<MPEG1SystemHeader> ^GetSystemHeader();

    property bool HasPacket {bool get() const { return m_bHasPacketHeader; }}
    property const MPEG1PacketHeader &PacketHeader { const MPEG1PacketHeader &get() { assert(m_bHasPacketHeader); return m_curPacketHeader; } }

    property DWORD PayloadSize{DWORD get() const { assert(m_bHasPacketHeader); return m_curPacketHeader.cbPayload; }}
    void ClearPacket() { m_bHasPacketHeader = false; }

    property bool IsEndOfStream {bool get() const { return m_bEOS; }}

private:

    bool FindNextStartCode(const BYTE *pData, DWORD cbLen, DWORD *pAte);
    bool ParsePackHeader(const BYTE *pData, DWORD cbLen, DWORD *pAte);
    bool ParseSystemHeader(const BYTE *pData, DWORD cbLen, DWORD *pAte);
    bool ParsePacketHeader(const BYTE *pData, DWORD cbLen, DWORD *pAte);
    void OnEndOfStream();

private:

    LONGLONG m_SCR;
    DWORD m_muxRate;
    Array<BYTE> ^m_data;

    ExpandableStruct<MPEG1SystemHeader> ^m_header;
    // Note: Size of header = sizeof(MPEG1SystemHeader) + (sizeof(MPEG1StreamHeader) * (cStreams - 1))

    bool m_bHasPacketHeader;
    MPEG1PacketHeader m_curPacketHeader;  // Most recent packet header.

    bool m_bEOS;
};


DWORD ReadVideoSequenceHeader(_In_reads_bytes_(cbData) const BYTE *pData, DWORD cbData, MPEG1VideoSeqHeader &seqHeader);

DWORD ReadAudioFrameHeader(const BYTE *pData, DWORD cbData, MPEG1AudioFrameHeader &audioHeader);
