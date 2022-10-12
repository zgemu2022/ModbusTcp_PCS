#ifndef _YX_DEFINE_H_
#define _YX_DEFINE_H_

#define setbit(x, y) x |= (1 << y)	//将X的第Y位置1
#define clrbit(x, y) x &= ~(1 << y) //将X的第Y位清0
//遥信
// posWord=0;

#define u16_InvRunState1 0		 //变流运行状态1 1200
#define st_FlagSciSystemState1 1 //变流器状态字1 1201

// u16_InvRunState1（变流运行状态1）遥信第一个寄存器1200
#define bPcsStoped 0	 //停机，传输到EMS
#define bSavingStatus 1	 //节能方式运行，传输到EMS
#define bStandbyStatus 2 //待机 暂时不加
//#define                       3  //预留
#define bUpWithSoftware 4 //软件启动 暂时不加
//#define                       5   //预留
#define bFaultStatus 6 //故障
//#define   预留                7   //
//#define   预留                8   //
#define bFffLineRunning 9 //离网运行 传给EMS(仅适用于VSG模式)
#define bMergeCircuit 10  //并网运行 传给EMS(仅适用于VSG模式)
//#define   预留               11   //
//#define   预留               12  //
#define bSelfTesting 13 //自检，暂时不加
#define bPcsRunning 14	//运行 传给EMS及PLC
//#define   预留               15  //
// posWord=1;
// st_FlagSciSystemState1（变流器状态字1）1201
//#define                       0  //预留
#define bConnectMode 1 //连接方式 远程/本地[0:本地,1:远程]
//#define   预留                2  //
//#define   预留                3  //

#define bAskMergeOn 4  //"PCC并网开关申请闭合信号//0：无效；1：申请闭合"
#define bAskMergeOff 5 //"PCC并网开关申请断开信号//0：无效；1：申请断开"
//#define   预留                6~15 //

// u16_SetPCSRunModeMoni （PQ实际工作模式)1204
// posWord=4;
#define bConstPwDischargeMode 0 // Bit0	恒功率放电；
#define bConstPwChargeMode 1	// Bit1	恒功率充电；
// Bit2	预留
#define bConstCurCharging 3 // Bit3	恒流充电；
// Bit4-5	预留
#define bConstCurDischarging 6 // Bit6	恒流放电；
// Bit7-15	预留

//故障信号遥信
// 名称或代码显示原则：外部原因产生的故障就显示名称；内部产生的显示代码。
// st_FlagInput 输入状态  第一个寄存器地址为0x1205
#define bRestartUnit 0		   // bit0  变流器1-累计故障  故障代码：0   显示故障名称
#define bLocalStop 1		   // bit1  变流器1-本地启动/关机[1/0: 启动/关机]  故障代码：1   显示名称
#define EMERG_STOP_D 2		   // bit2  变流器1-急停[1/0: 按下/旋起]  故障代码：2   显示故障名称
#define bIGBTFanFault 3		   // bit3  变流器1-设备内部故障3  故障代码：3   显示代码
#define bACLightning 4		   // bit4  变流器1-设备内部故障4  故障代码：4   显示代码
#define bDCLightning 5		   // bit5  变流器1-设备内部故障5  故障代码：5   显示代码
#define bBreaker1Fdb 6		   // bit6  变流器1-断路器1状态反馈  故障代码：6   显示名称
#define bBreaker2Fdb 7		   // bit7  变流器1-断路器2状态反馈  故障代码：7   显示名称
#define bBMSFaultIO 8		   // bit8  变流器1-BMS_IO输入故障  故障代码：8   显示故障名称
#define bDcRelayFault 9		   // bit9  变流器1-设备内部故障9  故障代码：9  显示代码
#define bAcMainContactFault 10 // bit10  变流器1-设备内部故障10  故障代码：10  显示代码
//#define 预留  11~15 //bit11~15  预留  故障代码：11~15  /

