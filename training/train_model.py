import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import confusion_matrix, classification_report
import matplotlib.pyplot as plt
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers

df = pd.read_csv('posture_dataset.csv')
df = df[df['label'].isin(['good_posture', 'bad_posture'])]
df['label'] = (df['label'] == 'good_posture').astype(int)

X = df[['acc_x', 'acc_y', 'acc_z']].values
y = df['label'].values

X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42, stratify=y
)

scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)

with open('normalization_params.txt', 'w') as f:
    f.write(f'MEAN_X {scaler.mean_[0]:.6f}\n')
    f.write(f'MEAN_Y {scaler.mean_[1]:.6f}\n')
    f.write(f'MEAN_Z {scaler.mean_[2]:.6f}\n')
    f.write(f'STD_X {scaler.scale_[0]:.6f}\n')
    f.write(f'STD_Y {scaler.scale_[1]:.6f}\n')
    f.write(f'STD_Z {scaler.scale_[2]:.6f}\n')

model = keras.Sequential([
    layers.Dense(16, activation='relu', input_shape=(3,)),
    layers.Dropout(0.3),
    layers.Dense(8, activation='relu'),
    layers.Dropout(0.2),
    layers.Dense(1, activation='sigmoid')
])

model.compile(
    optimizer='adam',
    loss='binary_crossentropy',
    metrics=['accuracy']
)

history = model.fit(
    X_train, y_train,
    epochs=50,
    batch_size=32,
    validation_split=0.2,
    verbose=1
)

test_loss, test_acc = model.evaluate(X_test, y_test)
print(f"Test accuracy: {test_acc:.4f}, loss: {test_loss:.4f}")

model.save('posture_binary_model.h5')

converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
tflite_model = converter.convert()

with open('posture_model.tflite', 'wb') as f:
    f.write(tflite_model)

with open('posture_model.h', 'w') as f:
    f.write('#ifndef POSTURE_MODEL_H\n')
    f.write('#define POSTURE_MODEL_H\n\n')
    f.write('const unsigned char posture_model[] = {\n')
    for i, byte in enumerate(tflite_model):
        if i % 12 == 0:
            f.write('  ')
        f.write(f'0x{byte:02x}')
        if i < len(tflite_model) - 1:
            f.write(', ')
        if (i + 1) % 12 == 0:
            f.write('\n')
    f.write('\n};\n')
    f.write(f'const unsigned int posture_model_len = {len(tflite_model)};\n\n')
    f.write('#endif\n')

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(10, 4))
ax1.plot(history.history['loss'], label='Train')
ax1.plot(history.history['val_loss'], label='Val')
ax1.set_xlabel('Epoch')
ax1.set_ylabel('Loss')
ax1.legend()

ax2.plot(history.history['accuracy'], label='Train')
ax2.plot(history.history['val_accuracy'], label='Val')
ax2.set_xlabel('Epoch')
ax2.set_ylabel('Accuracy')
ax2.legend()

plt.tight_layout()
plt.savefig('training_binary_history.png', dpi=150)
plt.close()

y_pred = (model.predict(X_test) > 0.5).astype(int)
cm = confusion_matrix(y_test, y_pred)

fig, ax = plt.subplots(figsize=(6, 5))
im = ax.imshow(cm, cmap='Blues')
ax.set_xticks([0, 1])
ax.set_yticks([0, 1])
ax.set_xticklabels(['Bad', 'Good'])
ax.set_yticklabels(['Bad', 'Good'])
ax.set_xlabel('Predicted')
ax.set_ylabel('Actual')

for i in range(2):
    for j in range(2):
        ax.text(j, i, cm[i, j], ha='center', va='center', color='black', fontsize=14)

plt.tight_layout()
plt.savefig('confusion_matrix_binary.png', dpi=150)
plt.close()
