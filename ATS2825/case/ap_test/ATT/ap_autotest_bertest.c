/*******************************************************************************
 *                              US212A
 *                            Module: Picture
 *                 Copyright(c) 2003-2012 Actions Semiconductor,
 *                            All Rights Reserved.
 *
 * History:
 *      <author>    <time>           <version >             <desc>
 *       zhangxs     2011-09-19 10:00     1.0             build this file
 *******************************************************************************/
/*!
 * \file     user1_main.c
 * \brief    picture��ģ�飬������̳�ʼ�����˳�������������
 * \author   zhangxs
 * \version  1.0
 * \date  2011/9/05
 *******************************************************************************/
#include "ap_manager_test.h"
#include "ap_autotest_mptest.h"
#include "bt_controller_interface.h"
#include "ap_autotest_bertest.h"

/** �����Ȳ���ȫ�ֿ��ƽṹ
*/
ber_control_t *g_ber_control;
const uint8 pkt_num_cmd[] = {0x01, 0x49, 0xfd, 0x01, 0xb9};
const uint8 error_bit_cmd[] = {0x01, 0x49, 0xfd, 0x01, 0xbc};
const uint8 grssi_cmd[] = {0x01, 0x49, 0xfd, 0x01, 0xb8};

//const uint8 clear_report_cmd0[] = {0x01, 0x4a, 0xfd, 0x04, 0x97, 0xf8, 0x01, 0x00};
//const uint8 clear_report_cmd1[] = {0x01, 0x4a, 0xfd, 0x04, 0x97, 0xf8, 0x0f, 0x00};

const uint8 clear_report_cmd0[] = {0x01, 0x49, 0xfd, 0x01, 0x97};
const uint8 clear_report_cmd1[] = {0x01, 0x4a, 0xfd, 0x04, 0x97, 0xe6, 0x09, 0x00};
const uint8 clear_report_cmd2[] = {0x01, 0x49, 0xfd, 0x01, 0x97};
const uint8 clear_report_cmd3[] = {0x01, 0x4a, 0xfd, 0x04, 0x97, 0xe6, 0x0f, 0x00};

/** 
 ����: RX��ӡ
 ����: 
 ����: 
 ˵��: 
*/
static void ber_print_rxtx(uint8 *buf, uint32 len, uint32 mode)
{
#if 0
    dma_print_info_t data_info;
    data_info.dma_channel = 2;//dma2
    if(len > 10)
    {
        len = 10;
    }

    libc_memset(&(g_ber_control->rx_debug_buf[0]), 0, RX_DEBUG_BYTES);
    libc_memcpy(&(g_ber_control->rx_debug_buf[0]), buf, len);
    data_info.len = len;
    data_info.data.buf = &(g_ber_control->rx_debug_buf[0]);
    if(mode == 0)
    {
        libc_dma_print("<RX>", &data_info, DMA_PRINT_BUF1);
    }
    else
    {
        libc_dma_print("<TX>", &data_info, DMA_PRINT_BUF1);        
    }
#endif
}