// st_FlagSystemFault 系统故障字  寄存器地址为0x1206
#define bInvVoltSampFault 0		// bit0 变流器1-设备内部故障100  故障代码：100  显示代码
#define bInvPhaseSeqFault 1		// bit1 变流器1-逆变相序故障   故障代码：101  显示故障名称
#define bInverterLockFault 2	// bit2 变流器1-设备内部故障102   故障代码：102   显示代码
#define bBreaker1Fault 3		// bit3 变流器1-直流断路器1故障  故障代码：103 显示故障名称
#define bBreaker2Fault 4		// bit4 变流器1-直流断路器2故障  故障代码：104   显示故障名称
#define bAcVoltSampFault 5		// bit5 变流器1-设备内部故障105  故障代码：105 显示代码
#define bDcVoltSampFault 6		// bit6 变流器1-设备内部故障106    故障代码：106 显示代码
#define bAcCurrentSensorFault 7 // bit7 变流器1-设备内部故障107 故障代码：107  显示代码
#define bDcCurrentSensorFault 8 // bit8 变流器1-设备内部故障108  故障代码：108   显示代码
#define bBatt1Reverse 9			// bit9 变流器1-电池组1反接  故障代码：109   显示故障名称
#define bGridContactorFault 10	// bit10 变流器1-并网接触器故障 故障代码：110  显示故障名称
#define bEEPROMFault 11			// bit11 变流器1-设备内部故障111 故障代码：111 显示代码
#define bSomkeSen 12			// bit12 变流器1-烟感动作告警  故障代码：112  显示故障名称
#define bHumiCtrWarn 13			// bit13 变流器1-湿度控制告警  故障代码：113  显示故障名称
#define bBatt1VoltHi 14			// bit14 变流器1-BATT1过压  故障代码：114  显示故障名称
#define bBatt1VoltLow 15		// bit15 变流器1-BATT1欠压  故障代码：115 显示故障名称

// st_FlagSystemFaultExt1  系统故障扩展字1  寄存器地址为0x1207
#define bCANCommFault 0			   // bit0 变流器1-设备内部故障200 故障代码：200 显示代码
#define bLCDCommFault 1			   // bit1 变流器1-LCD通信故障 故障代码：201 显示故障名称
#define bBMSCommFault 2			   // bit2 变流器1-BMS_485通信故障 故障代码：202 显示故障名称
#define bMasterDspCpldComFault 3   // bit3 变流器1-设备内部故障203 故障代码：203 显示代码
#define bMasterAuthorizeTrunOff 4  // bit4 变流器1-主机授权关机 故障代码：204 显示故障名称
#define bSOCTurnOff 5			   // bit5 变流器1-SOC关机 故障代码：205 显示故障名称
#define bPhaseToPEShort 6		   // bit6 变流器1-相对地短路故障 故障代码：206 显示故障名称
#define bGridPhaseSeqFault 7	   // bit7 变流器1-电网相序错误 故障代码：207 显示故障名称
#define bGridCurrUnbalance 8	   // bit8 变流器1-设备内部故障208 故障代码：208 显示代码
#define bSystemAddrClash 9		   // bit9 变流器1-系统存在地址冲突 故障代码：209 显示故障名称
#define bAddrTypeSetError 10	   // bit10 变流器1-模块地址或机型设置异常 故障代码：210 显示故障名称
#define bGridResonateTrunOff 11	   // bit11 变流器1-电网谐振关机 故障代码：211 显示故障名称
#define bAcCurrDcVauleOverFault 12 // bit12 变流器1-设备内部故障212 故障代码：212 显示代码
#define bE2promInfoError 13		   // bit13 变流器1-机型、序列号、版本信息参数异常 故障代码：213 显示故障名称
#define bProductNotAuthorized 14   // bit14 变流器1-产品未被授权 故障代码：214 显示故障名称
#define bBatt2Reverse 15		   // bit15 变流器1-电池组2反接 故障代码：215 显示故障名称

// st_FlagSystemFaultExt2  系统故障扩展字2  寄存器地址为0x1208
#define bGridRmsHi 0	// bit0 变流器1-电网有效值高 故障代码：300 显示故障名称
#define bGridRmsLow 1	// bit1 变流器1-电网有效值低 故障代码：301 显示故障名称
#define bGridFreHi 2	// bit2 变流器1-电网频率高 故障代码：302 显示故障名称
#define bGridFreLow 3	// bit3 变流器1-电网频率低 故障代码：303 显示故障名称
#define bDcBusHi 4		// bit4 变流器1-DCBUS过高 故障代码：304 显示故障名称
#define bDcBusLow 5		// bit5 变流器1-DCBUS过低 故障代码：305 显示故障名称
#define bBatt2VoltHi 6	// bit6 变流器1-BATT2过压 故障代码：306 显示故障名称
#define bBatt2VoltLow 7 // bit7 变流器1-BATT2欠压 故障代码：307 显示故障名称
//#define 预留 8~11 bit8~11 预留 故障代码：308~311
#define bBatt1VoltSampFault 12 // bit12 变流器1-设备内部故障312 故障代码：312 显示代码
#define bBatt2VoltSampFault 13 // bit13 变流器1-设备内部故障313 故障代码：313 显示代码
#define bDCPosToEarth 14	   // bit14 变流器1-DC正极接地 故障代码：314 显示故障名称
#define bDCNegToEarth 15	   // bit15 变流器1-DC负极接地 故障代码：315 显示故障名称

