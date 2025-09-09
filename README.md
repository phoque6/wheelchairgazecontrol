Project ARGOS: Eye-Controlled ESP32 Wheelchair for Assistive Mobility
1. Introduction: The Need for Innovation in Assistive Technology

Paralysis affects millions worldwide—restricting voluntary muscle movement due to damage along the neural pathways between the brain and muscles. While advances in medicine have improved diagnosis and treatment, motor rehabilitation remains a significant challenge for individuals with conditions such as quadriplegia, ALS (Lou Gehrig’s Disease), or spinal cord injuries.

Project ARGOS emerges as an innovative response—a proof-of-concept smart wheelchair prototype controlled entirely by eye movements, enabling hands-free mobility through the integration of computer vision, cloud computing, and robotics.

Named after Argos Panoptes, the all-seeing giant of Greek mythology, the system acts as the "eyes" for users who are otherwise immobilised. With no physical input devices needed, Project ARGOS uses AI to translate gaze direction and blinking into motion commands.

2. Technologies Behind Project ARGOS

🧠 MediaPipe (Google AI Library)
MediaPipe is an open-source cross-platform framework developed by Google, optimised for building real-time perception pipelines. It provides robust eye and facial landmark detection using deep learning models.
Used for: Eye-gaze detection (left, right, center) and blink detection.
Why: Lightweight, fast, Python-compatible, and suitable for edge computing.

☁️ Firebase Realtime Database
Firebase acts as the cloud-based communication bridge between the eye-tracking Python script and the ESP32 robot. MediaPipe writes real-time gaze data to Firebase, which the ESP32 reads instantly to act upon.
Used for: Cloud communication, real-time motor commands.
Why: Seamless syncing between devices without needing direct serial connection.

🔧 ESP32 Arduino Microcontroller
ESP32 is a powerful, low-cost Wi-Fi and Bluetooth-enabled microcontroller capable of running Arduino code and controlling motors.
Used for: Driving the motors of the wheelchair based on cloud commands.
Why: Ideal for real-time robotics with wireless connectivity and GPIO support.

3. Project Objectives

Enable eye-controlled movement of a prototype wheelchair using real-time computer vision.
Develop a Python-based gaze detection system that requires no physical interaction.
Build an ESP32-powered robotic car chassis to act as a proof-of-concept wheelchair.
Ensure the entire system is cost-effective, wireless, and accessible for future assistive tech implementations.

4. Development Stages of Project ARGOS
🔍 Stage 1: Ideation and Research
The student IVP team brainstormed and researched existing assistive technologies, identifying speed, cost, and real-time responsiveness as common limitations. They explored how AI tools like MediaPipe could offer a low-cost alternative to expensive commercial eye trackers.

🛠 Stage 2: Assembling the Proof-of-Concept Wheelchair
A basic robotic car chassis was assembled with two motors and an ESP32 development board to mimic a compact wheelchair. Power supply, motor drivers (L298N), and wheels were configured for forward, left, right, and stop movements.

💻 Stage 3: MediaPipe Gaze Tracking (Python)
Students wrote a Python script using OpenCV + MediaPipe to detect:
Eye position
Direction (left, right, center)
Blink detection
The script translated this input into text commands (“LEFT”, “RIGHT”, “FORWARD”, “STOP”) and uploaded them to Firebase RTDB.

🌐 Stage 4: Firebase Integration

Firebase acted as the cloud gateway. The Python program updated a field in the Firebase RTDB, while the ESP32 constantly listened for changes using the Firebase ESP32 library. On detecting a new value, the ESP32 triggered motor actions accordingly.

🔄 Stage 5: Testing, Iteration, Refinement

Extensive testing was done to improve:
Eye detection accuracy
Motor responsiveness
Turning precision
Motor power values were calibrated to ensure smoother left/right turns using analogWrite on the motor PWM pins in Arduino. Multiple iterations were performed to reduce lag and increase stability.

5. How the System Works (Overview)

1. Gaze Detection (MediaPipe)
Camera → Python Script → Detects eye direction → Sends string command to Firebase.

2. Command Relay (Firebase)
Realtime Database updates → ESP32 checks Firebase every few milliseconds.

3. Motor Control (ESP32)
ESP32 reads command → Translates into motor action → Moves the prototype car.

6. Outcome & Impact
Zero lag in control—real-time reaction to eye movement.
No physical contact needed—fully touch-free.
Achieved 98% accuracy in detecting gaze direction and blink using just a laptop webcam.
Demonstrated potential for affordable assistive mobility, especially in low-resource settings.

7. Future Development Ideas
Integration with GPS for outdoor use.
Voice fallback system for redundancy.
Multi-device syncing for family members to track real-time location.
Integration with medical alerts and patient monitoring systems.

8. Conclusion
Project ARGOS showcases how AI + hardware + student innovation can come together to solve real-world problems. This prototype, while small, lays the foundation for accessible assistive technology solutions that could transform lives.With continuous iterations, Project ARGOS can evolve from a school project into a life-changing system for those with mobility challenges.
