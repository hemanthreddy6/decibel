#include <bits/stdc++.h>
#include <SFML/Audio.hpp>
using namespace std;

#define STANDARD_SAMPLE_RATE 22050

// Command to install SFML: sudo apt install libopenal-dev libsndfile1-dev libsfml-dev
// Compilation flags: -lsfml-audio -lsfml-system

// Struct to store audio file
struct Audio {
    unsigned long long length;
    unsigned int* ptr;

    Audio() {
        this->length = 0;
    }

    Audio(unsigned long long l) {
        this->length = l;
        ptr = (unsigned int*)malloc(sizeof(unsigned int)*l);
    }
};

// Helper functions
unsigned int readValue(vector<char>&file, unsigned int start, unsigned int count) {
    unsigned int value = 0;
    for(unsigned int i=0;i<count;i++)
    {
        value += (static_cast<u_int8_t>(file[start+i]) << (i * 8));
    }
    return value;
}

void writeValue(vector<char>&file, unsigned int value, unsigned int count) {
    for(unsigned int i=0;i<count;i++)
    {
        file.push_back((char)(value & 0xFF));
        value >>= 8;
    }
}

// Audio functions
extern "C"{
struct Audio load_audio(char* filename) {
    ifstream file(filename, ios::binary);
    vector<char> fileBytes((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    unsigned int channel_count = readValue(fileBytes, 22, 2);
    unsigned int sample_rate = readValue(fileBytes, 24, 4);
    unsigned int block_alignment = readValue(fileBytes, 32, 2);
    unsigned int bits_per_sample = readValue(fileBytes, 34, 2);
    unsigned int bytes_per_sample = bits_per_sample / 8;
    unsigned int payload_size = readValue(fileBytes, 40, 4);
    unsigned int block_count = payload_size/block_alignment;

    if(sample_rate != STANDARD_SAMPLE_RATE) {
        cerr << filename <<" is currently not supported by decibel" << endl;
        return Audio();
    }

    unsigned int* ptr = (unsigned int*)malloc(sizeof(unsigned int)*block_count);
    
    for(int i=0;i<block_count;i++)
    {
        unsigned int data = readValue(fileBytes, 44 + i*block_alignment, bytes_per_sample);
        if (channel_count!=1) {
            data += (readValue(fileBytes, 44 + i*block_alignment + bytes_per_sample, bytes_per_sample) << (bytes_per_sample * 8));
        } else {
            data += (readValue(fileBytes, 44 + i*block_alignment, bytes_per_sample) << (bytes_per_sample * 8));
        }
        ptr[i] = data;
    }

    struct Audio audio;
    audio.length = block_count;
    audio.ptr = ptr;

    return audio;
}

void save_audio(struct Audio audio_var, char* filename) {
    unsigned int channel_count = 2;
    unsigned int block_count = audio_var.length;
    unsigned int bytes_per_sample = 2;
    unsigned int first_chunk_size = 36 + bytes_per_sample*channel_count*block_count;
    unsigned int second_chunk_size = 16;
    unsigned int format_tag = 1;
    unsigned int samples_per_sec = STANDARD_SAMPLE_RATE;
    unsigned int block_alignment = bytes_per_sample * channel_count;
    unsigned int bytes_per_sec = samples_per_sec * block_alignment;
    unsigned int bits_per_sample = (bytes_per_sample * 8);
    unsigned int third_chunk_size = block_alignment * block_count;
    
    vector<char>outBytes;

    // ckID
    outBytes.push_back('R');
    outBytes.push_back('I');
    outBytes.push_back('F');
    outBytes.push_back('F');

    // cksize
    writeValue(outBytes, first_chunk_size, 4);

    // WAVEID
    outBytes.push_back('W');
    outBytes.push_back('A');
    outBytes.push_back('V');
    outBytes.push_back('E');

    // ckID
    outBytes.push_back('f');
    outBytes.push_back('m');
    outBytes.push_back('t');
    outBytes.push_back(' ');

    // cksize
    writeValue(outBytes, second_chunk_size, 4);

    // wFormatTag
    writeValue(outBytes, format_tag, 2);

    // nChannels
    writeValue(outBytes, channel_count, 2);

    // nSamplesPerSec
    writeValue(outBytes, samples_per_sec, 4);

    // nAvgBytesPerSec
    writeValue(outBytes, bytes_per_sec, 4);

    // nBlockAlign
    writeValue(outBytes, block_alignment, 2);

    // wBitsPerSample
    writeValue(outBytes, bits_per_sample, 2);

    // ckID
    outBytes.push_back('d');
    outBytes.push_back('a');
    outBytes.push_back('t');
    outBytes.push_back('a');

    // cksize
    writeValue(outBytes, third_chunk_size, 4);

    // sampled data
    for(int i=0;i<block_count;i++)
    {
        writeValue(outBytes, (audio_var.ptr[i] & ((1<<bits_per_sample)-1)), bytes_per_sample);
        writeValue(outBytes, (audio_var.ptr[i] >> bits_per_sample), bytes_per_sample);
    }

    ofstream outFile(filename, ios::binary);
    outFile.write(outBytes.data(), outBytes.size());
}

void play_audio(struct Audio audio_var) {
    char* temp_file = (char*)"decibel_temp.wav";

    save_audio(audio_var, temp_file);

    sf::SoundBuffer buffer;
    buffer.loadFromFile(temp_file);

    sf::Sound sound;
    sound.setBuffer(buffer);
    sound.play();

    cout << "Playing audio" << endl;
    while (sound.getStatus() == sf::Sound::Playing) {
        sf::sleep(sf::milliseconds(100));
    }

    remove(temp_file);
}
}
