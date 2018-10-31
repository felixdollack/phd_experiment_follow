#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxGui.h"
#include "ofx_udp_trigger.h"
#include "vicon_receiver.h"
#include "ofx_blinky.h"

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
    ofxLabel _phone_label, _tracking_label, _presentation_label;
    ofxToggle _toggle_button_eog, _toggle_button_sound;
    ofxButton _push_button_eight, _push_button_limacon;
    ofxButton _push_button_connect, _push_button_disconnect;
    ofxButton _reset_head_origin;
    // ui callbacks
    void connectPhone();
    void disconnectPhone();
    void resetHeadOrigin();
    void setPathToEight();
    void setPathToLimacon();
    void toggleRecording(const void *sender, bool &value);
    void toggleSound(const void *sender, bool &value);

    // visual feedback
    void setupVisualFeedback();
    void drawVisualFeedback();
    float _ui_world_diameter;
    ofVec2f _ui_center, _ui_world_start;
    float _x_origin, _y_origin, _z_origin, _phi_origin;

    // sound source specific settings
    float _source_height, _source_radius;
    ofVec2f _source_positions;
    int _selected_shape;
    Blinky *_source_instance;
    ofVec2f shape_eight(float a, float time, float time_offset);
    ofVec2f shape_limacon(float b, float a, float time, float time_offset);
    vector<ofVec2f> _full_path;
    float _path_duration, _path_step, _step_duration, _path_revolutions;
    float _time, _current_phi, _phi_offset;
    ofVec2f _shape_offset;

    // shimmer eog
    string _eog_host;
    UdpTrigger *_eog_trigger;
    void setupEogTrigger();
    bool _start_recoring, _is_recording;

    // motion capture
    int _mocap_receive_port, _mocap_send_port;
    string _mocap_ip;
    bool _use_vicon;
    ViconReceiver _vicon_receiver;
    HeadPositionAndRotation _head_data;
    void setupMotionCapture();

    // network
    int _android_port;
    string _my_ip = "";
    string getIPhost();
    vector<string> getLocalIPs();
    void setupTCPserver();
    void sendMessageToPhone(int client, string message);
    ofxTCPServer* _android_tcp_server;
};
