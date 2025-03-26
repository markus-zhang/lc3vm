/*
    This is the midi parser
    - Emit midi events
*/

#include <fluidsynth.h>
#include <unistd.h>
#include <string>
#include <vector>

enum MidiEventType
{
    MIDI_EVENT_NOTE_OFF = 0x8,
    MIDI_EVENT_NOTE_ON = 0x9,
    MIDI_EVENT_NOTE_AFTERTOUCH = 0xA,
    MIDI_EVENT_CONTROLLER = 0xB,
    MIDI_EVENT_PROGRAM_CHANGE = 0xC,
    MIDI_EVENT_CHANNEL_AFTERTOUCH = 0xD,
    MIDI_EVENT_PITCH_BEND = 0xE,
    MIDI_EVENT_META = 0xFF,
    MIDI_EVENT_SYSEX_DIVIDED = 0xF0,
    MIDI_EVENT_SYSEX_AUTH = 0xF7
};

uint16_t Swap16Endian(uint16_t value);
uint32_t Swap32Endian(uint32_t value);

struct MidiEvent ReadMidiEvent(uint8_t** buffer);

struct __attribute__((packed)) MidiHeaderChunk
{
    uint32_t headerChunkId;
    uint32_t headerChunkSize;
    uint16_t headerFormatType;
    uint16_t headerNumTracks;
    uint16_t headerTimeDivision;

    uint8_t* LoadHeader(uint8_t* buffer)
    {
        headerChunkId = *((uint32_t*)buffer);
        printf("ChunkId is: 0x%x\n", headerChunkId);
        buffer += sizeof(headerChunkId);

        headerChunkSize = *((uint32_t*)buffer);
        printf("ChunkSize is: 0x%x\n", headerChunkSize);
        buffer += sizeof(headerChunkSize);

        headerFormatType = Swap16Endian(*((uint16_t*)buffer));
        printf("FormatType is: 0x%x\n", headerFormatType);
        buffer += sizeof(headerFormatType);

        headerNumTracks = Swap16Endian(*((uint16_t*)buffer));
        buffer += sizeof(headerNumTracks);
        
        /* 
            EXPLAIN:
            00 F0
            If the top bit is 0 (in this case), the rest 15 bits is time division in ticks per beat
            So in this case 000 0000 1111 0000 which is 0xF0 ticks per beat
        */

        headerTimeDivision = Swap16Endian(*((uint16_t*)buffer));

        printf("Division is: 0x%x\n", headerTimeDivision);

        if (!ValidateHeader())
        {
            fprintf(stderr, "Midi Header Validation Failed!\n");
        }

        printf("buffer pointer at: 0x%x\n", *((uint8_t*)buffer));

        return buffer;
    }

private:
    bool ValidateHeader()
    {
        if (headerChunkId != 0x6468544D)
        {
            return false;
        }
        if (headerChunkSize != 0x06000000)
        {
            return false;
        }
        if (headerFormatType < 0 || headerFormatType > 2)
        {
            return false;
        }
        /* EXPLAIN: 1-65535 */
        if (headerNumTracks == 0)
        {
            return false;
        }
        return true;
    }
};


struct __attribute__((packed)) MidiTrackChunk
{
    uint32_t trackChunkId;
    uint32_t trackChunkSize;
    // MidiEvent events[512];
    uint16_t eventCount;

    uint8_t* LoadTrack(uint8_t* buffer)
    {
        trackChunkId = *((uint32_t*)buffer);
        printf("trackChunkId is: 0x%x\n", trackChunkId);
        buffer += sizeof(trackChunkId);

        trackChunkSize = Swap32Endian(*((uint32_t*)buffer));
        printf("trackChunkSize is: 0x%x\n", trackChunkSize);
        buffer += sizeof(trackChunkSize);

        uint8_t* p = buffer;

        eventCount = 0;
        while (p < buffer + trackChunkSize)
        {
            // events[eventCount] = ReadMidiEvent(&p);
        }
        printf("Number of events: %d\n", eventCount);
    }

private:
    struct MidiEvent ReadMidiEvent(uint8_t** buffer)
    {
        /*
            buffer should point to the first byte of the first midi event (of a track chunk)
        */

        MidiEvent me;

        /*  
            EXPLAIN:
            Variable length Delta Time
            - If the top bit is 0, then this byte is the last byte
            - If the top bit is 1, then this byte is not the last byte
            - top bits are not used in calculation

            e.g. 81 48 
            This gives 10000001 01001000, so the second byte is the last byte.
            Throwing away both top bits, this gives us
            0000001 1001000 -> 11001000 which is 0xC8 = 200 in decimal

            e.g. 8C 81 48
            10001100 10000001 01001000
            ->
            1100 0000001 1001000
            Looks like the algorithm below is correct:
            - 1100 is shifted 2 * 7 bits = 14 bits
            - 1 is shifted 7 bites
            - 1001000 not shifted
            result = 110000000011001000

            byte b = read_one_byte();
            int32_t result = 0;
            while (true)
            {
                result << 7;
                result += (b & 0x7F);
                if (b.top_bit == 0)
                {
                    break;
                }
            }
        */

        uint32_t dt = 0;
        while (true)
        {
            uint8_t byte = *(*((uint8_t**)buffer));
            
            dt = dt << 7;
            dt += (byte & 0x7F);

            if ((byte & 0x80) == 0)
            {
                break;
            }

            (*buffer) += 1;
        }

