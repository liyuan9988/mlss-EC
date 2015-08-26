#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <process.h>
#endif

#include "PubSheet.h"

#ifdef _WIN32
#else
#include <Accelerate/Accelerate.h>
double abs(double a){return (a>0)? a:-a;}
#endif

__int64
split_string(char* buffer, char**& result)
{
	if(buffer[strlen(buffer)-1]== RETURN) buffer[strlen(buffer)-1]= NULLWORD;
	__int64 i, j, n=strlen(buffer)+1; 
	__int64 nItem=0;
	for(i=0; i<n; i++){
		if(buffer[i]==',' || buffer[i]==NULLWORD) nItem++;
	}
	result=new char*[nItem+1];
	j=0; 
	char* p=buffer;
	for(i=0; i<n; i++){
		if(buffer[i]==',' || buffer[i]==NULLWORD){
		  buffer[i]=NULLWORD;
		  result[j]=new char[strlen(p)+1];
		  strcpy(result[j],p);
		  j++; p=buffer+i+1;
		}
	}
	return nItem;
}

void
PubSheet::zero_clear()
{
	m_nrow=0;
	m_ncol=0;
	m_titles=0;
	m_colType=0;

	m_nStr=0;
	m_str_col_info=0;
	m_lng_col_info=0;
	m_dbl_col_info=0;

	m_hash_table=0;
	m_offset=0;
}

PubSheet::PubSheet()
{
	zero_clear();
}

PubSheet::~PubSheet()
{
#ifdef _DEBUG
	pub_pop_alloc(m_alloc_seqNo);
#endif

	long i,j;
	if(!m_titles) return;
	if(!m_nStr) return;

	for(i=0; i<m_ncol; i++){
		if(m_str_col_info[i]){
			for(j=0; j<m_nStr[i]; j++){
				delete [] m_str_col_info[i][j];
			}
			delete [] m_str_col_info[i];
		}
		if(m_lng_col_info[i]) delete [] m_lng_col_info[i];
		if(m_dbl_col_info[i]) delete [] m_dbl_col_info[i];
	}

	if(m_offset) delete [] m_offset;
	if(m_hash_table){
		for(i=0; i<m_ncol; i++){
			if(m_colType[i]==cType){
				if(m_hash_table[i]) delete m_hash_table[i];
			}
		}
		delete [] m_hash_table;
	}

	for(i=0; i<m_ncol; i++){
		delete [] m_titles[i];
	}
	delete [] m_titles;
	delete [] m_colType;
	delete [] m_str_col_info;
	delete [] m_lng_col_info;
	delete [] m_dbl_col_info;
	delete [] m_nStr;
}


void
PubSheet::set_dim_infoCore(long nRow, char* colType)
{
	long i;
	m_nrow=nRow;
	m_ncol=strlen(colType);
	m_colType=new char[strlen(colType)+1];
	strcpy(m_colType,colType);
	for(i=0; i<m_ncol; i++){
		if(colType[i]==cType){
		}else if(colType[i]==lType){
		}else if(colType[i]==dType){
		}else if(colType[i]==oType){
			exit(1);
		}
	}
	m_hash_table=new PubHashTable*[m_ncol];
	for(i=0; i<m_ncol; i++) m_hash_table[i]=0;

	m_nStr=new long[m_ncol];
	m_str_col_info=new char**[m_ncol];
	m_lng_col_info=new long*[m_ncol];
	m_dbl_col_info=new double*[m_ncol];
	for(i=0; i<m_ncol; i++){
		m_nStr[i]=0;
		m_str_col_info[i]=0;
		m_lng_col_info[i]=0;
		m_dbl_col_info[i]=0;
	}

	m_offset=new long[m_ncol];
	long offset=0;
	for(i=0; i<m_ncol; i++){
		if(m_colType[i]==cType){
			m_hash_table[i]=new PubHashTable(m_nrow);
			m_str_col_info[i]=new char*[m_nrow];
			m_offset[i]=offset; offset+=sizeof(long);
			m_lng_col_info[i]=new long[m_nrow];
		}else if(m_colType[i]==lType){
			m_offset[i]=offset; offset+=sizeof(long);
			m_lng_col_info[i]=new long[m_nrow];
		}else if(m_colType[i]==oType){
			m_offset[i]=offset; offset+=sizeof(long);
			m_lng_col_info[i]=new long[m_nrow];
		}else if(m_colType[i]==dType){
			m_offset[i]=offset; offset+=sizeof(double);
			m_dbl_col_info[i]=new double[m_nrow];
		}
	}
}

