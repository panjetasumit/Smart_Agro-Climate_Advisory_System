from flask import Flask, request, jsonify
from flask_cors import CORS
import pickle
import pandas as pd

# Initialize Flask application
app = Flask(__name__)
CORS(app)  # Enable Cross-Origin Resource Sharing

# Load the trained model
with open("rainfall_prediction_model.pkl", "rb") as file:
    model_data = pickle.load(file)

# Define feature names
feature_names = ['pressure', 'maxtemp', 'temparature', 'mintemp', 'dewpoint', 'humidity', 'cloud', 'windspeed']

# Define the route for prediction
@app.route("/predict", methods=["POST", "GET"])
def predict():
    try:
        if request.method == "POST":
            data = request.json
        elif request.method == "GET":
            # For GET, we will get data from the query parameters
            data = {
                'pressure': request.args.get('pressure'),
                'maxtemp': request.args.get('maxtemp'),
                'temparature': request.args.get('temparature'),
                'mintemp': request.args.get('mintemp'),
                'dewpoint': request.args.get('dewpoint'),
                'humidity': request.args.get('humidity'),
                'cloud': request.args.get('cloud'),
                'windspeed': request.args.get('windspeed'),
            }
            print("Received data:", data)

        # Ensure the incoming data matches the expected features
        input_df = pd.DataFrame([data], columns=feature_names)

        # Make the prediction
        prediction = model_data.predict(input_df)[0]
        crop = "Paddy" if prediction == 1 else "Wheat"  # You can add more logic if needed

        return jsonify({
            "rainfall": "Yes" if prediction == 1 else "No",
            "suggested_crop": crop
        })
    except Exception as e:
        return jsonify({"error": str(e)})

# Run the application
if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
