
#ifndef _YC_DEFINE_H_
#define _YC_DEFINE_H_
//遥测数据定义(pcs)
#define DC_Bus_voltage 0  // 0x1100	"母线电压 int16	0.1 V
#define DC_Bus_current 1  // 0x1101	"母线电流 int16	0.1A
#define DC_power_input 2  // 0x1102	"直流功率 int16	0.1 kW  正为放电，负为充电
#define Line_AB_voltage 3 // 0x1103	"电网AB线电压 int16	0.1 V
#define Line_BC_voltage 4 // 0x1104	"电网BC线电压 int16	0.1 V
#define Line_CA_voltage 5 // 0x1105	"电网CA线电压 int16	0.1 V
#define Phase_A_current 6 // 0x1106	"电网A相电流 int16	0.1 A
#define Phase_B_current 7 // 0x1107	"电网B相电流 int16	0.1 A
#define Phase_C_current 8 // 0x1108	"电网C相电流 int16	0.1 A
#define Power_factor 9	  // 0x1109	"功率因数 int16	0.001
#define Frequency 10	  // 0x110A	"电网频率 int16	0.01 Hz
#define Active_power 11	  // 0x110B	"交流有功功率 int16	0.1kW 正为放电，负为充电
#define Reactive_power 12 // 0x110C	"交流无功功率 int16	0.1kVar 正为感性，负为容性
#define Apparent_power 13 // 0x110D	"交流视在功率 int16	0.1kVA

#define DC_Bus_voltage_zj 0		  // 0x1174	  "母线电压"	整机	int16	0.1 V	R
#define DC_Bus_current_zj 1		  // 0x1075    "母线电流"	整机	int16	0.1A	R
#define DC_power_input_zj 2		  // 0x1076    "直流功率"	整机	int16	0.1 kW	R
#define Line_AB_voltage_zj 3	  // 0x1177	"电网AB线电压   整机	int16	0.1 V	R
#define Line_BC_voltage_zj 4	  // 0x1178	"电网BC线电压   整机	int16	0.1 V	R
#define Line_CA_voltage_zj 5	  // 0x1179	"电网CA线电压   整机	int16	0.1 V	R
#define Phase_A_current_zj 6	  // 0x117A	"电网A相电流    整机	int16	0.1 A	R
#define Phase_B_current_zj 7	  // 0x117B	"电网B相电流    整机	int16	0.1 A	R
#define Phase_C_current_zj 8	  // 0x117C	"电网C相电流    整机	int16	0.1 A	R
#define Power_factor_zj 9		  // 0x117D	"功率因数       整机	int16	0.001	R
#define Frequency_zj 10			  // 0x117E	"电网频率       整机	int16	0.01 Hz	R
#define Active_power_zj 11		  // 0x117F	"交流有功功率   整机	int16	0.1kW	R
#define Reactive_power_zj 12	  // 0x1180	"交流无功功率   整机	int16	0.1kVar
#define Apparent_power_zj 13	  // 0x1181	"交流视在功率   整机	int16	0.1kVA	R
#define Line_AB_PCC_voltage_zj 14 // 0x1182	PCC点AB线电压	整机	int16	0.1V	R

#endif