void
PubSheet::set_dim_info(long nRow, char* colType)
{
	long i;
	set_dim_infoCore(nRow, colType);
	m_titles=new char*[m_ncol];
	for(i=0; i<m_ncol; i++){
		m_titles[i]=new char[20];
		sprintf(m_titles[i],"A%d",i+1);
	}
}

void 
PubSheet::set_dim_info(long nRow, char* colType, char** title)
{
	long i;
	set_dim_infoCore(nRow, colType);
	m_titles=new char*[m_ncol];
	for(i=0; i<m_ncol; i++){
		m_titles[i]=new char[strlen(title[i])+1];
		strcpy(m_titles[i],title[i]);
	}
}

void
PubSheet::write_one_cell(long row, long col, double content)
{
	m_dbl_col_info[col][row]=content;
}

int	 
PubSheet::save_to_text_file(FILE* fp, char* format)
{
	fprintf(fp,"colType=%s\n",m_colType);
	fprintf(fp,"nRow=%d\n",m_nrow);
	dump_to_text_file(fp,format); 
	return 0;
}

int	 
PubSheet::dump_to_text_file(char* fileName, char* format)
{
	FILE* fp=fopen(fileName,"w");
	if(!fp){
		exit(1);
	}
	int ans=dump_to_text_file(fp,format);
	fclose(fp);
	return ans;
}

int	 
PubSheet::dump_to_text_file(FILE* fp, char* format)
{
	long i,j;
	char oneBufferRow[ONE_ROW_SIZE];
	for(i=0; i<m_ncol; i++){
		if(i>0) fprintf(fp,",");
		fprintf(fp,"%s",m_titles[i]);
	}
	fprintf(fp,"\n");
	for(i=0; i<m_nrow; i++){
		for(j=0; j<m_ncol; j++){
			if(j>0) fprintf(fp,",");
			if(m_colType[j]==cType || m_colType[j]==oType){
				fprintf(fp,"%s",m_str_col_info[j][m_lng_col_info[j][i]]);
			}else if(m_colType[j]==dType){
				if(format){
					fprintf(fp,format,m_dbl_col_info[j][i]);
				}else{
					fprintf(fp,"%lf",m_dbl_col_info[j][i]);
				}
			}else if(m_colType[j]==lType){
				fprintf(fp,"%ld",m_lng_col_info[j][i]);
			}
		}
		fprintf(fp,"\n");
	}
	return 0;
}

int
PubSheet::read_from_text_file(char* fileName, char* colType)
{
    FILE* fp=fopen(fileName,"r");
    if(!fp){
        printf("DSheet::File(%s) Not cannot exists",fileName);
        exit(1);
    }
    int ans=read_from_text_file(fp, colType);
    fclose(fp);
    return ans;
}

