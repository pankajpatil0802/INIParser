#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// module variables
	int				iFoundTopic;
	int				iFoundItem;
	int				iError;
	long				lTopicFilePos;
	char				szTopicHeading[80];
	char				szLastTopicHeading[80];
	char				szItemHeading[80];
	char				szIniLine[160];
	char				szIniPath[160];
	char				szTempPath[160];
	FILE				*pFIniFile;
	FILE				*pFTempIni;

// PPSetPrivateProfileString
// save configuration string
int PPSetPrivateProfileString ( char *pszPath, 
				char *pszTopic, 
				char *pszItem, 
				char *pszValue
				) 
{
	int iItemLength;
	int iValueLength;

	iError = 0;
	szLastTopicHeading[0] = '\0';

	strcpy ( szIniPath, pszPath);

	strcpy ( szTempPath, pszPath);
	strcat ( szTempPath, "temp");

	// add brackets to topic
	strcpy ( szTopicHeading, "[");
	strcat ( szTopicHeading, pszTopic);
	strcat ( szTopicHeading, "]\n");

	strcpy ( szItemHeading, pszItem);
	strcat ( szItemHeading, "=");

	iItemLength = strlen ( szItemHeading);
	iValueLength = strlen ( pszValue);

	iFoundTopic = 0;
	iFoundItem = 0;

	if ( ( pFTempIni = fopen ( szTempPath, "w")) == NULL)
	{
		printf ( "could not open temp ini file to write settings\n");
		iError = 1;
		return ( iError);
	}

	// try to open current config file for reading
	if ( ( pFIniFile = fopen ( szIniPath, "r")) != NULL)
	{
		// read a line from the config file until EOF
		while ( fgets ( szIniLine, 159, pFIniFile) != NULL)
		{
			// the item has been found so continue reading file to end
			if ( iFoundItem == 1) {
				fputs ( szIniLine, pFTempIni);
				continue;
			}
			// topic has not been found yet
			if ( iFoundTopic == 0) {
				if ( strcmp ( szTopicHeading, szIniLine) == 0) {
					// found the topic
					iFoundTopic = 1;
				}
				fputs ( szIniLine, pFTempIni);
				continue;
			}
			// the item has not been found yet
			if ( ( iFoundItem == 0) && ( iFoundTopic == 1))
			{
				if ( strncmp ( szItemHeading, szIniLine, iItemLength) == 0)
				{
					// found the item
					iFoundItem = 1;
					if ( iValueLength > 0)
					{
						fputs ( szItemHeading, pFTempIni);
						fputs ( pszValue, pFTempIni);
						fputs ( "\n", pFTempIni);
					}
					continue;
				}
				// if newline or [, the end of the topic has been reached
				// so add the item to the topic
				if ( ( szIniLine[0] == '\n') || ( szIniLine[0] == '['))
				{
					iFoundItem = 1;
					if ( iValueLength > 0)
					{
						fputs ( szItemHeading, pFTempIni);
						fputs ( pszValue, pFTempIni);
						fputs ( "\n", pFTempIni);
					}
					fputs ( "\n", pFTempIni);
					if ( szIniLine[0] == '[')
					{
						fputs ( szIniLine, pFTempIni);
					}
					continue;
				}
				// if the item has not been found, write line to temp file
				if ( iFoundItem == 0) {
					fputs ( szIniLine, pFTempIni);
					continue;
				}
			}
		}
		fclose ( pFIniFile);
	}
	// still did not find the item after reading the config file
	if ( iFoundItem == 0)
	{
		// config file does not exist
		// or topic does not exist so write to temp file
		if ( iValueLength > 0)
		{
			if ( iFoundTopic == 0)
			{
				fputs ( szTopicHeading, pFTempIni);
			}
			fputs ( szItemHeading, pFTempIni);
			fputs ( pszValue, pFTempIni);
			fputs ( "\n\n", pFTempIni);
		}
	}

	fclose ( pFTempIni);

	//delete the ini file
	remove ( szIniPath);

	// rename the temp file to ini file
	rename ( szTempPath, szIniPath);

	return ( iError);
}

