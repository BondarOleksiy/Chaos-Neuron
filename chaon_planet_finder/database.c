#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <conio.h>
#include "database.h"
#include "fft.h"

#define MAX_FILE_PATH 200

#define FITS_HEADER_BLOCK_SIZE 2880
#define EXT_OFFSET 5760

int cmpfunc(const void * a, const void * b)
{
  if (*(double*)a > *(double*)b)
    return 1;
  else if (*(double*)a < *(double*)b)
    return -1;
  else
    return 0;  
}

double find_array_median(double *array, int size)
{
	double median=0;
	double *sorted_array = (double*)malloc(size*sizeof(double));
	
	int real_data_size=0;
	
	for(int i=0;i<size;i++)
	{
		if(!isnan(array[i]))
		{
			sorted_array[real_data_size] = array[i];
			real_data_size++;
		}
	}
	
	qsort(sorted_array,real_data_size,sizeof(double),cmpfunc);
	
	if(size%2==0) median = (sorted_array[real_data_size/2]+sorted_array[(real_data_size/2)-1])/2;
	else median = sorted_array[real_data_size/2];
	
	free(sorted_array);
	return(median);
}

int fits_read_data_size(char * fits_path)
{
	FILE *fp;
	if ((fp=fopen(fits_path, "rb"))==NULL)
	{
		printf ("Cannot open file.\n");
		printf ("%s",fits_path);
		exit(1);
	}

	fseek(fp, EXT_OFFSET, SEEK_SET);
	char buf[FITS_HEADER_BLOCK_SIZE];
	char naxis2_str[50];
	int cursor=0;

	int uncompressed_data_size=0;

	fread(&buf, sizeof(buf), 1, fp);

	for(int i=0;i<sizeof(buf);i++)
	{
		if(buf[i]=='N')
		{
			if(buf[i+1]=='A'&&buf[i+2]=='X'&&buf[i+3]=='I'&&buf[i+4]=='S'&&buf[i+5]=='2')
			{
				while(buf[i]!='=')i++;
				i++;
				while(buf[i]!='/')
				{
					naxis2_str[cursor]=buf[i];
					i++;
					cursor++;
				}
				uncompressed_data_size = atoi(naxis2_str);
			}
		}
	}
	fclose(fp);
	return(uncompressed_data_size);
}

void change_endiannes(char *data,int bytes_num)
{
	char *ptr;
	int i=0;
	
	ptr = (char*)malloc(bytes_num);
	
	for(i=0;i<bytes_num;i++)
	{
		ptr[i]=data[bytes_num-1-i];
	}
	
	memcpy(data, ptr, bytes_num);
	
	free(ptr);
}

#define TABLE_OFFSET 0x00004EC0