int	 
PubSheet::read_from_text_file(FILE* fp, char* colType, long nRow)
{
	long i,j,k;
	char** oneBufferRow;
	char* buffer=new char[ONE_ROW_SIZE];

	fgets(buffer,ONE_ROW_SIZE-1,fp);
	m_ncol=split_string(buffer, oneBufferRow);
	if(m_ncol!=strlen(colType)){
		exit(1);
	}

	m_colType=new char[strlen(colType)+1];
	strcpy(m_colType,colType);

	m_titles=new char*[m_ncol];
	for(i=0; i<m_ncol; i++){
		m_titles[i]=new char[strlen(oneBufferRow[i])+1];
		strcpy(m_titles[i], oneBufferRow[i]);
	}

	if(nRow==0){
#ifdef _WIN32
		__int64 position=_ftelli64( fp );
		m_nrow=0;
		while(fgets(buffer,ONE_ROW_SIZE-1,fp)!=NULL) m_nrow++;
		_fseeki64(fp, position, SEEK_SET);
#else
		__int64 position=ftell( fp );
		m_nrow=0;
		while(fgets(buffer,ONE_ROW_SIZE-1,fp)!=NULL) m_nrow++;
		fseek(fp, position, SEEK_SET);
#endif
	}else{
		m_nrow=nRow;
	}

	m_str_col_info=new char**[m_ncol];
	m_lng_col_info=new long*[m_ncol];
	m_dbl_col_info=new double*[m_ncol];
	m_nStr=new long[m_ncol];
    
	PubHashTable** hashTable=new PubHashTable*[m_ncol];
    m_offset=new long[m_ncol];
	long offset = 0;
	for(i=0; i<m_ncol; i++){
		m_nStr[i]=0;
		m_str_col_info[i]=0;
		m_lng_col_info[i]=0;
		m_dbl_col_info[i]=0;
		hashTable[i]=0;
		if(m_colType[i]==cType || m_colType[i]==oType){
			m_str_col_info[i]=new char*[m_nrow];
			m_lng_col_info[i]=new long[m_nrow];
			hashTable[i]=new PubHashTable(m_nrow);
            m_offset[i]=offset; offset+=sizeof(long);
		}else if(m_colType[i]==dType){
			m_dbl_col_info[i]=new double[m_nrow];
            m_offset[i]=offset; offset+=sizeof(double);
		}else if(m_colType[i]==lType){
			m_lng_col_info[i]=new long[m_nrow];
            m_offset[i]=offset; offset+=sizeof(long);
        }
	}

	for(i=0; i<m_nrow; i++){
		fgets(buffer,ONE_ROW_SIZE-1,fp);
		long n=split_string(buffer, oneBufferRow);
		if(n!=m_ncol){
			exit(1);
		}
		for(j=0; j<m_ncol; j++){
			if(m_colType[j]==cType || m_colType[j]==oType){
				long key=hashTable[j]->add(oneBufferRow[j]);
				if(m_nStr[j]<=key){
					if(m_nStr[j]!=key){
						exit(1);
					}
					m_str_col_info[j][key]=new char[strlen(oneBufferRow[j])+1];
					strcpy(m_str_col_info[j][key],oneBufferRow[j]);
					m_nStr[j]++;
				}
				m_lng_col_info[j][i]=key;
			}else if(m_colType[j]==dType){
				m_dbl_col_info[j][i]=atof(oneBufferRow[j]);
			}else{
				m_lng_col_info[j][i]=atol(oneBufferRow[j]);
			}
		}
	}
	delete [] buffer;

	return 0;
}
int
PubSheet::load_from_text_file(FILE* fp)
{
	char buff[ONE_ROW_SIZE];
	char colType[ONE_ROW_SIZE];

	fgets(buff,ONE_ROW_SIZE-1,fp);
	if(buff[strlen(buff)-1]=='\n') buff[strlen(buff)-1]=NULLWORD;
	strcpy(colType,buff+strlen("colType="));
	fgets(buff,ONE_ROW_SIZE-1,fp);
	if(buff[strlen(buff)-1]=='\n') buff[strlen(buff)-1]=NULLWORD;
	long nRow=atol(buff+strlen("nRow="));
	this->read_from_text_file(fp,colType,nRow); 
	return 0;
}


double
oneRnd()
{
	return ((double)rand())/RAND_MAX;
}

