#include "localutil.h"
#include <pmsis.h>
#include "fs.h"
#include "hostfs.h"
#include "hyperflash.h"


int predict_float (void * array, int n_classes){

    // Declare word list, determine recognized keyword
    // 'silence,unknown,yes,no,up,down,left,right,on,off,stop,go,'
    float supraunitary = 0;
    float subunitary = 0;
    int supra_idx = 0;
    int sub_idx = 0;
    char prediction[10];
    for (int i = 0; i < n_classes; i++){
        printf ("d[%i] = %f\n", i, ((float*) array)[i]);

        if (((float *) array)[i] > supraunitary && ((float *) array)[i] > 1){
            supraunitary = ((float *) array)[i];
            supra_idx = i;
        }
        if (((float *) array)[i] < subunitary && ((float *) array)[i] < 1){
            subunitary = ((float *) array)[i];
            sub_idx = i;
        }

    }


    int idx;

    if (subunitary > 0)
        idx = sub_idx;
    else
        idx = supra_idx;

    switch (idx){
        case 0:
            strncpy(prediction, "silence", 10);
            break;
        case 1:
            strncpy(prediction, "unknown", 10);
            break;
        case 2:
            strncpy(prediction, "yes", 10);
            break;
        case 3:
            strncpy(prediction, "no", 10);
            break;
        case 4:
            strncpy(prediction, "up", 10);
            break;
        case 5:
            strncpy(prediction, "down", 10);
            break;
        case 6:
            strncpy(prediction, "left", 10);
            break;
        case 7:
            strncpy(prediction, "right", 10);
            break;
        case 8:
            strncpy(prediction, "on", 10);
            break;
        case 9:
            strncpy(prediction, "off", 10);
            break;
        case 10:
            strncpy(prediction, "stop", 10);
            break;
        case 11:
            strncpy(prediction, "go", 10);
            break;
        default:
            printf ("Undefined class!\n");
    }

    printf("The uttered keyword was: %s (%i).\n", prediction, idx);
    return idx;
}


int predict (void * l2_buffer, int n_classes){

    // Declare word list, determine recognized keyword
    // 'silence,unknown,yes,no,up,down,left,right,on,off,stop,go,'
    int max_val = -40000;
    int max_idx = 0;
    char prediction[10];
    for (int i = 0; i < n_classes; i++){
        printf ("d[%i] = %i\n", i, ((int*) l2_buffer)[i]);

        if (((int *) l2_buffer)[i] > max_val){
            max_val = ((int*) l2_buffer)[i];
            max_idx = i;
        }
    }

    switch (max_idx){
        case 0:
            strncpy(prediction, "silence", 10);
            break;
        case 1:
            strncpy(prediction, "unknown", 10);
            break;
        case 2:
            strncpy(prediction, "yes", 10);
            break;
        case 3:
            strncpy(prediction, "no", 10);
            break;
        case 4:
            strncpy(prediction, "up", 10);
            break;
        case 5:
            strncpy(prediction, "down", 10);
            break;
        case 6:
            strncpy(prediction, "left", 10);
            break;
        case 7:
            strncpy(prediction, "right", 10);
            break;
        case 8:
            strncpy(prediction, "on", 10);
            break;
        case 9:
            strncpy(prediction, "off", 10);
            break;
        case 10:
            strncpy(prediction, "stop", 10);
            break;
        case 11:
            strncpy(prediction, "go", 10);
            break;
        default:
            printf ("Undefined class!\n");
    }

    printf("The uttered keyword was: %s (%i).\n", prediction, max_idx);
    return max_idx;
}

