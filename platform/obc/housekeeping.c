#include "housekeeping.h"

#undef __FILE_ID__
#define __FILE_ID__ 666

tc_tm_pkt hk_pkt;
uint8_t hk_pkt_data[264];

void hk_INIT() {
   hk_pkt.data = hk_pkt_data;
}

void hk_SCH() {

    hk_crt_pkt_TC(&hk_pkt, EPS_APP_ID, HEALTH_REP);
    route_pkt(&hk_pkt);
    hk_crt_pkt_TC(&hk_pkt, COMMS_APP_ID, HEALTH_REP);
    route_pkt(&hk_pkt);

    wdg.hk_valid = true;
    HAL_sys_delay(15000);

    wdg.hk_valid = true;
    HAL_sys_delay(15000);

    wod_log();
    clear_wod();
    hk_crt_pkt_TM(&hk_pkt, DBG_APP_ID, WOD_REP);
    route_pkt(&hk_pkt);

    hk_crt_pkt_TC(&hk_pkt, EPS_APP_ID, EX_HEALTH_REP);
    route_pkt(&hk_pkt);
    hk_crt_pkt_TC(&hk_pkt, COMMS_APP_ID, EX_HEALTH_REP);
    route_pkt(&hk_pkt);
    hk_crt_pkt_TC(&hk_pkt, ADCS_APP_ID, EX_HEALTH_REP);
    route_pkt(&hk_pkt);

    wdg.hk_valid = true;
    HAL_sys_delay(15000);

    wdg.hk_valid = true;
    HAL_sys_delay(15000);  

    hk_crt_pkt_TM(&hk_pkt, DBG_APP_ID, EXT_WOD_REP);
    route_pkt(&hk_pkt);
    clear_ext_wod();
}

void clear_wod() {
    sat_status.batt_curr = 0;
    sat_status.batt_volt = 0;
    sat_status.bus_3v3_curr = 0;
    sat_status.bus_5v_curr = 0;
    sat_status.temp_eps = 0;
    sat_status.temp_batt = 0;
    sat_status.temp_comms = 0;
}

void clear_ext_wod() {

}

SAT_returnState hk_parameters_report(TC_TM_app_id app_id, HK_struct_id sid, uint8_t *data) {
    
    if(app_id == EPS_APP_ID && sid == HEALTH_REP) {
        sat_status.batt_curr = data[1];
        sat_status.batt_volt = data[2];
        sat_status.bus_3v3_curr = data[3];
        sat_status.bus_5v_curr = data[4];
        sat_status.temp_eps = data[5];
        sat_status.temp_batt = data[6];

    } else if(app_id == COMMS_APP_ID && sid == HEALTH_REP) {
        sat_status.temp_comms = data[1];
    } else if(app_id == ADCS_APP_ID && sid == EX_HEALTH_REP) {

    } else if(app_id == EPS_APP_ID && sid == EX_HEALTH_REP) {

    } else if(app_id == COMMS_APP_ID && sid == EX_HEALTH_REP) {

    } else {
        return SATR_ERROR; // this should change to inv pkt
    }
    
    return SATR_OK;
}

SAT_returnState hk_report_parameters(HK_struct_id sid, tc_tm_pkt *pkt) {
    
    pkt->data[0] = (HK_struct_id)sid;
    
    if(sid == EX_HEALTH_REP) {

        struct time_utc temp_time;

        get_time_UTC(&temp_time);

        pkt->data[1] = temp_time.day;
        pkt->data[2] = temp_time.month;
        pkt->data[3] = temp_time.year;
        
        pkt->data[4] = temp_time.hour;
        pkt->data[5] = temp_time.min;
        pkt->data[6] = temp_time.sec;
        
        pkt->len = 7;
    } else if(sid == WOD_REP) {

        uint32_t time_temp;
        get_time_QB50(&time_temp);

        cnv32_8(time_temp, &pkt->data[1]);
        wod_log_load(&pkt->data[5]);

        uint16_t size = 4+(32*8);
        mass_storage_storeFile(WOD_LOG, 0, &pkt->data[1], &size);
        pkt->len = 1+4+(32*8); //we should see what mode is.
    } else if(sid == EXT_WOD_REP) {

        uint32_t time_temp;
        get_time_QB50(&time_temp);

        cnv32_8(time_temp, &pkt->data[1]);

        time_temp = HAL_sys_GetTick();
        cnv32_8(time_temp, &pkt->data[5]);

        uint16_t size = 4+4;
        mass_storage_storeFile(EXT_WOD_LOG, 0, &pkt->data[1], &size);
        pkt->len = 1+4+4; //we should see what mode is.
    }

    return SATR_OK;
}