/** 
 ����: ��ȡHCI������
 ����: 
 ����:
 ˵��:
*/
void _ber_test_read_hci_data(uint16 read_num, uint8 hci_overflow)
{
    uint8 len = 0;
    uint32 ber_value = 0;
    uint16 err_bit;
    uint32 real_len;
    
    while (g_ber_control->g_hci_deal.get_data_len() >= g_ber_control->g_hci_deal.minReqLen)
    {
        g_ber_control->rev_pkt_flag = TRUE;
        
        //��ȡǰ�����ֽڣ����Ի�ȡRX�ܳ���
        g_ber_control->g_hci_deal.read_data((uint32) (&(g_ber_control->g_hci_deal.headerBuffer[0])), 3);

        real_len = g_ber_control->g_hci_deal.headerBuffer[2];
     
        while (g_ber_control->g_hci_deal.get_data_len() >= real_len)
        {
            g_ber_control->g_hci_deal.read_data((uint32) (&(g_ber_control->g_hci_deal.headerBuffer[3])), real_len);                        
        }
       
        //libc_print("len", 3 + real_len, 2);
        
        ber_print_rxtx(&(g_ber_control->g_hci_deal.headerBuffer[0]), 3 + real_len, 0);
        
        if((g_ber_control->g_hci_deal.headerBuffer[0] == 0x04) &&
            (g_ber_control->g_hci_deal.headerBuffer[1] == 0x0e) &&
            (g_ber_control->g_hci_deal.headerBuffer[2] == 0x06) &&
            (g_ber_control->g_hci_deal.headerBuffer[3] == 0x02))
        {        
            //reset receive data
            g_ber_control->g_hci_deal.headerBuffer[0] = 0x00;
            
            if(g_ber_control->ber_test_state == BER_STATE_FINISH)
            {
                //libc_print("more val", 0,0);
                break;
            }
            
            if(g_ber_control->cmd_state == BER_CMD_STATE_READ_PACKET_NUM)
            {                
                //get pkt count
                g_ber_control->pkt_info.result = (g_ber_control->g_hci_deal.headerBuffer[8] << 8) |
                                                          (g_ber_control->g_hci_deal.headerBuffer[7]);
                //g_ber_control->pkt_info.time = sys_get_ab_timer();
                libc_print("pktcnt:", g_ber_control->pkt_info.result,2);

                if(g_ber_control->pkt_info.result >= MIN_RECV_PKT_NUM)
                {
                    //������С���հ�������Ϊ�յ�����Ч���ݣ��˺�ʼ����BER
                    g_ber_control->cmd_state = BER_CMD_STATE_WRITE_ERROR_BIT;

                    g_ber_control->rev_valid_pkt_flag = TRUE;
                }
                else
                {
                    g_ber_control->cmd_state = BER_CMD_STATE_WRITE_CLEAR_REPORT;
                }
            }
            else if(g_ber_control->cmd_state == BER_CMD_STATE_READ_ERROR_BIT)
            {
                //get error bit
                err_bit = (g_ber_control->g_hci_deal.headerBuffer[8] << 8) |
                                        (g_ber_control->g_hci_deal.headerBuffer[7]);

                if(err_bit != 0)
                {
                    g_ber_control->cmd_state = BER_CMD_STATE_WRITE_RSSI;

                    //libc_print("err bit:", err_bit,2);

                    //libc_print("time:", sys_get_ab_timer() - g_ber_control->pkt_info.time,2);
                    //if((sys_get_ab_timer() - g_ber_control->pkt_info.time) < 30)
                    {
                        //get ber value
                        ber_value = (err_bit * 10000) / ((g_ber_control->pkt_info.result) * 216);

                        g_ber_control->ber_value[g_ber_control->pkt_info_count] = ber_value;

                        libc_print("ber_val:", ber_value, 2);
                        
                        //reset temp buf
                        //g_ber_control->pkt_info.result = 0;
                        //g_ber_control->pkt_info.time = 0;
                        g_ber_control->cmd_state = BER_CMD_STATE_WRITE_RSSI;
                    }                    
                }
                else
                {
                    g_ber_control->cmd_state = BER_CMD_STATE_WRITE_CLEAR_REPORT;
                }                
            }
            else if(g_ber_control->cmd_state == BER_CMD_STATE_READ_RSSI)
            {
                //get rssi
                g_ber_control->rssi = (g_ber_control->g_hci_deal.headerBuffer[8] << 8) |
                                        (g_ber_control->g_hci_deal.headerBuffer[7]);

                g_ber_control->rssi >>= 10; 

                g_ber_control->rssi_value[g_ber_control->pkt_info_count] = g_ber_control->rssi;

                //�Ȼ�ȡber���ٻ�ȡrssi����ȡ��rssi�����ݻ���
                g_ber_control->pkt_info_count++;

                //ʵ�ʵ�rssi���㹫ʽ���������
                //g_ber_control->rssi = (g_ber_control->rssi * 2 - 96);                        
                //libc_print("rssi:", g_ber_control->rssi,2);  
                g_ber_control->cmd_state = BER_CMD_STATE_WRITE_CLEAR_REPORT;
            } 
            else
            {
                
            }

            
            if(g_ber_control->pkt_info_count >= PKT_INFO_COUNT)
            {
                if(g_ber_control->ber_test_state != BER_STATE_FINISH)
                {
                    //finish ber test
                    //libc_print("get enough:", g_ber_control->pkt_info_count,2);
                    g_ber_control->ber_test_state = BER_STATE_FINISH;

                    g_ber_control->test_cnt++;
                }
            }
        }
    }
    
    return;
}