void dump_wav_open(char *filename, int width, int sampling_rate, int nb_channels, int size)
{
    unsigned int idx = 0;
    unsigned int sz = WAV_HEADER_SIZE + size;

    // 4 bytes "RIFF"
    header_buffer[idx++] = 'R';
    header_buffer[idx++] = 'I';
    header_buffer[idx++] = 'F';
    header_buffer[idx++] = 'F';

    // 4 bytes File size - 8bytes 32kS 0x10024 - 65408S 0x1ff24
    //header_buffer[idx++] = 0x24;
    //header_buffer[idx++] = 0xff;
    //header_buffer[idx++] = 0x01;
    //header_buffer[idx++] = 0x00;
    header_buffer[idx++] = (unsigned char) (sz & 0x000000ff);
    header_buffer[idx++] = (unsigned char)((sz & 0x0000ff00) >> 8);
    header_buffer[idx++] = (unsigned char)((sz & 0x00ff0000) >> 16);
    header_buffer[idx++] = (unsigned char)((sz & 0xff000000) >> 24);

    // 4 bytes file type: "WAVE"
    header_buffer[idx++] = 'W';
    header_buffer[idx++] = 'A';
    header_buffer[idx++] = 'V';
    header_buffer[idx++] = 'E';

    // 4 bytes format chunk: "fmt " last char is trailing NULL
    header_buffer[idx++] = 'f';
    header_buffer[idx++] = 'm';
    header_buffer[idx++] = 't';
    header_buffer[idx++] = ' ';

    // 4 bytes length of format data below, until data part
    header_buffer[idx++] = 0x10;
    header_buffer[idx++] = 0x00;
    header_buffer[idx++] = 0x00;
    header_buffer[idx++] = 0x00;

    // 2 bytes type of format: 1 (PCM)
    header_buffer[idx++] = 0x01;
    header_buffer[idx++] = 0x00;

    // 2 bytes nb of channels: 1 or 2
    //header_buffer[idx++] = 0x02;
    //header_buffer[idx++] = 0x01;
    header_buffer[idx++] = nb_channels;
    header_buffer[idx++] = 0x00;

    // 4 bytes sample rate in Hz:
    header_buffer[idx++] = (sampling_rate >> 0) & 0xff;
    header_buffer[idx++] = (sampling_rate >> 8) & 0xff;
    header_buffer[idx++] = (sampling_rate >> 16) & 0xff;
    header_buffer[idx++] = (sampling_rate >> 24) & 0xff;

    // 4 bytes (Sample Rate * BitsPerSample * Channels) / 8:
    // (8000*16*1)/8=0x3e80 * 2
    // (16000*16*1)/8=32000 or 0x6F00
    // (22050*16*1)/8=0xac44
    // (22050*16*2)/8=0x15888
    int rate = (sampling_rate * width * nb_channels) / 8;
    header_buffer[idx++] = (rate >> 0) & 0xff;
    header_buffer[idx++] = (rate >> 8) & 0xff;
    header_buffer[idx++] = (rate >> 16) & 0xff;
    header_buffer[idx++] = (rate >> 24) & 0xff;

    // 2 bytes (BitsPerSample * Channels) / 8:
    // 16*1/8=2 - 16b mono
    // 16*2/8=4 - 16b stereo
    rate = (width * nb_channels) / 8;
    header_buffer[idx++] = (rate >> 0) & 0xff;
    header_buffer[idx++] = (rate >> 8) & 0xff;

    // 2 bytes bit per sample:
    header_buffer[idx++] = width;
    header_buffer[idx++] = 0x00;

    // 4 bytes "data" chunk
    header_buffer[idx++] = 'd';
    header_buffer[idx++] = 'a';
    header_buffer[idx++] = 't';
    header_buffer[idx++] = 'a';

    // 4 bytes size of data section in bytes:
    header_buffer[idx++] = (unsigned char) (size & 0x000000ff);
    header_buffer[idx++] = (unsigned char)((size & 0x0000ff00) >> 8);
    header_buffer[idx++] = (unsigned char)((size & 0x00ff0000) >> 16);
    header_buffer[idx++] = (unsigned char)((size & 0xff000000) >> 24);

    struct pi_hostfs_conf conf;
    pi_hostfs_conf_init(&conf);

    pi_open_from_conf(&fs_wav, &conf);

    if (pi_fs_mount(&fs_wav))
     return;

    wavfile = pi_fs_open(&fs_wav, filename, PI_FS_FLAGS_WRITE);
    if (wavfile == 0)
    {
        printf("Failed to open file, %s\n", filename);
        return;
    }

    pi_fs_write(wavfile, header_buffer, WAV_HEADER_SIZE);
}

void dump_wav_write(void *data, int size)
{
    pi_fs_write(wavfile, data, size);
}


void dump_wav_close()
{
    pi_fs_close(wavfile);

    pi_fs_unmount(&fs_wav);
}

// void dump_data_write(char *filename, void *data, int size)
// {

//     static struct pi_device fs_data;
//     static pi_fs_file_t * file_data;
//     struct pi_hostfs_conf conf;

//     pi_hostfs_conf_init(&conf);
//     pi_open_from_conf(&fs_data, &conf);
//     if (pi_fs_mount(&fs_data))
//      return;

//     file_data = pi_fs_open(&fs_data, filename, PI_FS_FLAGS_WRITE);
//     if (file_data == 0)
//     {
//         printf("Failed to open file_data, %s\n", filename);
//         return;
//     }

//     pi_fs_write(file_data, data,  size);
//     pi_fs_close(file_data);
//     pi_fs_unmount(&fs_data);
// }