//一様乱数生成
void 
PubSheet::make_runif(long nItem, double lowerB, double upperB, double* ans)
{
	double tmp;
	if(lowerB>upperB){
	    tmp = lowerB;
	    lowerB=upperB;
	    upperB=tmp;
	}
	double range=upperB-lowerB;
	for(long i=0; i<nItem; i++){
		ans[i]=oneRnd()*range+lowerB;
	}
}

void 
PubSheet::make_runif(long nItem, long lowerB, long upperB, long* ans)
{
	long tmp;
	if(lowerB>upperB){
	    tmp=lowerB;
	    lowerB=upperB;
	    upperB=tmp;
	}
	long range=upperB-lowerB;

	lowerB=(lowerB<0)? (lowerB-1):lowerB;
	for(long i=0; i<nItem; i++){
	    tmp=(long)(oneRnd()*range+lowerB);
		while(tmp==upperB) tmp=(long)(oneRnd()*range+lowerB);
	    ans[i]=tmp;
	}
}

//経験分布
long
PubSheet::select_by_score(long nScore, double* scoreL)
{
	long i;
	double sumV=0.0;
	for(i=0; i<nScore; i++){
		sumV+=scoreL[i];
	}
	double tmp;
	PubSheet::make_runif(1,0.0,sumV,&tmp);
	for(i=0; i<nScore; i++){
		tmp-=scoreL[i];
		if(tmp<=0.0) return i;
	}
	return nScore-1;
}

long
PubSheet::get_index_by_col_name(char* colName)
{
	long i;
	for(i=0; i<m_ncol; i++){
		if(!strcmp(m_titles[i],colName)){
			return i;
		}
	}
	return -1;
}

double
PubSheet::get_one_dbl_by_name(long row, char* colName)
{
    long idx=get_index_by_col_name(colName);
    if (idx<0) {
        exit(1);
    }
	return get_one_dbl(row,idx);
}

double	
PubSheet::get_one_dbl(long row, char* colName, double defaultV)
{
	long idx=get_index_by_col_name(colName);
	return (idx<0)? defaultV:get_one_dbl(row,idx);
}

double	
PubSheet::get_one_dbl(long row, long col)
{
	if(col<0 || col>=m_ncol) return -HUGE_VAL;
	if(row<0 || row>=m_nrow) return -HUGE_VAL;

	if(m_colType[col]==cType || m_colType[col]==oType){
		return atof(m_str_col_info[col][m_lng_col_info[col][row]]);
	}else if(m_colType[col]==dType){
		return m_dbl_col_info[col][row];
	}else if(m_colType[col]==lType){
		return (double)(m_lng_col_info[col][row]);
	}
	return -HUGE_VAL;
}

long
PubSheet::get_one_lng_with_cast_by_name(long row, char* colName)
{
    long idx=get_index_by_col_name(colName);
	if (idx<0) {
        exit(1);
    }
    return get_one_lng_with_cast(row,idx);
}

long
PubSheet::get_one_lng_with_cast(long row, char* colName, long defaultV)
{
	long idx=get_index_by_col_name(colName);
	return (idx<0)? defaultV:get_one_lng_with_cast(row,idx);
}

long
PubSheet::get_one_lng_with_cast(long row, long col)
{
	if(col<0 || col>=m_ncol) return LONG_MIN;
	if(row<0 || row>=m_nrow) return LONG_MIN;

	if(m_colType[col]==cType || m_colType[col]==oType){
		return atol(m_str_col_info[col][m_lng_col_info[col][row]]);
	}else if(m_colType[col]==dType){
		return (long)m_dbl_col_info[col][row];
	}else if(m_colType[col]==lType){
		return m_lng_col_info[col][row];
	}
	return LONG_MIN;
}

long
PubSheet::get_one_lng_without_cast_by_name(long row,char* colName)
{
    long idx=get_index_by_col_name(colName);
    if (idx<0) {
        exit(1);
    }
	return get_one_lng_without_cast(row,idx);
}


