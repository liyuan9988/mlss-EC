//
//  LDA.cpp
//  IntegratedModel
//
//  Created by liyuan xu on 2014/10/29.
//  Copyright (c) 2014年 liyuan xu. All rights reserved.
//

#include "LDA.h"
#include "PubSheet.h"

LDA::LDA()
{
    m_alpha=0;
    m_beta=0;
    m_data=0;
    m_nTopic=0;
    m_nSample=0;
    m_burn_in=0;
    m_record_period = 0;
    
    m_nWords=0;
    m_nDoc=0;
    m_doc_word_topic_allocate=0;
    m_whole_topic_count=0;
    m_doc_topic_count=0;
    m_word_topic_count=0;
    
    m_acc_whole_topic_count=0;
    m_acc_doc_topic_count=0;
    m_acc_word_topic_count=0;
    
    m_word_prob = 0;
    m_doc_prob = 0;
    m_prob_tmp = 0;
    
}

LDA::~LDA()
{
    long i,j;
    
    if(m_doc_word_topic_allocate){
        for (i=0; i<m_nDoc; i++) {
            if(m_doc_word_topic_allocate[i]) delete [] m_doc_word_topic_allocate[i];
        }
        delete [] m_doc_word_topic_allocate;
    }
    
    if(m_whole_topic_count) delete [] m_whole_topic_count;
    if(m_acc_whole_topic_count) delete [] m_acc_whole_topic_count;
    
    if(m_doc_topic_count){
        for (i=0; i<m_nDoc; i++) {
            if(m_doc_topic_count[i]) delete [] m_doc_topic_count[i];
        }
        delete [] m_doc_topic_count;
    }
    
    if(m_acc_doc_topic_count){
        for (i=0; i<m_nDoc; i++) {
            if(m_acc_doc_topic_count[i]) delete [] m_acc_doc_topic_count[i];
        }
        delete [] m_acc_doc_topic_count;
    }
    
    if (m_word_topic_count) {
        for (i=0; i<m_nWords; i++) {
            if(m_word_topic_count[i]) delete [] m_word_topic_count[i];
        }
        delete [] m_word_topic_count;
    }
    
    if (m_acc_word_topic_count) {
        for (i=0; i<m_nWords; i++) {
            if(m_acc_word_topic_count[i]) delete [] m_acc_word_topic_count[i];
        }
        delete [] m_acc_word_topic_count;
    }
    if(m_word_prob) delete m_word_prob;
    if(m_doc_prob) delete m_doc_prob;
    if(m_topic_prob) delete m_topic_prob;
    if(m_prob_tmp) delete [] m_prob_tmp;
    
}

void
LDA::set_params(PubSheet* option)
{
    m_alpha = option->get_one_dbl(0, "alpha", 0.1);
    m_beta = option->get_one_dbl(0, "beta", 0.1);
    m_nTopic = option->get_one_lng_with_cast(0, "nTopic", 5);
    m_nSample = option->get_one_lng_with_cast(0, "nSample", 500);
    m_burn_in = option->get_one_lng_with_cast(0, "burn_in",100);
    m_record_period = option->get_one_lng_with_cast(0, "record_period", 1);
}

void
LDA::init_vars()
{
    long i,j;
    m_nDoc = m_data->m_nrow;
    m_nWords = m_data->m_ncol;
    
    m_doc_word_topic_allocate = new long*[m_nDoc];
    for (i=0; i<m_nDoc; i++) {
        m_doc_word_topic_allocate[i] = new long[m_nWords];
        for (j=0; j<m_nWords; j++) {
            if(abs(m_data->get_one_dbl(i, j))>1.0e-5){
                PubSheet::make_runif(1, 0, m_nTopic-1, &m_doc_word_topic_allocate[i][j]);
            }else{
                m_doc_word_topic_allocate[i][j] = -1;
            }
        }
    }
    
    
    m_whole_topic_count = new double[m_nTopic];
    m_acc_whole_topic_count = new double[m_nTopic];
    for (i=0; i<m_nTopic; i++) {
        m_whole_topic_count[i] = 0.0;
        m_acc_whole_topic_count[i] = 0.0;
    }
    
    m_word_topic_count = new double*[m_nWords];
    m_acc_word_topic_count = new double*[m_nWords];
    for (i=0; i<m_nWords; i++) {
        m_word_topic_count[i] = new double[m_nTopic];
        m_acc_word_topic_count[i] = new double[m_nTopic];
        for(j=0; j<m_nTopic; j++){
            m_word_topic_count[i][j] = 0.0;
            m_acc_word_topic_count[i][j] = 0.0;
        }
    }
    
    m_doc_topic_count = new double*[m_nDoc];
    m_acc_doc_topic_count = new double*[m_nDoc];
    for (i=0; i<m_nDoc; i++) {
        m_doc_topic_count[i] = new double[m_nTopic];
        m_acc_doc_topic_count[i] = new double[m_nTopic];
        for (j=0; j<m_nTopic; j++) {
            m_doc_topic_count[i][j] = 0.0;
            m_acc_doc_topic_count[i][j] = 0.0;
        }
    }
    init_count_matrix();
}

