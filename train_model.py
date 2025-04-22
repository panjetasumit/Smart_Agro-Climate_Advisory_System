import pandas as pd
import pickle
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier

# Load dataset
data = pd.read_csv("Rainfall.csv")

# Data preprocessing
data.columns = data.columns.str.strip()  # Remove spaces
data = data.drop(columns=["day"])  # Drop non-relevant columns
data["rainfall"] = data["rainfall"].map({"yes": 1, "no": 0})  # Encode target variable
data.fillna(data.median(), inplace=True)  # Handle missing values

# Features & Target
X = data.drop(columns=["rainfall"])
y = data["rainfall"]

# Train-test split
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Train model
model = RandomForestClassifier(n_estimators=100, random_state=42)
model.fit(X_train, y_train)

# Save the trained model
model_data = {"model": model, "feature_names": X.columns.tolist()}
with open("rainfall_prediction_model.pkl", "wb") as file:
    pickle.dump(model_data, file)

print("Model trained and saved successfully!")
