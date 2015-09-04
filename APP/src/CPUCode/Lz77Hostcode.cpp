#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fstream>
#include "Maxfiles.h"
#include "MaxSLiCInterface.h"

const int WINDOW_SIZE   = Lz77Compress_WINDOW_SIZE;
const int MAX_UNCODED   = Lz77Compress_MAX_UNCODED;
const int HASH_SIZE     = Lz77Compress_HASH_SIZE;

int main(void)
{
    // replicate the file to increase data size
    int file_size_replication = 1;
    
    printf("Program starts\n");
    bool status = true;
    std::ifstream in("file_input");
    std::string contents((std::istreambuf_iterator<char>(in)), 
    std::istreambuf_iterator<char>());
    
    // truncate the input length because LMEM requires that the data size is a multiple of 384
    int len_input = (contents.length() / 384) * 384; 
    const char* char_inputs = contents.c_str();
   
    int len_origin = len_input;
    len_input *= file_size_replication;

    printf("Input Length: %d bytes\n",len_input); 
    uint8_t* inputs = new uint8_t[len_input]; 
    FILE* input = fopen("trunc_input","wb");    
    for(int i = 0; i < len_input; i++)
    {
        inputs[i] = char_inputs[i % len_origin];      
        fputc(inputs[i],input);
    }
    fclose(input);
    
    // initialize output
    int len_output = len_input;
    uint32_t*   output_data;
    uint8_t*    output_len;
    output_data = new uint32_t[len_output*3/2];
    output_len  = new uint8_t[len_output*3/2];
    memset(output_data, 0, sizeof(output_data));
    memset(output_len,  0, sizeof(output_len));
    
    printf("-- Running dfe\n");
    struct timeval start, end;
    // Send the input to LMEM
    gettimeofday( &start, NULL ); 
    
    Lz77Compress_WriteLmem(len_input, inputs);
    
    gettimeofday( &end, NULL );
    int load_time1 = (1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec -start.tv_usec) / 1000;
    
    // Run the DFE to get outputs
    gettimeofday( &start, NULL ); 
    
    Lz77Compress(len_input);
    
    gettimeofday( &end, NULL );
    int run_time = (1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec -start.tv_usec) / 1000;
    
    // Get the output from LMEM
    gettimeofday( &start, NULL);

    Lz77Compress_ReadLmem(len_input, output_data, output_len);
    
    gettimeofday( &end, NULL );
    int load_time2 = (1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec -start.tv_usec) / 1000;
    fprintf(stderr, "-- \tDFE load time: %d ms\n", load_time1 + load_time2);
    fprintf(stderr, "-- \tDFE run time: %d ms  DFE bandwidth: %d KB/s\n", run_time, (len_input / run_time));
    len_output = len_input;
      
    // CPUcodes that organize the DFE outputs and output as file
    printf("-- Collect DFE outputs\n");
    gettimeofday( &start, NULL );
    FILE* fp_out = fopen("dfe_encode","wb");
    if(fp_out == NULL)
        printf("Error when opening file!");
    int sum_length = 0;
    int bit_count = 0;
    unsigned char bit_buffer = 0;
    for(int i = 0; i < len_output; i++)
    {
        sum_length += output_len[i];
        int len         = output_len[i];
        uint32_t data   = output_data[i];
        if(!len)
            continue;
        if(len + bit_count >= 8)
        {
            fputc(bit_buffer << (8 - bit_count) | data >> (len - (8 - bit_count) ) ,fp_out); 
            len -= (8 - bit_count);
            bit_count = 0;
        }
        while(len >= 8)
        {
            fputc(data >> (len - 8), fp_out);
            len -= 8;
        } 
        bit_buffer  = data;
        bit_count   = len;
    }
    if(bit_count != 0)
        fputc(bit_buffer << (8 - bit_count), fp_out);
    fclose(fp_out); 
    
    gettimeofday( &end, NULL );
    int timeuse = (1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec) / 1000;
    fprintf(stderr,"-- \tCollect time: %d ms\n", timeuse);
    fprintf(stderr, "-- \tDFE overall speed: %d KB/s", (len_input/(load_time1 + run_time + load_time2 + timeuse)));

    printf("-- Decoding using CPU code\n");
    system("../../ORIG/software/Lz77Compress -d -i dfe_encode -o dfe_decode");
    
    printf("-- Checking the decoded output\n"); 
    std::ifstream in2("dfe_decode");
    std::string decode_contents((std::istreambuf_iterator<char>(in2)), 
    std::istreambuf_iterator<char>());
    const char* decode_results = decode_contents.c_str();
    int error_cnt = 0;
    if((int)decode_contents.length() != len_input)
    {   
        fprintf(stderr, "failed! decode size should be %d instead of %d\n", len_input, (int)decode_contents.length());
    }   
    else
    {   
        for(int i = 0; i < (int)decode_contents.length(); i++)
        {   
            if((uint8_t)decode_results[i] != inputs[i])
            {   
                fprintf(stderr,"\tCheck failed at index %d, should be %d instead of %d \n", i, inputs[i], (uint8_t)(decode_results[i]));
                status = false;
                error_cnt += 1;
            }   
        }   
    }  

    printf("-- Encode using CPU\n");
    gettimeofday( &start, NULL );

    system("../../ORIG/software/Lz77Compress -c -i trunc_input -o cpu_encode");
    
    gettimeofday( &end, NULL );
    timeuse = (1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec -start.tv_usec) / 1000;
    fprintf(stderr,"-- \tCPU encode time: %d ms\n", timeuse);
    
    struct stat input_s;
    struct stat cpu_s; 
    struct stat dfe_s;
    stat("input", &input_s);
    stat("cpu_encode", &cpu_s); 
    stat("dfe_encode", &dfe_s);

    fprintf(stderr,"INPUT length: %d bytes\n", (int)input_s.st_size);
    fprintf(stderr,"DFE encode length: %d bytes\n", (int)dfe_s.st_size);
    fprintf(stderr,"CPU encode length: %d bytes\n", (int)cpu_s.st_size);
    
    if(status)
    {
        fprintf(stderr,"DECODE SUCCESSFULLY!\n");
    }
    else
    {
        fprintf(stderr,"!!!!!!!!Oops, decode failed");
    }
    printf("Program ends\n");
    
    return 0;
}
