#ifndef __MYFILE_H__
#define __MYFILE_H__

#include "variation.h"
typedef struct name_value
{
	char name[32];
	char value[64];
}MyArray;

char *file_get_one(char *s, int count, const char *filename);
int file_set_one(const char *s, const char *filename);
int file_get_array(MyArray *array, int num, const char *filename);
int file_set_array(const MyArray *array, int num, const char *filename);
int clear_file(char *filename);
int delete_line(char* filename,char* temfilename,char* compareData,int len);
int get_num_from_id(char inverter_ids[MAXINVERTERCOUNT][13]);
int insert_line(char * filename,char *str);
int search_line(char* filename,char* compareData,int len);
int get_protection_from_file(const char pro_name[][32],float *pro_value,int *pro_flag,int num);
#endif	/*__MYFILE_H__*/