void
LDA::init_count_matrix()
{
    long i,j;
    long topic_id;
    double value;
    for (i=0; i<m_nDoc; i++) {
        for (j=0; j<m_nWords; j++) {
            if (m_doc_word_topic_allocate[i][j]!=-1) {
                topic_id = m_doc_word_topic_allocate[i][j];
                value = m_data->get_one_dbl(i, j);
                
                m_whole_topic_count[topic_id] += value;
                m_word_topic_count[j][topic_id] += value;
                m_doc_topic_count[i][topic_id] += value;
            }
        }
    }
}

void
LDA::gibbs_sampling()
{
    long i,j;
    for (i=0; i<m_nDoc; i++) {
        for (j=0; j<m_nWords; j++) {
            if(m_doc_word_topic_allocate[i][j] != -1){
                iteration_in_gibbs(i,j);
            }
        }
    }
    
}

void
LDA::record_to_acc()
{
    long i,j;
    for (i=0; i<m_nTopic; i++) {
        m_acc_whole_topic_count[i] += m_whole_topic_count[i];
    }
    for (i=0; i<m_nWords; i++) {
        for (j=0; j<m_nTopic; j++) {
            m_acc_word_topic_count[i][j] += m_word_topic_count[i][j];
        }
    }
    for (i=0; i<m_nDoc; i++) {
        for (j=0; j<m_nTopic; j++) {
            m_acc_doc_topic_count[i][j] += m_doc_topic_count[i][j];
        }
    }
}


void
LDA::iteration_in_gibbs(long doc_id,long word_id)
{
    long i,j;
    double value;
    long old_topic;
    if(!m_prob_tmp) m_prob_tmp = new double[m_nTopic];
    for (i=0; i<m_nTopic; i++) {
        m_prob_tmp[i] = 0.0;
    }
    if (m_doc_word_topic_allocate[doc_id][word_id]==-1) {
        printf("LDA::cal_prob_in_gibbs::topic id error");
        exit(1);
    }else{
        old_topic = m_doc_word_topic_allocate[doc_id][word_id];
        value = m_data->get_one_dbl(doc_id, word_id);
    }
    
    m_doc_topic_count[doc_id][old_topic] -= value;
    m_word_topic_count[word_id][old_topic] -= value;
    m_whole_topic_count[old_topic] -= value;
    
    for (i=0; i<m_nTopic; i++) {
        m_prob_tmp[i] = (m_doc_topic_count[doc_id][i]+m_alpha)*(m_word_topic_count[word_id][i]+m_beta)/(m_whole_topic_count[i]+m_nWords*m_beta);
    }
    long new_topic = PubSheet::select_by_score(m_nTopic, m_prob_tmp);
    m_doc_topic_count[doc_id][new_topic] += value;
    m_word_topic_count[word_id][new_topic] += value;
    m_whole_topic_count[new_topic] += value;
    m_doc_word_topic_allocate[doc_id][word_id] = new_topic;
    
}



void
LDA::train(PubSheet *data)
{
    long i,j;
    m_data = data;
    init_vars();
    for (i=0; i<m_burn_in; i++) {
        gibbs_sampling();
    }
    for (i=0; i<m_nSample; i++) {
        for (j=0; j<m_record_period; j++) {
            gibbs_sampling();
        }
        record_to_acc();
    }
    if(m_whole_topic_count){
        delete [] m_whole_topic_count;
        m_whole_topic_count = 0;
    }
    
    if(m_doc_topic_count){
        for (i=0; i<m_nDoc; i++) {
            if(m_doc_topic_count[i]) delete [] m_doc_topic_count[i];
        }
        delete [] m_doc_topic_count;
        m_doc_topic_count = 0;
    }
    
    if (m_word_topic_count) {
        for (i=0; i<m_nWords; i++) {
            if(m_word_topic_count[i]) delete [] m_word_topic_count[i];
        }
        delete [] m_word_topic_count;
        m_word_topic_count = 0;
    }
    
    get_result();
    
}

