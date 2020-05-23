# 1 "c:\\users\\wmb\\appdata\\local\\temp\\tmpmogfes"
#include <Arduino.h>
# 1 "C:/Users/wmb/Documents/GitHub/Grbl_Settings/Grbl_Esp32/Grbl_Esp32.ino"
# 21 "C:/Users/wmb/Documents/GitHub/Grbl_Settings/Grbl_Esp32/Grbl_Esp32.ino"
#include "grbl.h"
#include "WiFi.h"

#include "Spindles/SpindleClass.cpp"


system_t sys;
int32_t sys_position[N_AXIS];
int32_t sys_probe_position[N_AXIS];
volatile uint8_t sys_probe_state;
volatile uint8_t sys_rt_exec_state;
volatile uint8_t sys_rt_exec_alarm;
volatile uint8_t sys_rt_exec_motion_override;
volatile uint8_t sys_rt_exec_accessory_override;
#ifdef DEBUG
    volatile uint8_t sys_rt_exec_debug;
#endif

Spindle *spindle;
void setup();
void loop();
#line 43 "C:/Users/wmb/Documents/GitHub/Grbl_Settings/Grbl_Esp32/Grbl_Esp32.ino"
void setup() {
    WiFi.persistent(false);
    WiFi.disconnect(true);
    WiFi.enableSTA(false);
    WiFi.enableAP(false);
    WiFi.mode(WIFI_OFF);
    serial_init();
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Grbl_ESP32 Ver %s Date %s", GRBL_VERSION, GRBL_VERSION_BUILD);
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Compiled with ESP32 SDK:%s", ESP.getSdkVersion());

#ifdef MACHINE_NAME
  #ifdef MACHINE_EXTRA
    #define MACHINE_STRING MACHINE_NAME MACHINE_EXTRA
  #else
    #define MACHINE_STRING MACHINE_NAME
  #endif
    report_machine_type(CLIENT_SERIAL);
#endif
    settings_init();
    stepper_init();
    system_ini();
    memset(sys_position, 0, sizeof(sys_position));
#ifdef USE_PEN_SERVO
    servo_init();
#endif
#ifdef USE_SERVO_AXES
    init_servos();
#endif
#ifdef USE_PEN_SOLENOID
    solenoid_init();
#endif
#ifdef USE_MACHINE_INIT
    machine_init();
#endif

#ifdef FORCE_INITIALIZATION_ALARM

    sys.state = STATE_ALARM;
#else
    sys.state = STATE_IDLE;
#endif







#ifdef HOMING_INIT_LOCK
    if (homing_enable->get()) sys.state = STATE_ALARM;
#endif
    spindle_select(SPINDLE_TYPE);
#ifdef ENABLE_WIFI
    wifi_config.begin();
#endif
#ifdef ENABLE_BLUETOOTH
    bt_config.begin();
#endif
    inputBuffer.begin();
}

void loop() {

    uint8_t prior_state = sys.state;
    memset(&sys, 0, sizeof(system_t));
    sys.state = prior_state;
    sys.f_override = DEFAULT_FEED_OVERRIDE;
    sys.r_override = DEFAULT_RAPID_OVERRIDE;
    sys.spindle_speed_ovr = DEFAULT_SPINDLE_SPEED_OVERRIDE;
    memset(sys_probe_position, 0, sizeof(sys_probe_position));
    sys_probe_state = 0;
    sys_rt_exec_state = 0;
    sys_rt_exec_alarm = 0;
    sys_rt_exec_motion_override = 0;
    sys_rt_exec_accessory_override = 0;

    serial_reset_read_buffer(CLIENT_ALL);
    gc_init();
    spindle->stop();
    coolant_init();
    limits_init();
    probe_init();
    plan_reset();
    st_reset();

    plan_sync_position();
    gc_sync_position();

    report_init_message(CLIENT_ALL);

    protocol_main_loop();
}