long
PubSheet::get_one_lng_without_cast(long row, char* colName, long defaultV)
{
	long idx=get_index_by_col_name(colName);
	return (idx<0)? defaultV:get_one_lng_without_cast(row,idx);
}

long
PubSheet::get_one_lng_without_cast(long row, long col)
{
	if(col<0 || col>=m_ncol) return LONG_MIN;
	if(row<0 || row>=m_nrow) return LONG_MIN;
	if(m_colType[col]==dType){
		return LONG_MIN;
	}else{
		return m_lng_col_info[col][row];
	}
}
char*
PubSheet::get_one_str_by_name(long row, char* colName, char* buffer)
{
    long idx=get_index_by_col_name(colName);
	if (idx<0) {
        exit(1);
    }
    return get_one_str(row, idx, buffer);
}

char*
PubSheet::get_one_str(long row, char* colName,  char* buffer, char* defaultV)
{
	long idx=get_index_by_col_name(colName);
	if(idx<0){
		strcpy(buffer,defaultV);
		return defaultV;
	}else{
		return get_one_str(row, idx, buffer);
	}
}

char*	
PubSheet::get_one_str(long row, long col, char* buffer)
{
	if(col<0 || col>=m_ncol) return 0;
	if(row<0 || row>=m_nrow) return 0;

	if(m_colType[col]==cType || m_colType[col]==oType){
		strcpy(buffer,m_str_col_info[col][m_lng_col_info[col][row]]);
		return buffer;
	}else if(m_colType[col]==dType){
		sprintf(buffer,"%f",m_dbl_col_info[col][row]);
		return buffer;
	}else if(m_colType[col]==lType){
		sprintf(buffer,"%l",m_lng_col_info[col][row]);
		return buffer;
	}
	return 0;
}

char*
PubSheet::get_one_str_by_cat_id(long col,long cat_id,char* buffer)
{
    if(col<0 || col>=m_ncol) return 0;
	if(cat_id<0 || cat_id >=m_nStr[col]) return 0;
    
	if(m_colType[col]==cType || m_colType[col]==oType){
		strcpy(buffer,m_str_col_info[col][cat_id]);
		return buffer;
	}
	return 0;
}

PubHashTable::PubHashTable(long nItem)
{
	m_size=nItem;      // Hashtableサイズ
    m_counter=0;
    m_objList=new char*[m_size];	// 内容そのもの
	for(long i=0; i<m_size; i++) m_objList[i]=0;
}

PubHashTable::~PubHashTable(void)
{
	for(long i=0; i<m_size; i++){
		if(m_objList[i]) delete m_objList[i];
	}
    delete [] m_objList;
}

long 
PubHashTable::data_size()
{
	return m_counter;
}

//コンテンツによるkeyの検索、存在しなければ、-1 を返す
long  
PubHashTable::search(char* content)
{
	long i;
	for(i=0; i<m_counter; i++){
		if(!strcmp(content, m_objList[i])) return i;
	}
	return -1;
}

void   
PubHashTable::resize_hTable()
{
	if(m_counter>=m_size){
		long newSize=m_size*2;
		long i;
		char** newBody=new char*[newSize];
		for(i=0; i<m_counter; i++){
			newBody[i]=m_objList[i];
		}
		for(i=m_counter; i<newSize; i++) newBody[i]=0;
		delete [] m_objList;
		m_objList=newBody;
		m_size=newSize;
	}
}
//内容保存(あれば、keyを返すのみ、なければ、追加して、新しいKeyを返す）
long  
PubHashTable::add(char* content)
{
	long  ans=search(content);
	if(ans>=0) return ans;

	resize_hTable();
	m_objList[m_counter]=new char[strlen(content)+1];
	strcpy(m_objList[m_counter],content);
	m_counter++;
	return m_counter-1;
}