// st_FlagTempFault  温度故障状态  寄存器地址为0x1209
#define bOverTempEnv 0			  // bit0 变流器1- 环境温度过高 [0/1: 正常/故障] 故障代码：400  显示故障名称
#define bOverTempReactor 1		  // bit1 变流器1-设备内部故障401 故障代码：401  显示代码
#define bOverTempIGBTA 2		  // bit2 变流器1-设备内部故障402 故障代码：402  显示代码
#define bOverTempIGBTB 3		  // bit3 变流器1-设备内部故障403 故障代码：403  显示代码
#define bOverTempIGBTC 4		  // bit4 变流器1-设备内部故障404 故障代码：404  显示代码
#define bIGBTATempSensorFault 5	  // bit5 变流器1-设备内部故障405 故障代码：405  显示代码
#define bIGBTBTempSensorFault 6	  // bit6 变流器1-设备内部故障406 故障代码：406  显示代码
#define bIGBTCTempSensorFault 7	  // bit7 变流器1-设备内部故障407 故障代码：407  显示代码
#define bRetOverTempAlarm 8		  // bit8 变流器1-设备内部故障408 故障代码：408  显示代码
#define bReactorTempSensorFault 9 // bit9 变流器1-设备内部故障409 故障代码：409  显示代码
#define bLowTempEnv 10			  // bit10 变流器1-环境温度过低 [0/1: 正常/故障] 故障代码：410  显示故障名称
#define bTempIGBTUnbalance 11	  // bit11 变流器1-设备内部故障411 故障代码：411  显示代码
//#define 预留  12~13 //bit12~13 预留 故障代码：412~413  /
#define bCabineTFTempFault 14	// bit14 变流器1-设备内部故障414 故障代码：414  显示代码
#define bCabineTFSensorFault 15 // bit15 变流器1-设备内部故障415 故障代码：415  显示代码

// st_FlagMasterRecCpldFault1  CPLD故障字1  寄存器地址为0x120A
//#define 预留  0~1  //bit0~1 预留 故障代码：500~501 /
#define bAPhaseOCLimit 2   // bit2 变流器1-设备内部故障502 故障代码：502 显示代码
#define bBPhaseOCLimit 3   // bit3 变流器1-设备内部故障503 故障代码：503 显示代码
#define bCPhaseOCLimit 4   // bit4 变流器1-设备内部故障504 故障代码：504 显示代码
#define bAPhaseOCFault 5   // bit5 变流器1-设备内部故障505 故障代码：505 显示代码
#define bBPhaseOCFault 6   // bit6 变流器1-设备内部故障506 故障代码：506 显示代码
#define bCPhaseOCFault 7   // bit7 变流器1-设备内部故障507 故障代码：507 显示代码
#define bPbusOVFault 8	   // bit8 变流器1-设备内部故障508 故障代码：508 显示代码
#define bNbusOVFault 9	   // bit9 变流器1-设备内部故障509 故障代码：509 显示代码
#define bIGBTAFault 10	   // bit10 变流器1-设备内部故障510 故障代码：510 显示代码
#define bIGBTBFault 11	   // bit11 变流器1-设备内部故障511 故障代码：511 显示代码
#define bIGBTCFault 12	   // bit12 变流器1-设备内部故障512 故障代码：512 显示代码
#define bIGBTATempFault 13 // bit13 变流器1-设备内部故障513 故障代码：513 显示代码
#define bIGBTBTempFault 14 // bit14 变流器1-设备内部故障514 故障代码：514 显示代码
#define bIGBTCTempFault 15 // bit15 变流器1-设备内部故障515 故障代码：515 显示代码

