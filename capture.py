import time
import datetime
from picamera2 import Picamera2, Preview 

# --- Configuration ---
CAPTURE_RESOLUTION = (1920, 1080)
PREVIEW_RESOLUTION = (640, 480)

def capture_image_with_confirmation():
    """
    Starts camera preview, waits for confirmation, then stops the camera 
    to re-configure and capture a high-res image.
    """
    print("Initializing Picamera2...")
    picam2 = Picamera2()

    # 1. Configure for Preview
    preview_config = picam2.create_preview_configuration(main={"size": PREVIEW_RESOLUTION})
    picam2.configure(preview_config)

    try:
        # Start the camera and the preview
        # Use Preview.QT for direct monitor connection, or Preview.QT for VNC
        picam2.start_preview(Preview.QT) 
        picam2.start()
        
        print(f"Camera preview started. Resolution: {PREVIEW_RESOLUTION}")
        print("Waiting for camera to warm up...")
        time.sleep(2) 

        # 2. Wait for User Confirmation
        input("Press 'Enter' to take a picture, or 'Ctrl+C' to quit preview and exit.\n")

        # 3. Stop Camera and Preview BEFORE Re-configuring ?? THE FIX
        picam2.stop_preview()
        picam2.stop() # MUST stop the camera before calling configure() again
        print("Camera stopped. Preparing for high-resolution capture...")
        
        # 4. Configure for Still Capture
        still_config = picam2.create_still_configuration(main={"size": CAPTURE_RESOLUTION})
        picam2.configure(still_config)
        
        # 5. Restart the camera to apply the new still configuration
        picam2.start() 
        print(f"Capture resolution set to: {CAPTURE_RESOLUTION}")
        time.sleep(1) # Give a moment for the new mode to stabilize

        # 6. Capture the Image
        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"image_{timestamp}.jpg"
        
        picam2.capture_file(filename)
        
        print("-" * 30)
        print(f"? Image successfully captured!")
        print(f"File saved as: {filename}")
        print("-" * 30)

    except KeyboardInterrupt:
        print("\nOperation cancelled by user (Ctrl+C).")
    
    except Exception as e:
        print(f"\nAn error occurred: {e}")

    finally:
        # 7. Final Cleanup
        picam2.stop()
        print("Camera gracefully stopped. Exiting script.")


if __name__ == "__main__":
    capture_image_with_confirmation()
