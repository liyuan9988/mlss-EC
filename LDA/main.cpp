#include <iostream>
#include "PubSheet.h"
#include "LDA.h"
int main(int argc, const char * argv[]) {
    PubSheet* train = new PubSheet;
    char* coltype = new char[9220];
    long i;
    for (i=0; i<9219; i++) {
        coltype[i]=dType;
    }
    coltype[9219]=NULLWORD;
    train->read_from_text_file("FILE-TO-LDA-INPUT", coltype);
    long col_id[9219];
    for (i=0; i<9219; i++) {
        col_id[i]=i;
    }
    PubSheet* option = new PubSheet;
    option->read_from_text_file("FILE-TO-LDA-OPTION","ddllll");
    
    LDA* mdl = new LDA();
    mdl->set_params(option);
    mdl->train(train);
    
    mdl->m_doc_prob->dump_to_text_file("FILE-TO-LDA-OUTPUT1");
    mdl->m_word_prob->dump_to_text_file("FILE-TO-LDA-OUTPUT2");
    mdl->m_topic_prob->dump_to_text_file("FILE-TO-LDA-OUTPUT3");
    delete [] coltype;
    return 0;
}
