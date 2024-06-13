/*
 * PUS_3_service.c
 *
 *  Created on: 2024. gada 12. jūn.
 *      Author: Rūdolfs Arvīds Kalniņš <rakal@kth.se>
 */
#include "Space_Packet_Protocol.h"

#define MAX_PAR_COUNT       256
#define MAX_STRUCT_COUNT    16
#define MAX_TM_DATA_LEN     (MAX_PAR_COUNT * 4) + 2 // Each parameter is potentialy 4 bytes and struct id is 2 bytes.

typedef struct {
    uint16_t SID;
    uint16_t collection_interval;
    uint16_t N1;
    uint16_t recv_par_order[MAX_PAR_COUNT];
} HK_par_report_structure_t;

HK_par_report_structure_t HKPRS_list[MAX_STRUCT_COUNT];

float HK_parameters[MAX_PAR_COUNT];

uint32_t HK_TM_seq_count = 0;

uint8_t periodic_report_en[MAX_STRUCT_COUNT];
uint16_t periodic_report_SIDs[MAX_STRUCT_COUNT];

// Writes data into out_field and returns incremented pointer.
// NOTE: I know this is a strange way to do this, but without doing double
// pointer stuff, idk how else you would approach this.
static uint8_t* get_next_field(uint8_t* data, uint16_t* out_field) {
    uint16_t field = 0;
    memcpy(&field, data, 2);
    data += 2;
    *out_field = field;
    return data;
}


static void create_report_struct(uint8_t* data) {
    // Assuming all fields in data are 16 bits.
    uint16_t SID = 0;
    data = get_next_field(data, &SID);
    HK_par_report_structure_t* HKPRS = &(HKPRS_list[SID]);
    data = get_next_field(data, &(HKPRS->SID));
    data = get_next_field(data, &(HKPRS->collection_interval));
    data = get_next_field(data, &(HKPRS->N1));
    if (HKPRS->N1 > MAX_PAR_COUNT) {
        return; // TODO: Add error code here
    }
    for(int i = 0; i < HKPRS->N1; i++) {
        data = get_next_field(data, &(HKPRS->recv_par_order[i]));
    }
}

static void delete_report_struct(uint8_t* data) {
    uint16_t nof_structs = 0;
    data = get_next_field(data, &nof_structs);
    for (int i = 0; i < nof_structs; i++) {
        uint16_t SID = 0;
        data = get_next_field(data, &SID);
        HK_par_report_structure_t* HKPRS = &(HKPRS_list[SID]);
        HKPRS->SID = 0;
        HKPRS->collection_interval = 0;
        HKPRS->N1 = 0;
        for(int i = 0; i < MAX_PAR_COUNT; i++) {
            HKPRS->recv_par_order[i] = 0;
        }
    }  
}

static void fill_report_struct(uint16_t SID) {
    float s_vbat = vbat;
    float s_temp = temperature;
    float s_fpga3v = fpga3v;
    float s_fpga1p5v = fpga1p5v;
    float s_uc3v = uc3v;

    HK_par_report_structure_t* HKPRS = &(HKPRS_list[SID]);
    for (int i = 0; i < HKPRS->N1; i++) {
        if (i > MAX_PAR_COUNT) {
            return; // TODO: Add error code here
        }

        switch(HKPRS->recv_par_order[i]) {
            case VBAT:
                HK_parameters[i] = s_vbat;
                break;
            case AMBIENT_TEMP:
                HK_parameters[i] = s_temp;
                break;
            case FPGA_CORE_VOLTAGE:
                HK_parameters[i] = s_fpga3v;
                break;
            case FPGA_IO_VOLTAGE:
                HK_parameters[i] = s_fpga1p5v;
                break;
            case UC_VOLTAGE:
                HK_parameters[i] = s_uc3v;
                break;
            default:
                break; 
        }
    }
}


static uint16_t encode_HK_struct(HK_par_report_structure_t* HKPRS, uint8_t* out_buffer) {
    uint8_t* orig_pointer = out_buffer;
    memcpy(out_buffer, &(HKPRS->SID), sizeof(HKPRS->SID));
    out_buffer += sizeof(HKPRS->SID);

    for (int i = 0; i < HKPRS->N1; i++) {
        memcpy(out_buffer, &(HK_parameters[i]), sizeof(HK_parameters[i]));
        out_buffer += sizeof(HK_parameters[i]);
    }
    return out_buffer - orig_pointer;
}

static void send_one_shot(SPP_primary_header_t* req_p_header , SPP_PUS_TC_header_t* req_s_header, uint8_t* data){
    uint16_t nof_structs = 0;
    data = get_next_field(data, &nof_structs);
    uint16_t SIDs[MAX_STRUCT_COUNT];
    for (int i = 0; i < nof_structs; i++) {
        data = get_next_field(data, &(SIDs[i]));
    }
    for (int i = 0; i < nof_structs; i++) {
        uint16_t SID = SIDs[i];
        fill_report_struct(SID);
        HK_par_report_structure_t* HKPRS = &(HKPRS_list[SID]);
        uint8_t TM_data[MAX_TM_DATA_LEN];
        uint16_t HK_data_len = encode_HK_struct(HKPRS, TM_data);
        SPP_primary_header_t TM_SPP_header = SPP_make_new_primary_header(
            SPP_VERSION,
            SPP_PACKET_TYPE_TM,
            1,
            req_p_header->application_process_id,
            SPP_SEQUENCE_SEG_UNSEG,
            HK_TM_seq_count++,
            SPP_PUS_TM_HEADER_LEN_WO_SPARE + HK_data_len + CRC_BYTE_LEN - 1
        );
        SPP_PUS_TM_header_t TM_PUS_header  = SPP_make_new_PUS_TM_header(
            PUS_VERSION,
            0,
            HOUSEKEEPING_SERVICE_ID,
            HK_PARAMETER_REPORT,
            0,
            req_s_header->source_id,
            0
        );
        SPP_send_TM(&TM_SPP_header, &TM_PUS_header, TM_data, HK_data_len);
    }
}

static void enable_periodic_report(uint8_t data) {
    uint16_t nof_structs = 0;
    data = get_next_field(data, nof_structs);
    for (int i = 0; i < nof_structs; i++) {
        data = get_next_field(data, &(periodic_report_SIDs[i]));
    }
    for (int i = 0; i < nof_structs; i++) {
        periodic_report_en[periodic_report_SIDs[i]] = 1;
    }
}


void SPP_send_periodic_HK_TM() {
    for (int i = 0; i < MAX_STRUCT_COUNT; i++) {
        if (periodic_report_en[i]) {
            
        }
    }
}

// HK - Housekeeping PUS service 3
SPP_error SPP_handle_HK_TC(SPP_primary_header_t* SPP_header , SPP_PUS_TC_header_t* secondary_header, uint8_t* data) {
    if (secondary_header == NULL) {
        return UNDEFINED_ERROR;
    }

    if (secondary_header->message_subtype_id == HK_CREATE_HK_PAR_REPORT_STRUCT) {
        create_report_struct(data);
    } else if (secondary_header->message_subtype_id == HK_DELETE_HK_PAR_REPORT_STRUCT) {
        delete_report_struct(data);
    } else if (secondary_header->message_subtype_id == HK_EN_PERIODIC_REPORTS) {
        enable_periodic_report(data);
    } else if (secondary_header->message_subtype_id == HK_ONE_SHOT) {
        send_one_shot(SPP_header, secondary_header, data);
    }
    return SPP_OK;
}
