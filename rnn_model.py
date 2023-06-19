import numpy as np
import pandas as pd
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers
import json
import os
import pdb
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import OneHotEncoder

## First thing we do is read the data in 
def load_files(path):

    flist = os.listdir(path)

    Ic = dict()
    Params = dict()

    for f in flist:
        if "_Ic.json" in f:
            with open(os.path.join(path,f) ) as fp:
                Ic[f.replace("_Ic.json", "")] = json.load(fp)
            continue
        elif "_params.json" in f:
            with open(os.path.join(path,f) ) as fp:
                Params[f.replace("_params.json", "")] = json.load(fp)
            continue
        else:
            raise ValueError("File names must have either _Ic.json or _params.json")

    if not all([f in Params.keys() for f in Ic.keys()]):
        raise ValueError("Not all params and curves present")

    Ic_v = list()
    Params_v = list()
    for i in Ic.keys():
        Ic_v.append(Ic[i])
        Params_v.append(Params[i])

    return Params_v, Ic_v

if __name__ == "__main__":
    
    Params, Ic = load_files("sims/23_06_19")

    # Pad the Ic vector with zeros so they are all the same length
    max_vsize = max([len(x) for x in Ic])
    [x.extend([0]*(max_vsize - len(x))) for x in Ic]
    X = np.array(Ic)

    ## Pull the labels
    y = pd.DataFrame(Params)[['BETA1', 'cm']]

    ## Test/Train split
    X_train, X_test, y_train, y_test = train_test_split(
            X, y, test_size=0.33, random_state=42)

    ## One-hot encode the Params vec
    enc = OneHotEncoder(handle_unknown='ignore', sparse=False)
    enc = enc.fit(y_train)
    y_train = enc.transform(y_train)
    y_test = enc.transform(y_test)
    
    pdb.set_trace() 

    ## Build the model
    input_shape = X_train.shape[1]
    model = keras.Sequential()
    model.add(layers.Input(shape=(input_shape,1,)))
    model.add(
        keras.layers.Bidirectional(
          keras.layers.LSTM(
              units=128
          )
        )
    )
    model.add(keras.layers.Dropout(rate=0.5))
    model.add(keras.layers.Dense(units=128, activation='relu'))
    model.add(keras.layers.Dense(y_train.shape[1], activation='softmax'))


    model.compile(
      loss='categorical_crossentropy',
      optimizer='adam',
      metrics=['acc']
    )


    history = model.fit(
        X_train, y_train,
        epochs=2,
        batch_size=32,
        validation_split=0.1,
        shuffle=False
    )

    print(model.summary())

    print(model.evaluate(X_test, y_test))
    print(y_pred = model.predict(X_test))

"""
model = keras.Sequential()
# Add an Embedding layer expecting input vocab of size 1000, and
# output embedding dimension of size 64.
model.add(layers.Embedding(input_dim=1000, output_dim=64))

# Add a LSTM layer with 128 internal units.
model.add(layers.LSTM(128))

# Add a Dense layer with 10 units.
model.add(layers.Dense(10))

model.summary()
"""
