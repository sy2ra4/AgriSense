# 03_predict_image.py
import os
import json
import numpy as np
import tensorflow as tf
from tensorflow.keras.models import load_model
from PIL import Image
from rembg import remove

# --- Configuration ---
IMAGE_SIZE = 224
MODEL_PATH = "plant_disease_model.h5"
CLASSES_PATH = "class_indices.json"


# --- Modified Preprocessing Function with Background Removal ---
def load_and_preprocess_image(image_path, target_size=(IMAGE_SIZE, IMAGE_SIZE)):
    """
    Loads, removes background, composites onto white, resizes, and prepares image 
    for model prediction.
    """
    if not os.path.exists(image_path):
        print(f"Error: Image file not found at {image_path}")
        return None

    # 1. Load the image and convert to RGBA
    img = Image.open(image_path).convert("RGBA")

    # 2. Remove background using rembg
    img_no_bg = remove(img)

    # 3. Composite transparent image onto a white background
    white_bg = Image.new("RGBA", img_no_bg.size, (255, 255, 255))
    img_composited = Image.alpha_composite(white_bg, img_no_bg)
    
    # 4. Resize and convert to RGB
    img_resized = img_composited.resize(target_size)
    img_rgb = img_resized.convert('RGB')

    # 5. Convert to numpy array and scale
    img_array = np.array(img_rgb).astype('float32') / 255.

    # 6. Add batch dimension (1, H, W, C)
    img_array = np.expand_dims(img_array, axis=0)

    return img_array


# --- Prediction Function ---
def predict_image_class(model, image_path, class_indices):
    # Preprocess the image
    preprocessed_image = load_and_preprocess_image(image_path)
    
    if preprocessed_image is None:
        return "Prediction Failed (Image Load Error)"

    # Make the prediction
    predictions = model.predict(preprocessed_image)

    # Get the index of the class with the highest probability
    predicted_class_index = np.argmax(predictions, axis=1)[0]

    # Map index back to the actual class name
    predicted_class_name = class_indices[str(predicted_class_index)]

    # Get confidence score
    confidence = predictions[0][predicted_class_index] * 100

    return predicted_class_name, confidence


# --- Main Execution ---
if __name__ == "__main__":
    # Load the trained model and class indices
    try:
        model = load_model(MODEL_PATH, compile=False)
        with open(CLASSES_PATH, 'r') as f:
            class_indices = json.load(f)

        class_indices = {str(v): k for k, v in class_indices.items()}
        
        # Set an image path for prediction
        example_image_path = "/content/1573847-WEB.jpg"
        
        if os.path.exists(example_image_path):
            predicted_class_name, confidence = predict_image_class(model, example_image_path, class_indices)
            print("-" * 40)
            print(f"Image: {os.path.basename(example_image_path)}")
            print(f"Predicted Disease: {predicted_class_name}")
            print(f"Confidence: {confidence:.2f}%")
            print("-" * 40)
        else:
            print(f"\nERROR: Please update the 'example_image_path' in 03_predict_image.py to a valid image file.")

    except FileNotFoundError:
        print("\nERROR: Model or class indices not found.")