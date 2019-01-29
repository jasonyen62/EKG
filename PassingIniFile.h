//
//
#ifndef _PASSING_INI_FILE_H_
#define _PASSING_INI_FILE_H_
//
#include <stdio.h>
#include <stdlib.h>
//
//
#define UPS_SETING_INI_FILE_PATH_NAME		"Ups.ini"
//
#define UPS_PARAM_NAME_MAX_LENGTH			sizeof("ShutdownSignalHighMilliSec")
//
/*enum UpsIni_Parameter {
		Ups_NormalModePowerFailSignal = 0,
		Ups_PowerFailPastMinute,
		Ups_ShutdownSignalHighMilliSec,
		//
		Ups_TotalLabel
};
//
//
struct _UPS_PARAM_LIST_ {
	const unsigned char	ParamIndex;
	const char			*ParamName[UPS_PARAM_NAME_MAX_LENGTH+1];
} UpsParamList[Ups_TotalLabel] = {
	{ Ups_NormalModePowerFailSignal,		"NormalModePowerFailSignal" },
	{ Ups_PowerFailPastMinute,			"PowerFailPastMinute" },
	{ Ups_ShutdownSignalHighMilliSec,		"ShutdownSignalHighMilliSec" }
};*/
//
//
int GetIniParameterForLine(
								char				*LineData,
								char				*ParameterName,
								unsigned char		ParamNameLen,
								unsigned char		ParamNameMode,
								char				*ParameterValue,
								unsigned char 	MaxValueLength
								);
//*
//
int GetParameterByNameFromFile(
								const char		*PathFile,
								const char		*ParameterName,
								char				*ParameterValueBuffer,
								int				BufferMaxLen
								);
//
bool GetParameterNameAndValueFromFile(
								const char		*PathFile,
								char				*ParamNameBuffer,
								unsigned char		ParamNameMaxLen,
								char				*ParameterValueBuffer,
								int				ValueBufferMaxLen
								);

//
//*/
//
#endif
//
//