        printf("Delta Time extracted as: 0x%x\n", dt);
        me.deltaTimes = dt;

        /* 
            EXPLAIN: 
            eventType (4-bit) and midiChannel (4-bit)
        */
        uint8_t nextByte = *(*((uint8_t**)buffer));
        me.eventType = nextByte >> 4;
        me.midiChannel = nextByte & 0x0F;
        (*buffer) += 1;

        /*
            EXPLAIN:
            Lots and lots of different cases...
            https://github.com/colxi/midi-parser-js/wiki/MIDI-File-Format-Specifications

            Event Type 	        Value 	    Parameter 1 	    Parameter 2
            Note Off 	        0x8 	note number 	        velocity
            Note On 	        0x9 	note number 	        velocity
            Note Aftertouch 	0xA 	note number 	        aftertouch value
            Controller 	        0xB 	controller number 	    controller value
            Program Change 	    0xC 	program number 	        not used
            Channel Aftertouch 	0xD 	aftertouch value 	    not used
            Pitch Bend 	        0xE 	pitch value (LSB) 	    pitch value (MSB
        */
        switch (me.eventType)
        {
            case (MIDI_EVENT_NOTE_OFF):
            case (MIDI_EVENT_NOTE_ON):
            case (MIDI_EVENT_NOTE_AFTERTOUCH):
            case (MIDI_EVENT_CONTROLLER):
            case (MIDI_EVENT_PITCH_BEND):
            {
                me.para1 = *(*((uint8_t**)buffer));
                (*buffer) += 1;
                me.para2 = *(*((uint8_t**)buffer));
                (*buffer) += 1;
                break;
            }
            case (MIDI_EVENT_PROGRAM_CHANGE):
            case (MIDI_EVENT_CHANNEL_AFTERTOUCH):
            {
                me.para1 = *(*((uint8_t**)buffer));
                (*buffer) += 1;
            }
        }

        return me;
    }
};


struct __attribute__((packed)) MidiEvent
{
    /*
        EXPLAIN:
        Delta Time 	    Event Type Value 	MIDI Channel 	Parameter 1 	Parameter 2
        variable-length 	4 bits 	            4 bits 	        1 byte 	        1 byte
    */
    // deltaTimaes maximum 28-bit
    uint32_t deltaTimes;
    uint8_t eventType;
    uint8_t midiChannel;
    uint8_t para1;
    uint8_t para2;
};

// class MidiPlayerBackend
// {
// public:
//     MidiPlayerBackend();
//     ~MidiPlayerBackend();

//     void Init(std::string midiPath);
// private:
//     void LoadMidiFile();

// public:
//     uint8_t* buffer;
//     uint64_t bufferSize;
// };

int main()
{
    uint8_t* buffer = (uint8_t*)malloc(65536);

    FILE* fp = fopen("./theyewtree.mid", "rb");
    if (!fp)
    {
        fprintf(stderr, "Failed to open midi file\n");
        exit(1);
    }

    size_t midiSize = fread(buffer, 1, 65536, fp);
    printf("Number of bytes read: %d\n", midiSize);

    if (midiSize <= 0)
    {
        fprintf(stderr, "Failed to read midi file\n");
        exit(2);
    }

    MidiHeaderChunk headerChunk;

    buffer = headerChunk.LoadHeader(buffer);

    uint8_t test[3] = {0x81, 0x48, 0x90};
    uint8_t* pTest = test;

    ReadMidiEvent(&pTest);

    return 0;
}

uint16_t Swap16Endian(uint16_t value)
{
    return ((value & 0x00FF) << 8) | ((value >> 8));
}

uint32_t Swap32Endian(uint32_t value)
{
    return ((value & 0xFF) << 24) | (((value >> 8) & 0xFF) << 16) | (((value >> 16) & 0xFF) << 8) | (value >> 24);
}

struct MidiEvent ReadMidiEvent(uint8_t** buffer)
{
    /*
        buffer should point to the first byte of the first midi event (of a track chunk)
    */

    MidiEvent me;

    /*  
        EXPLAIN:
        Variable length Delta Time
        - If the top bit is 0, then this byte is the last byte
        - If the top bit is 1, then this byte is not the last byte
        - top bits are not used in calculation

        e.g. 81 48 
        This gives 10000001 01001000, so the second byte is the last byte.
        Throwing away both top bits, this gives us
        0000001 1001000 -> 11001000 which is 0xC8 = 200 in decimal

        e.g. 8C 81 48
        10001100 10000001 01001000
        ->
        1100 0000001 1001000
        Looks like the algorithm below is correct:
        - 1100 is shifted 2 * 7 bits = 14 bits
        - 1 is shifted 7 bites
        - 1001000 not shifted
        result = 110000000011001000

        byte b = read_one_byte();
        int32_t result = 0;
        while (true)
        {
            result << 7;
            result += (b & 0x7F);
            if (b.top_bit == 0)
            {
                break;
            }
        }
    */

    uint32_t dt = 0;
    while (true)
    {
        uint8_t byte = *(*((uint8_t**)buffer));
        
        dt = dt << 7;
        dt += (byte & 0x7F);

        if ((byte & 0x80) == 0)
        {
            break;
        }

        (*buffer) += 1;
    }

    printf("Delta Time extracted as: 0x%x\n", dt);
    me.deltaTimes = dt;

    return me;
}