// st_FlagMasterRecCpldFault2  CPLD故障字2  寄存器地址为0x120B
//#define 预留 0~1 //bit0~1 预留 故障代码：600~601 /
#define bEMERG_STOP 2		// bit2 变流器1-急停 故障代码：602 显示故障名称
#define bSYNCIFault 3		// bit3 变流器1-设备内部故障603 故障代码：603 显示代码
#define bUnderVoltBatt1 4	// bit4 变流器1-BATT1欠压故障 故障代码：604 显示故障名称
#define bUnderVoltBatt2 5	// bit5 变流器1-BATT2欠压故障 故障代码：605 显示故障名称
#define bOverCurrentBatt1 6 // bit6 变流器1-BATT1过流故障 故障代码：606 显示故障名称
#define bOverCurrentBatt2 7 // bit7 变流器1-BATT2过流故障 故障代码：607 显示故障名称
#define bOverVoltBatt1 8	// bit8 变流器1-BATT1过压故障 故障代码：608 显示故障名称
#define bOverVoltBatt2 9	// bit9 变流器1-'BATT2过压故障 故障代码：609 显示故障名称
#define bFuse1Fault 10		// bit10 变流器1-熔断器1故障 故障代码：610 显示故障名称
#define bFuse2Fault 11		// bit11 变流器1-熔断器2故障 故障代码：611 显示故障名称
#define bImmersionFault 12	// bit12 变流器1-浸水故障 故障代码：612 显示故障名称
//#define 预留 13~15 //bit13~15 预留 故障代码：613~615 /

// st_FlagMasterRecCpldFault3  CPLD故障字3  寄存器地址为0x120C
//#define 预留 0~1  //bit0~1 预留 故障代码：700~701 /
#define bTwoLevelModeFdbk 2		 // bit2 变流器1-两电平模式反馈 故障代码：702 显示名称 非故障位
#define bGridContactorFB 3		 // bit3 变流器1-并网接触器反馈 故障代码：703 显示名称 非故障位
#define bGateForbidon 4			 // bit4 变流器1-门禁信号 故障代码：704 显示故障名称
#define bSmokeFault 5			 // bit5 变流器1-烟感故障 故障代码：705 显示故障名称
#define bACLightingSwitchNotOn 6 // bit6 变流器1-交流防雷微断未闭合 故障代码：706 显示故障名称
#define bBoardLowPower 7		 // bit7 变流器1-设备内部故障707 故障代码：707 显示代码
#define bPccBreakerFB 8			 // bit8 变流器1-PCC点开关状态反馈 故障代码：708 显示名称 非故障位
//#define 预留 9~15  //bit9~15 预留 故障代码：709~715 /

// st_FlagMasterRecSlave.bit.Fault  CPU2故障字  寄存器地址为0x120D
#define bSlaveMasterDspComFault 0 // bit0 变流器1-设备内部故障800  故障代码：800  显示代码
#define bBatt1CurrentFault 1	  // bit1 变流器1-BATT1过流  故障代码：801  显示故障名称
#define bDcCurrentFault 2		  // bit2 变流器1-直流过流  故障代码：802  显示故障名称
#define bBatt2CurrentFault 3	  // bit3 变流器1-BATT2过流  故障代码：803  显示故障名称
#define bGridCurrentFault 4		  // bit4 变流器1-并网电流过流  故障代码：804  显示故障名称
#define bIzCurrentFault 5		  // bit5 变流器1-设备内部故障805  故障代码：805  显示代码
#define bDcBusUpFault 6			  // bit6 变流器1-设备内部故障806  故障代码：806  显示代码
#define bDcBusUnFault 7			  // bit7 变流器1-设备内部故障807  故障代码：807  显示代码
#define bDcBusFault 8			  // bit8 变流器1-母线过压故障  故障代码：808  显示故障名称
#define bSpllFault 9			  // bit9 变流器1-设备内部故障809  故障代码：809  显示代码
#define bNeutralPointedFault 10	  // bit10 变流器1-设备内部故障810  故障代码：810  显示代码
#define bUnRsponseMasterCom 11	  // bit11 变流器1-设备内部故障811  故障代码：811   显示代码
#define bSlaveDspFaultFlag 12	  // bit12 变流器1-设备内部故障812  故障代码：812  显示代码
#define bGridAmpLockFailed 13	  // bit13 变流器1-设备内部故障813  故障代码：813  显示代码
#define bGridPreSynFailed 14	  // bit14 变流器1-设备内部故障814  故障代码：814  显示代码
#define bPCCPreSynFailed 15		  // bit15 变流器1-设备内部故障815  故障代码：815  显示代码

