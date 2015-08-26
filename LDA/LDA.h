//
//  LDA.h
//  IntegratedModel
//
//  Created by liyuan xu on 2014/10/29.
//  Copyright (c) 2014年 liyuan xu. All rights reserved.
//

#ifndef __IntegratedModel__LDA__
#define __IntegratedModel__LDA__

#include <stdio.h>

class PubSheet;
class FileWriter;
class FileReader;

class LDA{
public:
    double m_alpha;
    double m_beta;
    PubSheet* m_data;
    long m_nTopic;
public:
    long m_nSample;
    long m_burn_in;
    long m_record_period;
public:
    long m_nWords;
    long m_nDoc;
    long** m_doc_word_topic_allocate;
    // map [doc][words] to topic 0 to m_nTopic-1, -1 if absent
    
    double* m_whole_topic_count;
    double** m_doc_topic_count;
    double** m_word_topic_count;
    
    double* m_acc_whole_topic_count;
    double** m_acc_doc_topic_count;
    double** m_acc_word_topic_count;
public:
    PubSheet* m_word_prob;
    PubSheet* m_doc_prob;
    PubSheet* m_topic_prob;
public:
    double* m_prob_tmp;
public:
    LDA();
    ~LDA();
public:
    void set_params(PubSheet* option);
    void init_vars();
    void init_count_matrix();
public:
    void train(PubSheet* data);
    PubSheet* predict(PubSheet* data);
    void calculate_prob();
    void get_result();
public:
    //void dump_to_binary_file(FileWriter* fp);
    void dump_to_text_file(FILE* fp);
    //void read_from_binary_file(FileReader* fp);
    void read_from_text_file(FILE* fp);
public:
    void gibbs_sampling();
    void iteration_in_gibbs(long doc_id,long word_id);
    void record_to_acc();
public:
    
};

#endif /* defined(__IntegratedModel__LDA__) */
