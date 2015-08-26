import numpy as np
from sklearn.ensemble import GradientBoostingClassifier
from sklearn.ensemble import RandomForestClassifier
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import roc_auc_score
from sklearn.cross_validation import train_test_split
from sklearn.ensemble import ExtraTreesClassifier

data = np.loadtxt("train_082101.csv",delimiter = "\t")
varX = data[:,1:]
varY = data[:,0]
clf =GradientBoostingClassifier(n_estimators=500,max_depth=1)
TEST_SIZE = 0.2
RANDOM_STATE = 0
X_train, X_val, y_train, y_val = train_test_split(varX, varY, test_size=TEST_SIZE, random_state=RANDOM_STATE)
clf.fit(X_train,y_train)
y_pred = clf.predict_proba(X_val)[:,1]
print(roc_auc_score(y_val,y_pred))

test = np.loadtxt("test_082101.csv",delimiter="\t")
clf.fit(varX,varY)
pred = clf.predict_proba(test)[:,1]
np.savetxt("prediction_082102.csv",pred,delimiter=",")
