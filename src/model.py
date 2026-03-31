import pickle
import os
import pandas as pd
from sklearn.ensemble import RandomForestRegressor
from sklearn.model_selection import train_test_split
from sklearn.metrics import mean_absolute_error, r2_score
from .dataset import generate_process_data, create_ml_dataset

def train_and_save_model(num_processes=1000, history_window=3, save_path='model.pkl'):
    """
    Trains a Random Forest Regressor on generated synthetic data and saves it.
    """
    print(f"Generating data for {num_processes} processes...")
    df_raw = generate_process_data(num_processes)
    df_ml = create_ml_dataset(df_raw, history_window)
    
    # Features and target
    X = df_ml.drop(columns=['target_burst'])
    y = df_ml['target_burst']
    
    # Train-test split
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
    
    print(f"Training Random Forest Regressor on {len(X_train)} samples...")
    model = RandomForestRegressor(n_estimators=100, max_depth=10, random_state=42)
    model.fit(X_train, y_train)
    
    # Evaluation
    y_pred = model.predict(X_test)
    mae = mean_absolute_error(y_test, y_pred)
    r2 = r2_score(y_test, y_pred)
    
    print(f"Model Evaluation - MAE: {mae:.2f}, R2 Score: {r2:.4f}")
    
    # Save model
    with open(save_path, 'wb') as f:
        pickle.dump(model, f)
    print(f"Model saved to {save_path}")
    
    return model, mae, r2

def load_trained_model(model_path='model.pkl'):
    """
    Loads moving trained model.
    """
    if not os.path.exists(model_path):
        raise FileNotFoundError(f"Model file not found at {model_path}. Please train the model first.")
        
    with open(model_path, 'rb') as f:
        model = pickle.load(f)
    return model

def predict_next_burst(model, history_features):
    """
    Predicts the next CPU burst given history features.
    `history_features` should be a 2D array or DataFrame with the same shape as training features.
    """
    return model.predict(history_features)[0]

if __name__ == "__main__":
    train_and_save_model()