// PPGetPrivateProfileInt
// read configuration string
int PPGetPrivateProfileInt ( char *pszPath, 
				char *pszTopic, 
				char *pszItem, 
				char *pszValue	
				)
{
	int iItemLength;
	int iValueLength;
	char *pcLastCharacter;

	iError = 0;

	strcpy ( szIniPath, pszPath);

	// add brackets to topic
	strcpy ( szTopicHeading, "[");
	strcat ( szTopicHeading, pszTopic);
	strcat ( szTopicHeading, "]\n");

	strcpy ( szItemHeading, pszItem);
	strcat ( szItemHeading, "=");

	iItemLength = strlen ( szItemHeading);

	iFoundTopic = 0;
	iFoundItem = 0;

	// try to open current config file for reading
	if ( ( pFIniFile = fopen ( szIniPath, "r")) != NULL)
	{
		// has the topic been found before
		if ( strcmp ( szLastTopicHeading, szTopicHeading) == 0)
		{
			iFoundTopic = 1;
			fseek ( pFIniFile, lTopicFilePos, SEEK_SET);
		}

		// read a line from the config file until EOF
		while ( fgets ( szIniLine, 159, pFIniFile) != NULL)
		{
			// topic has not been found yet
			if ( iFoundTopic == 0)
			{
				if ( strcmp ( szTopicHeading, szIniLine) == 0)
				{
				// found the topic
				iFoundTopic = 1;
				lTopicFilePos = ftell ( pFIniFile);
				strcpy ( szLastTopicHeading, szTopicHeading);
			}
			continue;
			}
			// the item has not been found yet
			if ( ( iFoundItem == 0) && ( iFoundTopic == 1))
			{
				// if newline or [, the end of the topic has been reached
				// no config value in file yet
				if ( ( szIniLine[0] == '\n') || ( szIniLine[0] == '['))
				{
					iFoundItem = 1;
					break;
				}

				if ( strncmp ( szItemHeading, szIniLine, iItemLength) == 0)
				{
					// found the item
					iFoundItem = 1;
					strcpy ( pszValue, &szIniLine[iItemLength]);
					continue;
				}
			}
		}
		fclose ( pFIniFile);
	}
	// remove trailing comment
	iValueLength = strlen ( pszValue);
	while ( iValueLength) 
	{
		if ( *(pszValue + iValueLength) == '#')
		{
			*(pszValue + iValueLength) = '\0';
		}
		iValueLength--;
	}
	// remove trailing white space
	while ( ( iValueLength = strlen ( pszValue)) > 0)
	{
		pcLastCharacter = ( pszValue + iValueLength - 1);
		if ( ( *pcLastCharacter == ' ')		||
			( *pcLastCharacter == '\n')		||
			( *pcLastCharacter == '\r')		||
			( *pcLastCharacter == '\t')		||
			( *pcLastCharacter == '\v')		||
			( *pcLastCharacter == '\a')		||
			( *pcLastCharacter == '\b')		||
			( *pcLastCharacter == '\f') ) 
		{
			*pcLastCharacter = '\0';
		}
		else 
		{
			break;
		}
	}

	return ( iError);
}

int main ( )
{
	char iniValue[200];

	szLastTopicHeading[0] = '\0'; // initialize

	PPSetPrivateProfileString ( "TESTCONF.INI", "DB_OPTIONS", "MAX_DB_PROCESS_COUNT", "1000");
	PPSetPrivateProfileString ( "TESTCONF.INI", "DB_OPTIONS", "MAX_DB_SIZE_COUNT", "1000");
	PPSetPrivateProfileString ( "TESTCONF.INI", "GEN_OPTIONS", "MAX_INPUT_PROCESS_COUNT", "10000");
	PPSetPrivateProfileString ( "TESTCONF.INI", "GEN_OPTIONS", "MAX_OUTPUT_PROCESS_COUNT", "10000");

	PPGetPrivateProfileInt ( "TESTCONF.INI", "DB_OPTIONS", "MAX_DB_PROCESS_COUNT", iniValue);
	printf ( "%s\n", iniValue);
	PPGetPrivateProfileInt ( "TESTCONF.INI", "DB_OPTIONS", "MAX_DB_SIZE_COUNT", iniValue);
	printf ( "%s\n", iniValue);
	PPGetPrivateProfileInt ( "TESTCONF.INI", "GEN_OPTIONS", "MAX_INPUT_PROCESS_COUNT", iniValue);
	printf ( "%s\n", iniValue);
	PPGetPrivateProfileInt ( "TESTCONF.INI", "GEN_OPTIONS", "MAX_OUTPUT_PROCESS_COUNT", iniValue);
	printf ( "%s\n", iniValue);

	int i = atoi(iniValue);
	i = i*2;
	sprintf(iniValue, "%d",i);

	PPSetPrivateProfileString ( "TESTCONF.INI", "GEN_OPTIONS", "MAX_OUTPUT_PROCESS_COUNT", iniValue);


	return 0;
}
