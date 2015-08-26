import col_names
import os

def get_user_and_item_name(whole_view_smat,whole_order_smat):
    user_name = {}
    item_name = {}
    user_count= 0
    item_count = 0
    for k,v in whole_view_smat.items():
        if(not user_name.has_key(k[0])):
            user_name[k[0]] = user_count
            user_count += 1
        if(not item_name.has_key(k[1])):
            item_name[k[1]] = item_count
            item_count += 1
    
    for k,v in whole_order_smat.items():
        if(not user_name.has_key(k[0])):
            user_name[k[0]] = user_count
            user_count += 1
        if(not item_name.has_key(k[1])):
            item_name[k[1]] = item_count
            item_count += 1
    
    return user_name,item_name

def get_item_name():
    f=open("item-0331-0409.tsv")
    item_name = {}
    for line_id,content in enumerate(f):
        if(line_id==0):
            continue
        content = content[:-1] if (content[-1]=="\n") else content
        content = content.split("\t")
        item_name[content[0]] = line_id-1
    f.close()
    return item_name

def get_item_order_smat(flag):
    os.chdir("PATH-TO=WORK-DIR")
    if(flag=="train"):
        fp=open("datatmp/train_order_log.tsv","r")
    elif (flag == "test"):
        fp=open("datatmp/test_order_log.tsv","r")
    else:
        fp=open("datatmp/orderlog.tsv","r")
    mat = dict()
    data = []
    for row,line in enumerate(fp):
        if(row!=0):
            if(line[-1]=="\n"):
                line=line[:-1]
            tmp = line.split("\t")
            data.append(tmp)
    fp.close()
    for i in data:
        item_list = i[col_names.ORDER_ITEM_IDS]
        item_list = item_list.split("/")
        item_list = item_list[1:]
        for item_id in item_list:
            key = (i[col_names.USER_ID],item_id)
            if(mat.has_key(key)):
                mat[key]+=1
            else:
                mat[key]=1
    return mat

def get_item_view_smat(flag):
    os.chdir("PATH-TO=WORK-DIR")
    if(flag=="train"):
        fp=open("datatmp/train_item_log.tsv","r")
    elif (flag == "test"):
        fp=open("datatmp/test_item_log.tsv","r")
    else:
        fp=open("datatmp/itemlog.tsv","r")
        
    mat = dict()
    data = []
    for row,line in enumerate(fp):
        if(row!=0):
            if(line[-1]=="\n"):
                line=line[:-1]
            tmp = line.split("\t")
            data.append(tmp)
    fp.close()
    for i in data:
        key = (i[col_names.USER_ID],i[col_names.ITEM_ID][1:])
        if(mat.has_key(key)):
            mat[key]+=1
        else:
            mat[key]=1
    return mat
