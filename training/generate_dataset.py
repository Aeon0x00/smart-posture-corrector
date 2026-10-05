import numpy as np
import pandas as pd

np.random.seed(42)
samples_per_class = 30000

good_x = np.random.normal(0.28, 0.01, samples_per_class)
good_y = np.random.normal(0.94, 0.01, samples_per_class)
good_z = np.random.normal(0.10, 0.03, samples_per_class)

good_x += np.sin(np.linspace(0, 100, samples_per_class)) * 0.005
good_y += np.sin(np.linspace(0, 80, samples_per_class)) * 0.005
good_z += np.sin(np.linspace(0, 60, samples_per_class)) * 0.01

good_labels = ['good_posture'] * samples_per_class

bad_x = np.random.normal(0.20, 0.015, samples_per_class)
bad_y = np.random.normal(0.65, 0.015, samples_per_class)
bad_z = np.random.normal(-0.74, 0.01, samples_per_class)

bad_x += np.sin(np.linspace(0, 90, samples_per_class)) * 0.008
bad_y += np.sin(np.linspace(0, 70, samples_per_class)) * 0.008
bad_z += np.sin(np.linspace(0, 50, samples_per_class)) * 0.012

bad_labels = ['bad_posture'] * samples_per_class

motion_samples = samples_per_class // 2
motion_x = np.random.uniform(-0.5, 0.5, motion_samples)
motion_y = np.random.uniform(-0.5, 1.0, motion_samples)
motion_z = np.random.uniform(-1.0, 1.0, motion_samples)

motion_labels = ['motion'] * motion_samples

df = pd.DataFrame({
    'acc_x': np.concatenate([good_x, bad_x, motion_x]),
    'acc_y': np.concatenate([good_y, bad_y, motion_y]),
    'acc_z': np.concatenate([good_z, bad_z, motion_z]),
    'label': good_labels + bad_labels + motion_labels
})

df = df.sample(frac=1, random_state=42).reset_index(drop=True)
df.to_csv('posture_dataset.csv', index=False)