void
LDA::get_result()
{
    long i,j;
    
    for (i=0; i<m_nTopic; i++) {
        m_acc_whole_topic_count[i] /= m_nSample;
    }
    for (i=0; i<m_nWords; i++) {
        for (j=0; j<m_nTopic; j++) {
            m_acc_word_topic_count[i][j] /= m_nSample;
        }
    }
    for (i=0; i<m_nDoc; i++) {
        for (j=0; j<m_nTopic; j++) {
            m_acc_doc_topic_count[i][j] /= m_nSample;
        }
    }
    
    m_doc_prob = new PubSheet;
    char* doc_prob_coltype = new char[m_nTopic+1];
    for (i=0; i<m_nTopic; i++) {
        doc_prob_coltype[i] = 'd';
    }
    doc_prob_coltype[m_nTopic]=NULLWORD;
    //m_doc_prob->set_dim_info(m_nDoc, doc_prob_coltype,(char*)0);
	m_doc_prob->set_dim_info(m_nDoc, doc_prob_coltype);
    for (i=0; i<m_nDoc; i++) {
        double sum_tmp = 0.0;
        for (j=0; j<m_nTopic; j++) {
            m_prob_tmp[j] = m_acc_doc_topic_count[i][j]+m_alpha;
            sum_tmp += m_prob_tmp[j];
        }
        for (j=0; j<m_nTopic; j++) {
            m_doc_prob->write_one_cell(i, j, m_prob_tmp[j]/sum_tmp);
        }
    }
    delete [] doc_prob_coltype;
    
    m_word_prob = new PubSheet;
    char* word_prob_coltype = new char[m_nWords+1];
    for (i=0; i<m_nWords; i++) {
        word_prob_coltype[i] = 'd';
    }
    word_prob_coltype[m_nWords]=NULLWORD;
    m_word_prob->set_dim_info(m_nTopic, word_prob_coltype, m_data->m_titles);
    for (i=0; i<m_nWords; i++) {
        for (j=0; j<m_nTopic; j++) {
            m_word_prob->write_one_cell(j, i, (m_acc_word_topic_count[i][j]+m_beta)/(m_acc_whole_topic_count[j]+m_nWords*m_beta));
        }
    }
    delete [] word_prob_coltype;
    
    m_topic_prob = new PubSheet;
    char* topic_prob_coltype = new char[m_nTopic+1];
    for (i=0; i<m_nTopic; i++) {
        topic_prob_coltype[i] = 'd';
    }
    topic_prob_coltype[m_nTopic]=NULLWORD;
    //m_topic_prob->set_dim_info(1, topic_prob_coltype, (char*)0);
    m_topic_prob->set_dim_info(1, topic_prob_coltype);
    double sum_prob = 0;
    for(i=0; i<m_nTopic; i++){
        sum_prob += m_acc_whole_topic_count[i];
    }
    for(i=0; i<m_nTopic; i++){
        m_topic_prob->write_one_cell(0, i, m_acc_whole_topic_count[i]/sum_prob);
    }
    delete [] topic_prob_coltype;
    
    
}

PubSheet*
LDA::predict(PubSheet* data)
{
    long i,j,k;
    PubSheet* result = new PubSheet;
    char* col_type = new char[m_nTopic+1];
    if(!m_prob_tmp) m_prob_tmp=new double[m_nTopic];
    double tmp,sum_tmp;
    for (i=0; i<m_nTopic; i++) {
        col_type[i] = 'd';
    }
    col_type[m_nTopic]=NULLWORD;
    //result->set_dim_info(data->m_nrow, col_type,(char*)0);
    result->set_dim_info(data->m_nrow, col_type);
    for (i=0; i<data->m_nrow; i++) {
        sum_tmp=0.0;
        for (k=0; k<m_nTopic; k++) {
            tmp=0.0;
            for (j=0; j<data->m_ncol; j++) {
                tmp += data->get_one_dbl(i, j)*m_word_prob->get_one_dbl(k,j);
            }
            sum_tmp += tmp;
            m_prob_tmp[k] = tmp;
        }
        for (k=0; k<m_nTopic; k++) {
            result->write_one_cell(i, k, m_prob_tmp[k]/sum_tmp);
        }
    }
    return result;
}


//void
//LDA::dump_to_binary_file(FileWriter* fp)
//{
//    fp->write(&m_alpha, sizeof(double), 1);
//    m_word_prob->dump_to_binary_file(fp);
//}

void
LDA::dump_to_text_file(FILE* fp)
{
    fprintf(fp, "%lf%c",m_alpha,RETURN);
    m_word_prob->save_to_text_file(fp, 0);
}

//void
//LDA::read_from_binary_file(FileReader* fp)
//{
//    fp->read(&m_alpha, sizeof(double), 1);
//    m_word_prob = new PubSheet;
//    m_word_prob->read_from_binary_file(fp);
//    m_nTopic = m_word_prob->m_nrow;
//    m_nWords = m_word_prob->m_ncol;
//}

void
LDA::read_from_text_file(FILE* fp)
{
    char buf[ONE_ROW_SIZE];
    fgets(buf, ONE_ROW_SIZE-1, fp);
    m_alpha = atof(buf);
    m_word_prob = new PubSheet;
    m_word_prob->load_from_text_file(fp);
    m_nTopic = m_word_prob->m_nrow;
    m_nWords = m_word_prob->m_ncol;
}