// st_FlagSysRunCondition  系统运行条件  寄存器地址为0x120E
#define bBattVoltRunOK_N 0	// bit0 变流器1-BATT电压不满足运行 故障代码：900 显示故障名称
#define bGridVoltRunOK_N 1	// bit1 变流器1-电网电压不适合运行 故障代码：901 显示故障名称
#define bGridFreRunOK_N 2	// bit2 变流器1-电网频率不适合运行 故障代码：902 显示故障名称
#define bOverTempDerating 3 // bit3 变流器1-过温降额 故障代码：903 显示故障名称
#define bOverUbusDerating 4 // bit4 变流器1-直流母线过压降额 故障代码：904 显示故障名称
#define bExtend485LCDFall 5 // bit5 变流器1-IO板通讯故障 故障代码：905 显示故障名称
#define bLVRTFlag 6			// bit6 变流器1-低穿标志 故障代码：906 显示名称 非故障位
#define bHVRTFlag 7			// bit7 变流器1-高穿标志 故障代码：907 显示名称 非故障位
#define bInvURunOK_N 8		// bit8 变流器1-逆变电压异常 故障代码：908 显示故障名称
//#define 预留  9~10  //bit9~10 预留 故障代码：909~910 /
#define bAgingTrunOffCtrl 11   // bit11 变流器1-老化关机控制 故障代码：911 显示故障名称
#define bNonRestartInverter 12 // bit12 变流器1-系统存在不可重启故障 故障代码：912 显示故障名称
#define bSysNonExistMaster 13  // bit13 变流器1-系统不存在主机 故障代码：913 显示故障名称
#define bPvRisoChkPass_N 14	   // bit14 变流器1-系统绝缘阻抗检测不通过 故障代码：914 显示故障名称
#define bRemoteTrunOffCtrl 15  // bit15 变流器1-远程关机 故障代码：915 显示名称 非故障位

// st_FlagSystemFaultExt3  系统故障扩展3  寄存器地址为0x120F
#define bBatt1CurrentSensorFault 0 // bit0 变流器1-设备内部故障1000  故障代码：1000  显示代码
#define bBatt2CurrentSensorFault 1 // bit1 变流器1-设备内部故障1001  故障代码：1001  显示代码
#define bBusPrechargeFault 2	   // bit2 变流器1-设备内部故障1002  故障代码：1002  显示代码
#define bSoftStartDelayFault 3	   // bit3 变流器1-设备内部故障1003  故障代码：1003  显示代码
#define bLcdTurnOffCtrl 4		   // bit4 变流器1-LCD启动/关机（0启动/1关机）  故障代码：1004  显示名称
#define bTimeConflictFault 5	   // bit5 变流器1-计划调度时间设置冲突故障  故障代码：1005  显示故障名称
#define bPlanStop 6				   // bit6 变流器1-计划性停机  故障代码：1006  显示名称
#define bBMSCANCommFault 7		   // bit7 变流器1-BMS_CAN通信故障  故障代码：1007  显示故障名称
#define bNotChargeOrDischargr 8	   // bit8 变流器1-BMS_禁止充放电  故障代码：1008  显示故障名称
#define bDisconnectBMS 9		   // bit9 变流器1-BMS_故障停机  故障代码：1009  显示故障名称
#define bPCCSwitchFault 10		   // bit10 变流器1-PCC开关反馈故障  故障代码：1010  显示故障名称
#define bVsgStartConditionOK_N 11  // bit11 变流器1-设备内部故障1011  故障代码：1011  显示代码
#define bSysPresSynFailed 12	   // bit12 变流器1-设备内部故障1012  故障代码：1012  显示代码
#define bNoChargingFault 13		   // bit13 变流器1-BMS_禁止充电  故障代码：1013  显示故障名称
#define bNoDischargingFault 14	   // bit14 变流器1-BMS_禁止放电  故障代码：1014  显示故障名称
#define bEMSCommFault 15		   // bit15 变流器1-EMS通讯故障  故障代码：1015  显示故障名称

#endif