pattern *get_pattern(char *pattern_path, int pattern_size)
{
	int raw_data_size = fits_read_data_size(pattern_path);

	double *raw_data = (double*)malloc(sizeof(double)*raw_data_size);
	
	int size = sizeof(pattern) 
			 + pattern_size * sizeof(double) //data
			 + pattern_size * sizeof(double) //spectrum
			 + MAX_FILE_PATH
	;
	
	pattern *pat = (pattern*)malloc(size);
	
	pat->name = (char*)pat + sizeof(pattern);
	pat->data = (double*)(pat->name + MAX_FILE_PATH);
	pat->spectrum = pat->data + pattern_size;
	
	pat->size = pattern_size;
	strcpy(pat->name, basename(pattern_path));

	////////////////////////////////////////////file proc
	FILE *fp;
	
	if ((fp=fopen(pattern_path, "rb"))==NULL)
	{
		printf ("Cannot open file. %s\n",pattern_path);
	}
	fseek(fp, TABLE_OFFSET, SEEK_SET);
	////////////////////////////////////////////file proc
	
	char buf[100];
	float temp=0;
	
	for(int i=0;i<raw_data_size;i++)// raw data with NANs
	{
		fread(&buf, sizeof(buf), 1, fp);
		
		temp = *(float *)(buf+32);
		change_endiannes((char*)&temp, sizeof(float));
		
		raw_data[i] = (double)temp;//raw data
	}
	
	double raw_data_median = find_array_median(raw_data, raw_data_size);
	
	for(int i=0;i<raw_data_size;i++)// data imputation
	{
		if(isnan(raw_data[i]))
		raw_data[i] = raw_data_median;
	}
	
	///////////////////////////////////////////downsampling
	int frame_size = raw_data_size/pattern_size;
	int frames_num = raw_data_size/frame_size;
	int raw_cursor = 0;
	double accum=0;
	
	for(int i=0;i<pattern_size;i++)
	{
		for(int j=0;j<frame_size;j++)
		{
			accum += raw_data[raw_cursor];
			raw_cursor++;
		}
		pat->data[i] = accum/frame_size;
		accum=0;
	}
	///////////////////////////////////////////downsampling
	
	///////////////////////////////////////////shifting
	
	double min_data = pat->data[0];
	double max_data = pat->data[0];
	
	for(int i=1;i<pattern_size;i++)
	{
		if(min_data>pat->data[i])min_data=pat->data[i];	
		else if(max_data<pat->data[i])max_data=pat->data[i];	
	}
	
	double scale_factor = max_data - min_data;
	
	for(int i=0;i<pattern_size;i++)
	{
		pat->data[i] = (pat->data[i] - min_data)/scale_factor;		
	}
	
	///////////////////////////////////////////shifting
	
	///////////////////////////////////////////raw spectrum
//	int raw_array_pow = (int)log2(raw_data_size);
//	int raw_limited_size = pow(2,raw_array_pow);
//	
//	double *raw_real_data = (double*)malloc(sizeof(double)*raw_limited_size);
//	double *raw_imag_data = (double*)malloc(sizeof(double)*raw_limited_size);
//	memcpy(raw_real_data, raw_data,raw_limited_size*sizeof(double));
//	for(int i=0;i<raw_limited_size;i++)raw_imag_data[i] = 0;
//	
//	FFT(FFT_FORWARD, raw_array_pow, raw_real_data, raw_imag_data);
//	
//	for(int i=0;i<raw_limited_size;i++)
//	{
//		raw_real_data[i] = sqrt(raw_real_data[i]*raw_real_data[i] + raw_imag_data[i]*raw_imag_data[i]);
//	}
//	
//	make_fix_xls(raw_real_data, raw_limited_size, pat->name);
//	
//	free(raw_real_data);                        
//	free(raw_imag_data);
	///////////////////////////////////////////raw spectrum
	
	///////////////////////////////////////////spectrum
	double *imag_data = (double*)malloc(sizeof(double)*pat->size);
	for(int i=0;i<pat->size;i++)imag_data[i]=0;
	
	int array_pow = (int)log2(pat->size);
	
	memcpy(pat->spectrum,pat->data,pat->size*sizeof(double));
	
	FFT(FFT_FORWARD, array_pow, pat->spectrum, imag_data);
	
	for(int i=0;i<pat->size;i++)
	{
		pat->spectrum[i] = sqrt(pat->spectrum[i]*pat->spectrum[i] + imag_data[i]*imag_data[i]);	
	}
	
	free(imag_data);
	///////////////////////////////////////////spectrum
	
	///////////////////////////////////////////remove DC
	
	//pat->spectrum[0] = find_array_median(pat->spectrum, pat->size/2);
	
	///////////////////////////////////////////remove DC
	
	//make_fix_xls(pat->spectrum, pat->size/2, pat->name);
	//make_fix_xls(pat->data, pat->size, pat->name);
	
	fclose(fp);
	free(raw_data);
	
	return(pat);
}

void free_database(database *base)
{
	for(int i=0;i<base->patterns_num;i++)free(base->input[i]);
	free(base);	
}

database *init_database(char *input_patterns_dir, int pattern_size)
{
	DIR *d;
  	struct dirent *dir;
  	int files_num=0;
  	d = opendir(input_patterns_dir);
  	
  	if (d)//////////////////////////////////count patterns num in dir
	{
    	while ((dir = readdir(d)) != NULL)
		{
    		files_num++;
    	}
  	}
  	else printf("Cant open folder.\n");
	files_num = files_num - 2;// - trash
	////////////////////////////////////////count patterns num in dir	
	
	//////////////////////////////////////make new base
	int size = sizeof(database) 
			 + sizeof(double) * files_num   // outputs for each pattern
			 + sizeof(pattern*) * files_num// pointers to patterns
			 + MAX_FILE_PATH
	;
	
	database *base = (database*)malloc(size);	
	
	base->patterns_num = files_num;
	base->size = size;
	base->pattern_size = pattern_size;
	
	base->input  = (pattern*)((char*)base + sizeof(database));
	base->output = (double*)((char*)base->input + sizeof(pattern*)*base->patterns_num);
	base->path   = (char*)base->output + sizeof(double)*base->patterns_num;
	
	strcpy(base->path, input_patterns_dir);
	
	///////////////////////////////////////make new base
	
	char file_path[MAX_FILE_PATH];
	
	rewinddir(d);
  	readdir(d);
  	readdir(d);

  	for(int i=0;i<files_num;i++)
  	{
	 	dir = readdir(d);
	 	
	 	strcpy(file_path, input_patterns_dir);
		strcat(file_path, "/");
	 	strcat(file_path, dir->d_name);
	 	
		base->input[i] = get_pattern(file_path, pattern_size);
  	}
  	
	return(base);
}

void make_fix_xls(double *x, int points, char *file_name)
{
	FILE *fp;
	char buf[100];
	
	strcat(file_name,".xls");
	
	if ((fp=fopen(file_name, "wb"))==NULL)
	{
		printf("Cannot create file.\n");
	}
	
	for(int i=0;i<points;i++)
	{	
		sprintf(buf, "%f", x[i]);
		fwrite(buf, 7, 1, fp);
		fwrite("\n", sizeof(char), 1, fp);
	}
	
	fclose (fp);

    printf("xls done\n");
}
