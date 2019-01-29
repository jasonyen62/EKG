//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//
//
/*
******************************************************************
* LineData: Source data line for passing parameter.
* ParameterName: Passing seting data by this parameter name.
* ParamNameLen: if ParamNameMode is set to '1', must to seting this value to safety stoage name chars in "ParameterName".
* ParamNameMode:
*				0 = normal mode, passing for specify parameter name.
*				1 = wildcard mode, passing any parameter name from "LineData"(don't specify parameter name),
					read parameter name is saving in "ParameterName" char array.
* ParameterValue: Storage array for passed parameter value.
* MaxValueLength: "ParameterValue" maximum storage bytes leength.
* return: gain parameter value bytes length.
******************************************************************
*/
int GetIniParameterForLine(	const char		*LineData,
								char				*ParameterName,
								unsigned char		ParamNameLen,
								unsigned char		ParamNameMode,
								char				*ParameterValue,
								unsigned char 	MaxValueLength
								)
{
	int a, datalen = strlen(LineData), ParamLen;
	//
	memset( ParameterValue, 0, MaxValueLength);
	//
	for( a  = 0; a < datalen; a++)
	{
		if( (LineData[a] == 0x09) || (LineData[a] == 0x20) )
			continue;
		//
		if( ParamNameMode == 1  )
		{
			unsigned char ii;
			//
			for( ii = 0; ii < ParamNameLen; ii++, a++)
			{
				if(	((LineData[a] >= 'a') && (LineData[a] <= 'z')) || ((LineData[a] >= 'A') && (LineData[a] <= 'Z')) ||
					((LineData[a] >= '0') && (LineData[a] <= '9'))
					)
					ParameterName[ii] = LineData[a];
					
				else
					break;
			}
			if( ii == 0 )
				return 0;
		}
		else
		{
			if( strncmp( (const char *)(LineData+a), (const char *)ParameterName, strlen(ParameterName))  )
				return 0;
			//
			a += strlen(ParameterName);
		}
		for( ; a < datalen; a++)
		{
			if( (LineData[a] == 0x09) || (LineData[a] == 0x20) )
				continue;
			if( LineData[a] != '=' )
				return 0;
			//
			for( a += 1; a < datalen; a++)
			{
				if( (LineData[a] == 0x09) || (LineData[a] == 0x20) )
					continue;
				//
				ParamLen = 0;
				if( LineData[a] == '"' )
				{
					for( a += 1; (a < datalen) && (ParamLen < MaxValueLength); a++, ParamLen++)
					{
						if( (LineData[a] == '"') ||(LineData[a] == 0x0A) || (LineData[a] == 0x0D) )
							break;
						ParameterValue[ParamLen] = LineData[a];
 					}
				}
				else
				{
					for( ; (a < datalen) && (ParamLen < MaxValueLength); a++, ParamLen++)
					{
						if( ((LineData[a] >= 'a') && (LineData[a] <= 'z')) || ((LineData[a] >= 'A') && (LineData[a] <= 'Z')) || ((LineData[a] >= '0') && (LineData[a] <= '9')) )
							ParameterValue[ParamLen] = LineData[a];
						else
							break;
					}
				}
				return ParamLen;
			}
		}
	}
	return 0;
}
//
//
int GetParameterByNameFromFile( const char *PathFile,  const char *ParameterName, char *ParameterValueBuffer, int BufferMaxLen)
{
	FILE	*fp;
	int	LineBufferLen = strlen(ParameterName)+BufferMaxLen+100;
	char	DataLine[LineBufferLen];
	int	rtn = 0;
	//
	if( !(fp = fopen( PathFile, "r")) )
		return 0;
	//
	memset( DataLine, 0, LineBufferLen);
	memset( ParameterValueBuffer, 0, BufferMaxLen);
	//
	while(fgets( DataLine, LineBufferLen, fp)!=NULL)
	{
		if(rtn==0)
			rtn = GetIniParameterForLine( DataLine, (char *)ParameterName, 0, 0, ParameterValueBuffer, BufferMaxLen);
	}
	//
	fclose(fp);
	return rtn;
}
//
//
bool GetParameterNameAndValueFromFile(
												const char		*PathFile,
												char				*ParamNameBuffer,
												unsigned char		ParamNameMaxLen,
												char				*ParameterValueBuffer,
												int				ValueBufferMaxLen
												)
{
	FILE	*fp;
	int	LineBufferLen = ParamNameMaxLen+ValueBufferMaxLen+100;
	char	DataLine[LineBufferLen];
	int	rtn = 0;
	//
	memset( DataLine, 0, LineBufferLen);
	memset( ParamNameBuffer, 0, ParamNameMaxLen);
	memset( ParameterValueBuffer, 0, ValueBufferMaxLen);
	//
	if( !(fp = fopen( PathFile, "r")) )
		return 0;
	//
	memset( DataLine, 0, LineBufferLen);
	if( fgets( DataLine, LineBufferLen, fp) )
		rtn = GetIniParameterForLine( DataLine, (char *)ParamNameBuffer, ParamNameMaxLen, 1, (char *)ParameterValueBuffer, ValueBufferMaxLen);
	//
	fclose(fp);
	return rtn;
}
//
//