void send_bt_data(uint8 *data_buffer, uint32 data_len)
{
    ber_print_rxtx(data_buffer, data_len, 1);
    bt_drv_send_data(data_buffer, data_len);        
}

/** 
 ����: ��ȡpkt number��error bit�Ķ�ʱ���ص�
 ����: 
 ����: 
 ˵��: 
*/
void _ber_test_get_pktinfo_cb(void)
{
    g_ber_control->tick_cnt++;
}

static uint32 libc_abs(int32 value)
{
    if (value > 0)
    {
        return value;
    }
    else
    {
        return (0 - value);
    }
}

uint32 att_parse_ber_result(uint32 result_num, uint32 *p_ber_val, uint32 *p_rssi_val)
{
    int32 i;
    int32 diff_val;
    int8 div_val;
    int32 ber_val_total;
    int32 max_diff_val;
    int32 max_diff_index;
    int32 invalid_data_flag;
    int32 rssi_val_total;

    int32 ber_val;

    int32 rssi_val;

    div_val = 0;

    ber_val_total = 0;

    rssi_val_total = 0;

    //�ȼ����ܵ�ƽ��ֵ
    for (i = 0; i < result_num; i++)
    {
        print_log("ber: %d rssi: %d", g_ber_control->ber_value[i], g_ber_control->rssi_value[i]);
        
        ber_val_total += g_ber_control->ber_value[i];

        rssi_val_total += g_ber_control->rssi_value[i];
        
        div_val++;
    }

    ber_val = (ber_val_total / div_val);

    rssi_val = (rssi_val_total / div_val);

    invalid_data_flag = FALSE;

    max_diff_val = 0;

    while (1)
    {
        //�жϲ�������ɢ�̶�,���ҳ���ɢ�̶Ⱥܴ�ĵ�
        for (i = 0; i < result_num; i++)
        {
            if (g_ber_control->ber_value[i] != INVALID_BER_VAL)
            {
                diff_val = libc_abs(g_ber_control->ber_value[i] - ber_val);

                if (diff_val > max_diff_val)
                {
                    max_diff_index = i;
                    max_diff_val = diff_val;
                }
            }
        }

        //�ж���ɢ�̶����ĵ��Ƿ񳬹����ƣ�����������ƣ��޳��õ㣬�ظ�������һ��
        if (max_diff_val > MAX_BER_DIFF_VAL)
        {           
            print_log("INVALID ber[%d]: %d", max_diff_index, g_ber_control->ber_value[max_diff_index]);

            //��Ǹõ�Ϊ��Ч��
            g_ber_control->ber_value[max_diff_index] = INVALID_BER_VAL;

            invalid_data_flag = TRUE;

            max_diff_val = 0;

            //���¼���һ��ƽ��ֵ
            ber_val_total = 0;

            div_val = 0;

            rssi_val_total = 0;

            for (i = 0; i < result_num; i++)
            {     
                if (g_ber_control->ber_value[i] != INVALID_BER_VAL)
                {
                    ber_val_total += g_ber_control->ber_value[i];

                    rssi_val_total += g_ber_control->rssi_value[i];
                                 
                    div_val++;
                }
            }  

            ber_val = (ber_val_total / div_val);  

            rssi_val = (rssi_val_total / div_val);
        }
        else
        {
            break;
        }
    }

    //������Ч����Ҫ���¼���cfoƽ��ֵ
    if (invalid_data_flag == TRUE)
    {
        ber_val_total = 0;

        div_val = 0;

        for (i = 0; i < result_num; i++)
        {
            if (g_ber_control->ber_value[i] != INVALID_BER_VAL)
            {
                ber_val_total += g_ber_control->ber_value[i];
                rssi_val_total += g_ber_control->rssi_value[i];
                div_val++;
            }
            else
            {
                continue;
            }
        }

        //����Ҫ��8���¼ֵ
        if (div_val < MAX_BER_DIFF_VAL)
        {
            print_log("BER record not enough: %d", div_val);
            
            *p_ber_val = ber_val;

            *p_rssi_val = rssi_val;
            
            //���ֶ���ber�쳣ֵ����ʾ���β��Խ���쳣
            return FALSE;
        }

        ber_val = (ber_val_total / div_val);

        rssi_val = (rssi_val_total / div_val);
    }

    *p_ber_val = ber_val;

    *p_rssi_val = rssi_val;    

    return TRUE;
}

