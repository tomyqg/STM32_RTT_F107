#ifndef __MYFILE_H__
#define __MYFILE_H__

typedef struct name_value
{
	char name[32];
	char value[64];
}MyArray;

char *file_get_one(char *s, int count, const char *filename);
int file_set_one(const char *s, const char *filename);
int file_get_array(MyArray *array, int num, const char *filename);
int file_set_array(const MyArray *array, int num, const char *filename);
int save_to_process_result(int cmd_id, char *savebuffer);
#endif	/*__MYFILE_H__*/
