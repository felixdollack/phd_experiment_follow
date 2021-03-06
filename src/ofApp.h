#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxGui.h"
#include "ofx_udp_trigger.h"
#include "vicon_receiver.h"
#include "ofx_blinky.h"
#include <ctime>
#include "ofxOsc.h"

struct Participant {
    float x;
    float y;
    float z;
    float phi;
};

class ofApp : public ofBaseApp{

	public:
		void exit();
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void windowResized(int w, int h);
		void gotMessage(ofMessage msg);

private:
    ofxXmlSettings *_settings;
    const string _settings_filename = "settings.xml";
    void loadSettingsAndWriteDefaultIfNeeded();
    void writeDefaultSettings();

    // ui settings
    float _ui_head_radius;
    // ui
    void setupUI();
    ofxPanel _uiPanel;
    ofxLabel _phone_label, _tracking_label, _presentation_label, _reproduction_label;
    ofxToggle _toggle_button_eog, _toggle_button_sound, _reproduction_button, _landmarks_button;
    ofxButton _push_button_eight, _push_button_limacon, _push_button_circle;
    ofxButton _push_button_connect, _push_button_disconnect;
    ofxButton _reset_head_origin;
    // ui callbacks
    void connectPhone();
    void disconnectPhone();
    void resetHeadOrigin();
    void setPathToEight();
    void setPathToLimacon();
    void setPathToCircle();
    void toggleRecording(const void *sender, bool &value);
    void toggleSound(const void *sender, bool &value);
    void toggleReproduction(const void *sender, bool &value);
    void toggleLandmarks(const void *sender, bool &value);

    // visual feedback
    void setupVisualFeedback();
    void drawVisualFeedback();
    ofVec2f mapPositionToPixel(ofVec2f pos);
    float _line_width = 6.0f;
    float _ui_world_diameter, _ui_max_distance;
    ofVec2f _ui_center, _ui_world_start;
    float _shape_eight_half_size, _shape_eight_phi_off;
    float _shape_limacon_phi_off, _shape_limacon_center, _shape_limacon_offset;
    float _shape_circle_diameter;

    // sound source specific settings
    float _source_height, _source_radius;
    ofVec2f _source_positions;
    int _selected_shape;
    bool _direction;
    Blinky *_source_instance;
    ofVec2f shape_eight(float a, float time, float time_offset);
    ofVec2f shape_limacon(float b, float a, float time, float time_offset);
    ofVec2f shape_circle(float radius, float time, bool left);
    vector<ofVec2f> _full_path;
    float _path_duration, _path_step, _step_duration, _path_revolutions;
    float _time, _current_phi, _phi_offset;
    ofVec2f _shape_offset;
    bool _sound_on, _landmarks_on;

    // shimmer eog
    string _eog_host;
    UdpTrigger *_eog_trigger;
    void setupEogTrigger();
    bool _start_recoring, _is_recording;
    ofSerial triggerBox;
    void toggleTriggerBox();

    // tobii eye tracker
    ofxOscSender *_tobii_osc;
    string _tobii_ip;
    int _tobii_port;
    void setupProjectEyeTracker();
    void setupSubjectEyeTracker();
    void connectEyeTracker();
    void streamEyeTracker();
    void stopStreamEyeTracker();
    void stopRecordingEyeTracker();
    void cleanupEyeTracker();
    void calibrateEyeTracker();
    void recordEyeTracker();
    void sendEyeTrackerEvent(string message);

    // motion capture
    int _mocap_receive_port, _mocap_send_port;
    string _mocap_ip;
    bool _use_vicon;
    float _max_distance;
    ViconReceiver _vicon_receiver;
    HeadPositionAndRotation _head_data;
    void setupMotionCapture();
    bool bReproduction, bLandmarks;
    // network
    int _android_port; // 9000
    string _android_ip;
    string _my_ip = ""; // localhost or 0.0.0.0
    string getIPhost();
    vector<string> getLocalIPs();
    void setupTCPserver();
    void sendMessageToPhone(int client, string message);
    ofxOscSender *_ssr_osc;
    void connectToSSR(bool value);
    void loadSsrScene(string filename);
    void streamSSR(bool value);
    void updateSoundPos(float x, float y);
    void updatePos(float x, float y);
    void updateAngle(float phi);
    bool _ssr_running;

    // data logging
    void setupParticipant();
    void updateParticipantPosition();
    string nowToString();
    bool _isLogFileCreated;
    Participant _origin, _current, _old;
    float _logStartTime;
    string _username, _scene_name;
};
