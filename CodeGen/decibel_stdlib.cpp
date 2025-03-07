#include <bits/stdc++.h>
#include <SFML/Audio.hpp>
#include <cmath>
#include <cstdlib>
#include <cstring>
using namespace std;

#define STANDARD_SAMPLE_RATE 44100 
#define PI 3.14159265358979323846

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

unsigned int find_data_chunk(vector<char> fileBytes){
    unsigned int curr_ind = 36;
    while (curr_ind < fileBytes.size()) {
        if (fileBytes[curr_ind] == 'd' && fileBytes[curr_ind+1] == 'a' && fileBytes[curr_ind+2]=='t' && fileBytes[curr_ind+3]=='a'){
            return curr_ind+4;
        } else {
            curr_ind += readValue(fileBytes, curr_ind+4, 4) + 8;
        }
    }
    return curr_ind+4;
}

// Audio functions
extern "C"{

char* string_concat(char* str1, char* str2){
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    char* new_str = (char*)malloc(sizeof(char)*(len1+len2));
    strcat(new_str, str1);
    strcat(new_str, str2);
    return new_str;
}

struct Audio load_audio(char* filename) {
    ifstream file(filename, ios::binary);
    vector<char> fileBytes((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    unsigned int channel_count = readValue(fileBytes, 22, 2);
    unsigned int sample_rate = readValue(fileBytes, 24, 4);
    unsigned int block_alignment = readValue(fileBytes, 32, 2);
    unsigned int bits_per_sample = readValue(fileBytes, 34, 2);
    unsigned int bytes_per_sample = bits_per_sample / 8;
    unsigned int data_chunk_start = find_data_chunk(fileBytes);
    unsigned int payload_size = readValue(fileBytes, data_chunk_start, 4);
    unsigned int block_count = payload_size/block_alignment;


    // cout << channel_count << " " << block_alignment << " " << block_count << " "  << payload_size << " " << bytes_per_sample << " "<< endl;

    if(sample_rate != STANDARD_SAMPLE_RATE) {
        cerr << filename <<" is currently not supported by decibel" << endl;
        return Audio();
    }

    unsigned int* ptr = (unsigned int*)malloc(sizeof(unsigned int)*block_count);
    
    for(int i=0;i<block_count;i++)
    {
        unsigned int left_data = readValue(fileBytes, data_chunk_start+4 + i*block_alignment, bytes_per_sample);
        unsigned int right_data;
        if (channel_count!=1) {
            right_data = readValue(fileBytes, data_chunk_start+4 + i*block_alignment + bytes_per_sample, bytes_per_sample);
        } else {
            right_data = left_data;
        }
        int left_data_signed;
        int rignt_data_signed;
        if (left_data > ((1 << bytes_per_sample) - 1)){
            left_data |= ((1 << (4-bytes_per_sample)) - 1) << bytes_per_sample;
        }
        if (right_data > ((1 << bytes_per_sample) - 1)){
            right_data |= ((1 << (4-bytes_per_sample)) - 1) << bytes_per_sample;
        }
        left_data_signed = *(int*)(&left_data);
        rignt_data_signed= *(int*)(&right_data);
        left_data_signed /= 1<<(bytes_per_sample*8 - 16);
        rignt_data_signed /= 1<<(bytes_per_sample*8 - 16);
        unsigned int left_data_unsigend = *(unsigned int*)(&left_data_signed);
        unsigned int right_data_unsigend = *(unsigned int*)(&rignt_data_signed);
        ptr[i] = (left_data_signed & ((1<<16) - 1)) + (rignt_data_signed << 16);
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
    sf::Sound sound(buffer);
    sound.play();

    cout << "Playing audio" << endl;
    while (sound.getStatus() == sf::Sound::Playing) {
        sf::sleep(sf::milliseconds(100));
    }

    remove(temp_file);
}

void free_audio(struct Audio audio_var) {
    free(audio_var.ptr);
}

struct Audio concat_audio(struct Audio first_audio, struct Audio second_audio) {
    unsigned int first_audio_length = first_audio.length;
    unsigned int second_audio_length = second_audio.length;

    struct Audio new_audio;
    new_audio.length = first_audio.length + second_audio.length;
    new_audio.ptr = (unsigned int*)malloc(sizeof(unsigned int) * new_audio.length);

    for(int i=0;i<first_audio_length;i++)
    {
        new_audio.ptr[i] = first_audio.ptr[i];
    }
    for(int i=0;i<second_audio_length;i++)
    {
        new_audio.ptr[first_audio_length + i] = second_audio.ptr[i];
    }

    return new_audio;
}

struct Audio slice_audio(struct Audio audio_var, double start_time_seconds, double end_time_seconds) {
    struct Audio new_audio;

    if (start_time_seconds > end_time_seconds 
        || end_time_seconds < 0 
        || start_time_seconds*STANDARD_SAMPLE_RATE > audio_var.length ) {
        return new_audio;
    }

    unsigned int start_index;
    unsigned int end_index;
    unsigned int new_length = 0;
    unsigned int index = 0;

    start_index = (unsigned int)((int)max((double)0, start_time_seconds*STANDARD_SAMPLE_RATE) + 1);
    end_index = (unsigned int)(min((double)(audio_var.length), end_time_seconds*STANDARD_SAMPLE_RATE));

    if (start_time_seconds < 0) {
        new_length += abs((int)(start_time_seconds*STANDARD_SAMPLE_RATE));
    }
    new_length += (end_index - start_index + 1);
    if (end_time_seconds*STANDARD_SAMPLE_RATE > audio_var.length) {
        new_length += abs(end_time_seconds*STANDARD_SAMPLE_RATE - audio_var.length);
    }

    new_audio.length = new_length;
    new_audio.ptr = (unsigned int*)malloc(sizeof(unsigned int) * new_length);

    if (start_time_seconds < 0) {
        for(unsigned int i=0;i<abs(start_time_seconds*STANDARD_SAMPLE_RATE);i++) {
            new_audio.ptr[index++] = 0;
        }
    }
    for(unsigned int i=start_index;i<=end_index;i++)
    {
        new_audio.ptr[index++] = audio_var.ptr[i-1];
    }
    if (end_time_seconds*STANDARD_SAMPLE_RATE > audio_var.length) {
        for(unsigned int i=0;i<abs(end_time_seconds*STANDARD_SAMPLE_RATE - audio_var.length);i++) {
            new_audio.ptr[index++] = 0;
        }
    }
    // cout << new_audio.length << " " << end_index << endl;

    return new_audio;
}

struct Audio repeat_audio(struct Audio audio_var, double times) {
    struct Audio new_audio;
    unsigned int old_length = audio_var.length;

    if (times <=0) {
        return new_audio;
    }

    new_audio.length = (unsigned int)old_length * times;
    new_audio.ptr = (unsigned int*)malloc(sizeof(unsigned int) * new_audio.length);

    unsigned int index = 0;

    for(int i=0;i<new_audio.length;i++) {
        new_audio.ptr[i] = audio_var.ptr[index++];
        if(index == old_length) {
            index = 0;
        }
    }

    return new_audio;
}

struct Audio superimpose_audio(struct Audio audio_var1, struct Audio audio_var2) {
    struct Audio new_audio;
    unsigned int index = 0;
    unsigned short temp;

    new_audio.length = max(audio_var1.length, audio_var2.length);
    new_audio.ptr = (unsigned int*)malloc(sizeof(unsigned int) * new_audio.length);

    for(int i=0;i<min(audio_var1.length, audio_var2.length);i++) {
        short first_left = (short)(audio_var1.ptr[i] & 0xFFFF);
        short first_right = (short)((audio_var1.ptr[i] >> 16) & 0xFFFF);
        short second_left = (short)(audio_var2.ptr[i] & 0xFFFF);
        short second_right = (short)((audio_var2.ptr[i] >> 16) & 0xFFFF);
        int final_left = first_left + second_left;
        if (final_left > INT16_MAX) final_left = INT16_MAX;
        if (final_left < INT16_MIN) final_left = INT16_MIN;
        int final_right = first_right + second_right;
        if (final_right > INT16_MAX) final_right = INT16_MAX;
        if (final_right < INT16_MIN) final_right = INT16_MIN;
        new_audio.ptr[index++] = ((unsigned short)final_left & 0xFFFF) | (((unsigned short)final_right & 0xFFFF) << 16);
    }

    while(index < new_audio.length) {
        if (audio_var1.length > index) {
            new_audio.ptr[index] = audio_var1.ptr[index];
        } else {
            new_audio.ptr[index] = audio_var2.ptr[index];
        }
        index++;
    }
    return new_audio;
}
struct Audio scale_audio_static(struct Audio audio_var1, double scaling_factor) {
    struct Audio new_audio;
    unsigned int index = 0;
    unsigned short temp;

    new_audio.length = audio_var1.length;
    new_audio.ptr = (unsigned int*)malloc(sizeof(unsigned int) * new_audio.length);

    for(int i=0;i<audio_var1.length;i++) {
        short first_left = (short)(audio_var1.ptr[i] & 0xFFFF);
        short first_right = (short)((audio_var1.ptr[i] >> 16) & 0xFFFF);
        int final_left = first_left * scaling_factor;
        if (final_left > INT16_MAX) final_left = INT16_MAX;
        if (final_left < INT16_MIN) final_left = INT16_MIN;
        int final_right = first_right * scaling_factor;
        if (final_right > INT16_MAX) final_right = INT16_MAX;
        if (final_right < INT16_MIN) final_right = INT16_MIN;
        new_audio.ptr[index++] = ((unsigned short)final_left & 0xFFFF) | (((unsigned short)final_right & 0xFFFF) << 16);
    }
    return new_audio;
}

struct Audio scale_audio_dynamic(struct Audio audio_var1, double (*scaling_factor)(double)) {
    struct Audio new_audio;
    unsigned int index = 0;
    unsigned short temp;

    new_audio.length = audio_var1.length;
    new_audio.ptr = (unsigned int*)malloc(sizeof(unsigned int) * new_audio.length);

    // cout << "Hello" << endl;
    // cout << scaling_factor(4) << endl;

    for(int i=0;i<audio_var1.length;i++) {
        double time = ((double)i)/(double)STANDARD_SAMPLE_RATE;
        short first_left = (short)(audio_var1.ptr[i] & 0xFFFF);
        short first_right = (short)((audio_var1.ptr[i] >> 16) & 0xFFFF);
        int final_left = first_left * scaling_factor(time);
        if (final_left > INT16_MAX) final_left = INT16_MAX;
        if (final_left < INT16_MIN) final_left = INT16_MIN;
        int final_right = first_right * scaling_factor(time);
        if (final_right > INT16_MAX) final_right = INT16_MAX;
        if (final_right < INT16_MIN) final_right = INT16_MIN;
        new_audio.ptr[index++] = ((unsigned short)final_left & 0xFFFF) | (((unsigned short)final_right & 0xFFFF) << 16);
    }
    return new_audio;
}

struct Audio generate_audio_static(short(*wav_function)(double), double frequency_semitones, double length_seconds){
    struct Audio new_audio;
    unsigned int index = 0;
    unsigned short temp;

    new_audio.length = length_seconds*STANDARD_SAMPLE_RATE;
    new_audio.ptr = (unsigned int*)malloc(sizeof(unsigned int) * new_audio.length);

    // cout << "Hello" << endl;
    // cout << scaling_factor(4) << endl;
    
    double phase = 0;
    double prev_time = 0;
    double frequency = 440.0*pow(2, frequency_semitones/12.0);

    for(int i=0;i<new_audio.length;i++) {
        double time = ((double)i)/(double)STANDARD_SAMPLE_RATE;
        int val = wav_function(phase - (double)((long long)(phase)));
        phase += frequency*(time-prev_time);
        prev_time = time;

        int final_left = val;
        if (final_left > INT16_MAX) final_left = INT16_MAX;
        if (final_left < INT16_MIN) final_left = INT16_MIN;
        int final_right = val;
        if (final_right > INT16_MAX) final_right = INT16_MAX;
        if (final_right < INT16_MIN) final_right = INT16_MIN;
        new_audio.ptr[index++] = ((unsigned short)final_left & 0xFFFF) | (((unsigned short)final_right & 0xFFFF) << 16);
    }
    return new_audio;
}

struct Audio generate_audio_dynamic(short(*wav_function)(double), double(*frequency_semitones)(double), double length_seconds){
    struct Audio new_audio;
    unsigned int index = 0;
    unsigned short temp;

    new_audio.length = length_seconds*STANDARD_SAMPLE_RATE;
    new_audio.ptr = (unsigned int*)malloc(sizeof(unsigned int) * new_audio.length);

    // cout << "Hello" << endl;
    // cout << scaling_factor(4) << endl;
    
    double phase = 0;
    double prev_time = 0;

    for(int i=0;i<new_audio.length;i++) {
        double time = ((double)i)/(double)STANDARD_SAMPLE_RATE;
        int val = wav_function(phase - (double)((long long)(phase)));
        double frequency = 440.0*pow(2, frequency_semitones(time)/12.0);
        phase += frequency*(time-prev_time);
        prev_time = time;

        int final_left = val;
        if (final_left > INT16_MAX) final_left = INT16_MAX;
        if (final_left < INT16_MIN) final_left = INT16_MIN;
        int final_right = val;
        if (final_right > INT16_MAX) final_right = INT16_MAX;
        if (final_right < INT16_MIN) final_right = INT16_MIN;
        new_audio.ptr[index++] = ((unsigned short)final_left & 0xFFFF) | (((unsigned short)final_right & 0xFFFF) << 16);
    }
    return new_audio;
}
struct Audio pan_audio_static(struct Audio audio_var1, double panning) {
    struct Audio new_audio;
    unsigned int index = 0;
    unsigned short temp;

    new_audio.length = audio_var1.length;
    new_audio.ptr = (unsigned int*)malloc(sizeof(unsigned int) * new_audio.length);

    double multiplier = 1.0/sin(PI/4.0);
    double left_gain = cos((1+panning)*PI/4.0);
    double right_gain = sin((1+panning)*PI/4.0);
    left_gain *= multiplier;
    right_gain *= multiplier;

    // cout << left_gain << " " << right_gain << endl;
    // cout << panning << endl;

    for(int i=0;i<audio_var1.length;i++) {
        short first_left = (short)(audio_var1.ptr[i] & 0xFFFF);
        short first_right = (short)((audio_var1.ptr[i] >> 16) & 0xFFFF);
        int final_left = first_left * left_gain;
        if (final_left > INT16_MAX) final_left = INT16_MAX;
        if (final_left < INT16_MIN) final_left = INT16_MIN;
        int final_right = first_right * right_gain;
        if (final_right > INT16_MAX) final_right = INT16_MAX;
        if (final_right < INT16_MIN) final_right = INT16_MIN;
        new_audio.ptr[index++] = ((unsigned short)final_left & 0xFFFF) | (((unsigned short)final_right & 0xFFFF) << 16);
    }
    return new_audio;
}

struct Audio pan_audio_dynamic(struct Audio audio_var1, double (*panning)(double)) {
    struct Audio new_audio;
    unsigned int index = 0;
    unsigned short temp;

    new_audio.length = audio_var1.length;
    new_audio.ptr = (unsigned int*)malloc(sizeof(unsigned int) * new_audio.length);
    double multiplier = 1.0/sin(PI/4.0);

    // cout << "Hello" << endl;
    // cout << scaling_factor(4) << endl;

    for(int i=0;i<audio_var1.length;i++) {
        double time = ((double)i)/(double)STANDARD_SAMPLE_RATE;
        short first_left = (short)(audio_var1.ptr[i] & 0xFFFF);
        short first_right = (short)((audio_var1.ptr[i] >> 16) & 0xFFFF);
        double left_gain = cos((1+panning(time))*PI/4.0);
        double right_gain = sin((1+panning(time))*PI/4.0);
        left_gain *= multiplier;
        right_gain *= multiplier;
        int final_left = first_left * left_gain;
        if (final_left > INT16_MAX) final_left = INT16_MAX;
        if (final_left < INT16_MIN) final_left = INT16_MIN;
        int final_right = first_right * right_gain;
        if (final_right > INT16_MAX) final_right = INT16_MAX;
        if (final_right < INT16_MIN) final_right = INT16_MIN;
        new_audio.ptr[index++] = ((unsigned short)final_left & 0xFFFF) | (((unsigned short)final_right & 0xFFFF) << 16);
    }
    return new_audio;
}

}

