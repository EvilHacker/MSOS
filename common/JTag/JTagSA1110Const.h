
#ifndef _JTagSA1110Const_h_
#define _JTagSA1110Const_h_

static const int nCs0Out = 54;
static const int nCs1Out = 55;
static const int nCs2Out = 56;
static const int nCs3Out = 57;
static const int nCs4Out = 58;
static const int nCs5Out = 59;

static const int rdnWrOut = 61;

static const int nOeOut	 = 72;
static const int nWeOut	 = 73;

static const int d310En	 = 212;

static char pinState[] = 
{
	0,	//	ROM_SEL_IN
	1,	//	nRESET_OUT_OUT
	1,	//	nRESET_IN
	1,	//	TXD_3_IN
	0,	//	TXD_3_OUT
	0,	//	TXD_3_EN
	1,	//	RXD_3_IN
	0,	//	RXD_3_OUT
	0,	//	RXD_3_EN
	1,	//	TXD_2_IN
	0,	//	TXD_2_OUT
	0,	//	TXD_2_EN
	0,	//	RXD_2_IN
	0,	//	RXD_2_OUT
	0,	//	RXD_2_EN
	1,	//	TXD_1_IN
	0,	//	TXD_1_OUT
	0,	//	TXD_1_EN
	1,	//	RXD_1_IN
	0,	//	RXD_1_OUT
	0,	//	RXD_1_EN
	0,	//	UDCp_IN
	0,	//	UDCp_OUT
	0,	//	UDCp_EN
	1,	//	UDCmUDCp_IN
	0,	//	UDCm_IN
	0,	//	UDCm_OUT
	0,	//	UDCm_EN
	0,	//	A0_OUT
	0,	//	A1_OUT
	1,	//	A2_OUT
	0,	//	A3_OUT
	0,	//	A4_OUT
	0,	//	A5_OUT
	0,	//	A6_OUT
	0,	//	A7_OUT
	0,	//	A8_OUT
	0,	//	A9_OUT
	0,	//	A10_OUT
	0,	//	A11_OUT
	0,	//	A12_OUT
	0,	//	A13_OUT
	0,	//	A14_OUT
	0,	//	A15_OUT
	0,	//	A16_OUT
	0,	//	A17_OUT
	0,	//	A18_OUT
	0,	//	A19_OUT
	0,	//	A20_OUT
	0,	//	A21_OUT
	0,	//	A22_OUT
	0,	//	A23_OUT
	0,	//	A24_OUT
	0,	//	A25_OUT
	1,	//	nCS0_OUT
	1,	//	nCS1_OUT
	1,	//	nCS2_OUT
	1,	//	nCS3_OUT
	1,	//	nCS4_OUT
	1,	//	nCS5_OUT
	1,	//	RDY_IN
	0,	//	RD_nWR_OUT
	1,	//	nCAS0_OUT
	1,	//	nCAS1_OUT
	1,	//	nCAS2_OUT
	1,	//	nCAS3_OUT
	1,	//	nRAS0_OUT
	0,	//	nRAS1_OUT
	0,	//	nRAS2_OUT
	0,	//	nRAS3_OUT
	0,	//	nSDCAS_OUT
	0,	//	nSDRAS_OUT
	0,	//	nOE_OUT
	1,	//	nWE_OUT
	0,	//	multiEN_EN
	1,	//	nPCE1_OUT
	1,	//	nPCE2_OUT
	0,	//	nPREG_OUT
	1,	//	nPWAIT_IN
	1,	//	nIOIS16_IN
	0,	//	PSKTSEL_OUT
	0,	//	nPIOW_OUT
	0,	//	nPIOR_OUT
	0,	//	nPWE_OUT
	0,	//	nPOE_OUT
	1,	//	L_FCLK_IN
	0,	//	L_FCLK_OUT
	0,	//	L_FCLK_EN
	1,	//	L_LCLK_IN
	0,	//	L_LCLK_OUT
	0,	//	L_LCLK_EN
	1,	//	L_DD7_IN
	0,	//	L_DD7_OUT
	0,	//	L_DD7_EN
	1,	//	L_DD6_IN
	0,	//	L_DD6_OUT
	0,	//	L_DD6_EN
	1,	//	L_DD5_IN
	0,	//	L_DD5_OUT
	0,	//	L_DD5_EN
	1,	//	L_DD4_IN
	0,	//	L_DD4_OUT
	0,	//	L_DD4_EN
	1,	//	L_DD3_IN
	0,	//	L_DD3_OUT
	0,	//	L_DD3_EN
	1,	//	L_DD2_IN
	0,	//	L_DD2_OUT
	0,	//	L_DD2_EN
	1,	//	L_DD1_IN
	0,	//	L_DD1_OUT
	0,	//	L_DD1_EN
	1,	//	L_DD0_IN
	0,	//	L_DD0_OUT
	0,	//	L_DD0_EN
	1,	//	L_PCLK_IN
	0,	//	L_PCLK_OUT
	0,	//	L_PCLK_EN
	1,	//	L_BIAS_IN
	0,	//	L_BIAS_OUT
	0,	//	L_BIAS_EN
	0,	//	GP0_IN
	0,	//	GP0_OUT
	0,	//	GP0_EN
	1,	//	GP1_IN
	0,	//	GP1_OUT
	0,	//	GP1_EN
	1,	//	GP2_IN
	0,	//	GP2_OUT
	1,	//	GP2_EN
	1,	//	GP3_IN
	0,	//	GP3_OUT
	0,	//	GP3_EN
	1,	//	GP4_IN
	0,	//	GP4_OUT
	0,	//	GP4_EN
	1,	//	GP5_IN
	0,	//	GP5_OUT
	0,	//	GP5_EN
	1,	//	GP6_IN
	0,	//	GP6_OUT
	0,	//	GP6_EN
	1,	//	GP7_IN
	0,	//	GP7_OUT
	0,	//	GP7_EN
	1,	//	GP8_IN
	0,	//	GP8_OUT
	0,	//	GP8_EN
	1,	//	GP9_IN
	0,	//	GP9_OUT
	0,	//	GP9_EN
	1,	//	GP10_IN
	0,	//	GP10_OUT
	0,	//	GP10_EN
	0,	//	GP11_IN
	0,	//	GP11_OUT
	0,	//	GP11_EN
	1,	//	GP12_IN
	0,	//	GP12_OUT
	0,	//	GP12_EN
	1,	//	GP13_IN
	0,	//	GP13_OUT
	0,	//	GP13_EN
	1,	//	GP14_IN
	0,	//	GP14_OUT
	0,	//	GP14_EN
	1,	//	GP15_IN
	0,	//	GP15_OUT
	1,	//	GP15_EN
	0,	//	GP16_IN
	1,	//	GP16_OUT
	1,	//	GP16_EN
	0,	//	GP17_IN
	0,	//	GP17_OUT
	1,	//	GP17_EN
	0,	//	GP18_IN
	1,	//	GP18_OUT
	1,	//	GP18_EN
	0,	//	GP19_IN
	0,	//	GP19_OUT
	0,	//	GP19_EN
	1,	//	GP20_IN
	0,	//	GP20_OUT
	0,	//	GP20_EN
	0,	//	GP21_IN
	0,	//	GP21_OUT
	0,	//	GP21_EN
	1,	//	GP22_IN
	0,	//	GP22_OUT
	0,	//	GP22_EN
	0,	//	GP23_IN
	0,	//	GP23_OUT
	0,	//	GP23_EN
	1,	//	GP24_IN
	0,	//	GP24_OUT
	0,	//	GP24_EN
	1,	//	GP25_IN
	0,	//	GP25_OUT
	0,	//	GP25_EN
	1,	//	GP26_IN
	0,	//	GP26_OUT
	0,	//	GP26_EN
	1,	//	GP27_IN
	0,	//	GP27_OUT
	0,	//	GP27_EN
	0,	//	SMROM_EN_IN
	0,	//	SDCKE0_OUT
	0,	//	SDCLK0_OUT
	0,	//	SDCLK1_OUT
	1,	//	SDCLK1_EN
	1,	//	SDCKE1_OUT
	0,	//	SDCLK2_OUT
	1,	//	D31_0_EN
	0,	//	D31_IN
	0,	//	D31_OUT
	0,	//	D23_IN
	0,	//	D23_OUT
	0,	//	D15_IN
	0,	//	D15_OUT
	0,	//	D7_IN
	0,	//	D7_OUT
	0,	//	D30_IN
	0,	//	D30_OUT
	0,	//	D22_IN
	0,	//	D22_OUT
	0,	//	D14_IN
	0,	//	D14_OUT
	0,	//	D6_IN
	0,	//	D6_OUT
	0,	//	D29_IN
	0,	//	D29_OUT
	0,	//	D21_IN
	0,	//	D21_OUT
	0,	//	D13_IN
	0,	//	D13_OUT
	0,	//	D5_IN
	0,	//	D5_OUT
	0,	//	D28_IN
	0,	//	D28_OUT
	0,	//	D20_IN
	0,	//	D20_OUT
	0,	//	D12_IN
	0,	//	D12_OUT
	0,	//	D4_IN
	0,	//	D4_OUT
	0,	//	D27_OUT
	0,	//	D27_IN
	0,	//	D19_IN
	0,	//	D19_OUT
	0,	//	D11_IN
	0,	//	D11_OUT
	0,	//	D3_IN
	0,	//	D3_OUT
	0,	//	D26_IN
	0,	//	D26_OUT
	0,	//	D18_IN
	0,	//	D18_OUT
	0,	//	D10_IN
	0,	//	D10_OUT
	0,	//	D2_IN
	0,	//	D2_OUT
	0,	//	D25_IN
	0,	//	D25_OUT
	0,	//	D17_IN
	0,	//	D17_OUT
	0,	//	D9_IN
	0,	//	D9_OUT
	0,	//	D1_IN
	0,	//	D1_OUT
	0,	//	D24_IN
	0,	//	D24_OUT
	0,	//	D16_IN
	1,	//	D16_OUT
	0,	//	D8_IN
	0,	//	D8_OUT
	0,	//	D0_IN
	1,	//	D0_OUT
	1,	//	TXD_C_IN
	0,	//	TXD_C_OUT
	0,	//	TXD_C_EN
	1,	//	RXD_C_IN
	0,	//	RXD_C_OUT
	0,	//	RXD_C_EN
	1,	//	SCLK_C_IN
	0,	//	SCLK_C_OUT
	0,	//	SCLK_C_EN
	1,	//	SFRM_C_IN
	0,	//	SFRM_C_OUT
	0,	//	SFRM_C_EN
	1,	//	PWR_EN_OUT
	0,	//	VDD_FAULT_IN
	0	//	BATT_FAULT_IN
};

static const int dataPins [] = 
{	 
	276,//	D0_OUT
	268,//	D1_OUT, 
	260,//	D2_OUT, 
	252,//	D3_OUT, 
	244,//	D4_OUT, 
	236,//	D5_OUT, 
	228,//	D6_OUT, 
	220,//	D7_OUT,
	274,//	D8_OUT, 
	266,//	D9_OUT,
	258,//	D10_OUT,
	250,//	D11_OUT,
	242,//	D12_OUT,
	234,//	D13_OUT,
	226,//	D14_OUT,
	218,//	D15_OUT,
	272,//	D16_OUT,
	264,//	D17_OUT,
	256,//	D18_OUT,
	248,//	D19_OUT,
	240,//	D20_OUT,
	232,//	D21_OUT,
	224,//	D22_OUT,
	216,//	D23_OUT,
	270,//	D24_OUT,
	262,//	D25_OUT,
	254,//	D26_OUT,
	246,//	D27_OUT,
	238,//	D28_OUT,
	230,//	D29_OUT,
	222,//	D30_OUT,
	214 //	D31_OUT
};

#endif // !defined(_JTagSA1110Const_h_)