void ber_clear_report(void)
{
    send_bt_data(clear_report_cmd0, sizeof(clear_report_cmd0));
    send_bt_data(clear_report_cmd1, sizeof(clear_report_cmd1));  
    send_bt_data(clear_report_cmd2, sizeof(clear_report_cmd0));
    send_bt_data(clear_report_cmd3, sizeof(clear_report_cmd1));                          
    g_ber_control->tick_cnt = 0;
    g_ber_control->cmd_state = BER_CMD_STATE_DELAY;  
}

int32 ber_test_loop_deal(ber_test_arg_t * ber_arg)
{
    uint32 ber_val;
    uint32 rssi_val;
    uint32 old_state = 0;
    int32 ret_val;

    /** RX��ض�ʱ��
    */
    g_ber_control->get_pktinfo_timer = sys_set_irq_timer1(_ber_test_get_pktinfo_cb, 50);
    

    /** ѭ������Ƿ���Ҫstop
    */
    while(1)
    {
        if(g_ber_control->ber_test_state == BER_STATE_FINISH)
        {       
            ret_val = att_parse_ber_result(PKT_INFO_COUNT, &ber_val, &rssi_val);

            //�жϼ�¼�����Ƿ񹻽����Ϣ
            if(ret_val == FALSE)
            {
                //�����¼���㣬Ҫ���½���ber��ֵ�Ļ�ȡ
                if(g_ber_control->test_cnt >= MAX_BER_RETRY_CNT)
                {          
                    print_log("ber_test fail:%d", ber_val);
                    ret_val = FALSE;
                    break;
                    
                }
                else
                {
                    print_log("ber_val:%d cnt: %d", ber_val, g_ber_control->test_cnt); 
                
                    g_ber_control->pkt_info_count = 0;
                    g_ber_control->pkt_info.result = 0;
                    libc_memset(g_ber_control->ber_value, 0, PKT_INFO_COUNT);  
                    libc_memset(g_ber_control->rssi_value, 0, PKT_INFO_COUNT);                    
                
                    g_ber_control->ber_test_state = BER_STATE_START;
                    g_ber_control->rev_valid_pkt_flag = FALSE;
                    g_ber_control->rev_pkt_flag = FALSE;

                    ber_clear_report();  
                }
            }
            else
            {
                if(ber_val < ber_arg->ber_thr_low || ber_val > ber_arg->ber_thr_high)
                {
                    //����ź�ǿ�ȱȽϺã������Գ�����ber�ϲ����Ϊ����ʧ��
                    //������Ҫ����Լ���
                    if((g_ber_control->test_cnt >= MAX_BER_RETRY_CNT)
                        || (rssi_val > 0x28))
                    {          
                        print_log("ber_test fail:%d", ber_val);
                        ret_val = FALSE;
                        break;
                        
                    }
                    else
                    {
                        print_log("ber_val:%d cnt: %d", ber_val, g_ber_control->test_cnt); 

                        g_ber_control->pkt_info_count = 0;
                        g_ber_control->pkt_info.result = 0;
                        libc_memset(g_ber_control->ber_value, 0, PKT_INFO_COUNT);  
                        libc_memset(g_ber_control->rssi_value, 0, PKT_INFO_COUNT);                    

                        g_ber_control->ber_test_state = BER_STATE_START;
                        g_ber_control->rev_valid_pkt_flag = FALSE;
                        g_ber_control->rev_pkt_flag = FALSE;

                        ber_clear_report();
                    }
                }
                else
                {
                    print_log("ber_test success:%d", ber_val);

                    ret_val = TRUE;
                    break;

                }
            }
        }
        else
        {
            if(g_ber_control->pkt_info_count < PKT_INFO_COUNT)
            {              
                if(old_state != g_ber_control->cmd_state)
                {
                    //libc_print("cmd state", g_ber_control->cmd_state, 2);
                    old_state = g_ber_control->cmd_state;
                }

                if(g_ber_control->cmd_state == BER_CMD_STATE_WRITE_PACKET_NUM)
                {
                    send_bt_data(pkt_num_cmd, sizeof(pkt_num_cmd));
                    g_ber_control->cmd_state = BER_CMD_STATE_READ_PACKET_NUM;
                }
                else if(g_ber_control->cmd_state == BER_CMD_STATE_WRITE_ERROR_BIT)
                {
                    send_bt_data(error_bit_cmd, sizeof(error_bit_cmd));  
                    g_ber_control->cmd_state = BER_CMD_STATE_READ_ERROR_BIT;
                }
                else if(g_ber_control->cmd_state == BER_CMD_STATE_WRITE_RSSI)
                {
                    send_bt_data(grssi_cmd, sizeof(grssi_cmd));
                    g_ber_control->cmd_state = BER_CMD_STATE_READ_RSSI;
                }
                else if(g_ber_control->cmd_state == BER_CMD_STATE_WRITE_CLEAR_REPORT)
                {
                    //work 3s delay 3s
                    //�յ�������ֵ��pkt��Ҫ��clear����
                    if(g_ber_control->pkt_info.result  < 0x8000)
                    {
                        sys_mdelay(RECV_PKT_REPORT_INTERVAL);
                        g_ber_control->cmd_state = BER_CMD_STATE_WRITE_PACKET_NUM;            
                    }
                    else
                    {                              
                        ber_clear_report();    
                    }
                }
                else 
                {
                    if(g_ber_control->tick_cnt == 20 && g_ber_control->rev_pkt_flag == TRUE)  
                    {
                        g_ber_control->cmd_state = BER_CMD_STATE_WRITE_PACKET_NUM; 
                        g_ber_control->tick_cnt = 0;
                        g_ber_control->rev_valid_pkt_flag = FALSE;
                    }                

                    //����10sδ�յ�event��Ӧ������Ϊ����ʧ��
                    if(g_ber_control->tick_cnt == 200 && g_ber_control->rev_valid_pkt_flag == FALSE)
                    {
                        libc_print("test timeout:", g_ber_control->cmd_state, 2);
                        ret_val = FALSE;
                        break;
                    }

                }
            }
        }
    } 

    /** ɾ��RX��ض�ʱ��
    */
    if(g_ber_control->get_pktinfo_timer != -1)
    {
        sys_del_irq_timer1(g_ber_control->get_pktinfo_timer);
        g_ber_control->get_pktinfo_timer = -1;
    }

    sys_drv_uninstall(DRV_GROUP_BT);  

    return ret_val;
}




