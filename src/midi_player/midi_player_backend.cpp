/*
    This is the midi parser
    - Emit midi events
*/

#include <fluidsynth.h>
#include <unistd.h>
#include <string>
#include <vector>

uint16_t Swap16Endian(uint16_t value);
uint16_t Swap32Endian(uint16_t value);

struct __attribute__((packed)) MidiHeaderChunk
{
    uint32_t headerChunkId;
    uint32_t headerChunkSize;
    uint16_t headerFormatType;
    uint16_t headerNumTracks;
    uint16_t headerTimeDivision;

    void LoadHeader(const uint8_t* buffer)
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
    MidiEvent* events;
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

    headerChunk.LoadHeader(buffer);

    return 0;
}

uint16_t Swap16Endian(uint16_t value)
{
    return ((value & 0x00FF) << 8) | ((value >> 8));
}

uint16_t Swap32Endian(uint16_t value)
{
    return ((value & 0x00FF) << 8) | ((value >> 8));
}