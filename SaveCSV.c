#include <stdio.h>
#include "LeapC.h"
#include "ExampleConnection.h"
#ifdef _WIN32
#include <conio.h>
#endif

int64_t lastFrameID = 0;
bool stop = false;

int main() {
    // Initialize the Leap connection
    OpenConnection();
    while (!IsConnected)
        millisleep(100);

    printf("Connected.\n");
    LEAP_DEVICE_INFO* deviceProps = GetDeviceProperties();
    if (deviceProps)
        printf("Using devie %s. \n", deviceProps->serial);

   

    // Open CSV file to save the coordinates
    FILE* csv_file = fopen("hand_joints.csv", "w");
    if (csv_file == NULL) {
        perror("Error opening file");
        return -1;
    }

    // Write the CSV header
    fprintf(csv_file, "HandID,HandType,Joint,Bone,StartX,StartY,StartZ,EndX,EndY,EndZ\n");

    // Capture and process frames
    while (!stop) {
        // Get a frame
        LEAP_TRACKING_EVENT* frame = GetFrame();
        if (frame && (frame->tracking_frame_id > lastFrameID)) {
            lastFrameID = frame->tracking_frame_id;
            
            // Process the hand coordinates data
            for (uint32_t h = 0; h < frame->nHands; h++) {
                LEAP_HAND* hand = &frame->pHands[h];
                printf("Hand id: %d, type: %s\n", hand->id, hand->type == eLeapHandType_Left ? "Left" : "Right");

                // Get the palm position
                LEAP_VECTOR palm_position = hand->palm.position;
                printf("Palm Position: X: %f, Y: %f, Z: %f\n", palm_position.x, palm_position.y, palm_position.z);

                // Write palm coordinates to the CSV file
                fprintf(csv_file, "%d,%s,Palm,None,%f,%f,%f,%f,%f,%f\n",
                    hand->id,
                    hand->type == eLeapHandType_Left ? "Left" : "Right",
                    palm_position.x, palm_position.y, palm_position.z,
                    palm_position.x, palm_position.y, palm_position.z); // Palm has one coordinate

                // Access each finger
                for (int f = 0; f < 5; ++f) {
                    LEAP_DIGIT finger = hand->digits[f];
                    printf("Finger %d:\n", f);

                    // Access each bone in the finger
                    for (int b = 0; b < 4; ++b) {
                        LEAP_BONE bone = finger.bones[b];
                        printf("  Bone %d Start: X: %f, Y: %f, Z: %f\n", b, bone.prev_joint.x, bone.prev_joint.y, bone.prev_joint.z);
                        printf("  Bone %d End: X: %f, Y: %f, Z: %f\n", b, bone.next_joint.x, bone.next_joint.y, bone.next_joint.z);

                        // Write bone coordinates to the CSV file
                        fprintf(csv_file, "%d,%s,Finger%d,Bone%d,%f,%f,%f,%f,%f,%f\n",
                            hand->id,
                            hand->type == eLeapHandType_Left ? "Left" : "Right",
                            f, b,
                            bone.prev_joint.x, bone.prev_joint.y, bone.prev_joint.z,  
                            bone.next_joint.x, bone.next_joint.y, bone.next_joint.z   
                        );
                    }
                }
            }
        }

// Check if 'q' key is pressed
#ifdef _WIN32
        if (_kbhit()) {
            char ch = _getch();
            if (ch == 'q' || 'Q') {
                stop = true;
            }
        }
#endif
    
        
    }

    // Close the CSV file
    fclose(csv_file);

   
    printf("Hand joint coordinates saved to 'hand_joints.csv'.\n");
    return 0;
}
