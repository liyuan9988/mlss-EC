#if !defined(AFX_PUBSHEET_H__979A86F8_50E2_11D5_91D0_0090CC01008D__INCLUDED_)
#define AFX_PUBSHEET_H__979A86F8_50E2_11D5_91D0_0090CC01008D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  

#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
    #define __int64 __int64_t
    #define RETURN 10
    #define NULLWORD 0
    #define DOUBLEQUOTE 34

    #include <pthread.h>
    #define HANDLE pthread_t
    #define CloseHandle pthread_detach
    #include <Accelerate/Accelerate.h>
    #define DeleteFile remove
#else
    #include <windows.h>
    #define NULLWORD '\0'
    #define RETURN '\n'
    #define DOUBLEQUOTE '\"'
#endif

#ifdef _WIN32
enum col_typeAA{
	aType = '0',	//any
    cType = 'c',	//charactar, long ëŒâû
	lType = 'l',	//long
	dType = 'd',    //double
    oType = 'o'     //ordered charactar, long ëŒâû
};
#else
enum col_typeAA{
    aType = '0',	//any
    CType = 'c',	//charactar, long ëŒâû
    lType = 'l',	//long
    dType = 'd',    //double
    oType = 'o'     //ordered charactar, long ëŒâû
};
#define cType CType
#endif


class PubSheet;
class PubHashTable;

class PubSheet
{
public:
	long m_nrow;
	long m_ncol;
	char** m_titles;
	char*  m_colType;

	long*     m_nStr;
	char***   m_str_col_info;
	long**    m_lng_col_info;
	double**  m_dbl_col_info;

	PubHashTable** m_hash_table;
	long*	  m_offset;

public:
	PubSheet();
	PubSheet(char* file_name);
	virtual ~PubSheet();

	void set_dim_infoCore(long nRow, char* colType);
	void set_dim_info(long nRow, char* colType, char** title);
	void set_dim_info(long nRow, char* colType);

	//åoå±ï™ïz
	static long select_by_score(long nScore, double* scoreL);
	//àÍólóêêîê∂ê¨
	static void make_runif(long nItem, double lowerB, double upperB, double* ans);
	static void make_runif(long nItem, long lowerB, long upperB, long* ans);

	void write_one_cell(long row, long col, double content);
	int	 dump_to_text_file(char* fileName, char* format=0);
	int  dump_to_text_file(FILE* fp, char* format=0);
	int	 read_from_text_file(char* fileName, char* colType);
	int  read_from_text_file(FILE* fp, char* colType, long nRow=0);

	long	get_index_by_col_name(char* colName);
	double	get_one_dbl(long row, long col);
	long	get_one_lng_with_cast(long row, long col);
	char*	get_one_str(long row, long col, char* buffer);
	long	get_one_lng_without_cast(long row, long col);
    
	double	get_one_dbl(long row, char* colName, double defaultV);
	long	get_one_lng_with_cast(long row, char* colName, long defaultV);
	char*	get_one_str(long row, char* colName, char* buffer, char* defauleV);
	long	get_one_lng_without_cast(long row,char* colName, long defauleV);
    
    double	get_one_dbl_by_name(long row, char* colName);
	long	get_one_lng_with_cast_by_name(long row, char* colName);
	char*	get_one_str_by_name(long row, char* colName, char* buffer);
	long	get_one_lng_without_cast_by_name(long row,char* colName);
    
    char*   get_one_str_by_cat_id(long col,long cat_id,char* buffer);

	int	 save_to_text_file(FILE* fp, char* format);
	int	 load_from_text_file(FILE* fp);
	void	zero_clear();
};

class PubHashTable
{
public:
	long   m_size;      
    long   m_counter;   
    char** m_objList;	
public:
	PubHashTable(long nItem);
	~PubHashTable(void);
public:
	void   resize_hTable();
public:
    long  data_size();			  
    long  search(char* content);  
    long  add(char* content);	  
};


#include <math.h>
#ifdef _WIN32
#else
double abs(double a);
#endif

#define ONE_ROW_SIZE 10000000
#define PI 3.14159265358979323846264338327950288419716939937510

#endif // !defined(AFX_PUBSHEET_H__979A86F8_50E2_11D5_91D0_0090CC01008D__INCLUDED_)
