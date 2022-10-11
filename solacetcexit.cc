//-----------------------------------------------------------------------------
// TCEXITDEMO
//-----------------------------------------------------------------------------
#include<string.h>

#include "tcsexit.h"
#include "include/solclient/solClientMsg.h"
#include "include/solclient/solClient.h"

#include <stdint.h>


#define VERBOSE

//-----------------------------------------------------------------------------
// Solace Data Structures
//-----------------------------------------------------------------------------
#define FILESTRLEN 200


typedef struct messageCorrelationStruct
{
    struct messageCorrelationStruct *next_p;
    long long unsigned int             msgId;              /**< The message ID. Will contain message number from above so carried across calls*/
    char                               recoveryId[34];     //added one char for\0
    char                               uniqueIdV2[26];

} messageCorrelationStruct_t, *messageCorrelationStruct_pt; /**< A pointer to ::messageCorrelationStruct structure of information. */



typedef struct SolaceThreadVars {

  char                                 host[FILESTRLEN];
  char                                 vpn[FILESTRLEN];
  char                                 client_username[FILESTRLEN];
  char                                 password[FILESTRLEN];
  char                                 topicPrefix[FILESTRLEN];
  int                                  eventQos;
  int                                  payloadBufSize;
  solClient_log_level_t		       appLogLevel;
  solClient_log_level_t	               sdkLogLevel;
  solClient_opaqueContext_pt           context_p;
  //solClient_context_createFuncInfo_t   contextFuncInfo;
  solClient_opaqueSession_pt           session_p;
  //solClient_session_createFuncInfo_t   sessionFuncInfo; 
  FILE                                 *fLog;
  //messageCorrelationStruct_pt          msgMemoryItem_p = NULL;
  messageCorrelationStruct_pt          msgMemoryListHead_p = NULL;
  messageCorrelationStruct_pt          msgMemoryListTail_p = NULL;
  long long unsigned int               msgNumber = 0;

} solaceThreadVar, *solaceThreadVar_p;




//-----------------------------------------------------------------------------
// Byte swap functions
//-----------------------------------------------------------------------------
void eightByteSwap(uint64_t * value){
   *value = (((*value & 0xFF00000000000000) >> 56) | ((*value & 0x00FF000000000000) >> 40) | ((*value & 0x0000FF0000000000) >> 24) | ((*value & 0x000000FF00000000) >>  8) |
             ((*value & 0x00000000FF000000) <<  8) | ((*value & 0x0000000000FF0000) << 24) | ((*value & 0x000000000000FF00) << 40) | ((*value & 0x00000000000000FF) << 56));
}

void fourByteSwap(uint32_t * value){
   *value = (((*value & 0xFF000000) >> 24) | ((*value & 0x00FF0000) >> 8) | ((*value & 0x0000FF00) << 8) | ((*value & 0x000000FF) << 24)); 
}


//-----------------------------------------------------------------------------
// Handle / Log Solace Error
//-----------------------------------------------------------------------------
void handleError ( solClient_returnCode_t rc, const char *errorStr )
{
    solClient_errorInfo_pt errorInfo = solClient_getLastErrorInfo (  );
    solClient_log ( SOLCLIENT_LOG_ERROR,
                    "%s - ReturnCode=\"%s\", SubCode=\"%s\", ResponseCode=%d, Info=\"%s\"",
                    errorStr, solClient_returnCodeToString ( rc ),
                    solClient_subCodeToString ( errorInfo->subCode ), errorInfo->responseCode, 
                    errorInfo->errorStr );
                    
    solClient_resetLastErrorInfo (  );
}


//-----------------------------------------------------------------------------
// copy chars
//-----------------------------------------------------------------------------
void cpContToVar(char * src, char * dest, int srcStart, int srcStop){
        int posn;
        for (posn = srcStart; posn < srcStop; posn++)
        {
            // break at newline         
            if( *(src + posn) == '\n') break;
            if( *(src + posn) == '\t') continue;
            if( *(src + posn) == ' ') continue;

            *(dest++) = *(src + posn);
        }
        // add trailing '0'
        *(dest) = '\0';
}


//-----------------------------------------------------------------------------
// Read Solace TC Exit configuration
//-----------------------------------------------------------------------------
int readSolaceExitConfig(solaceThreadVar_p stv_p){
  
  FILE *fp; // pointer for config file
  char con[FILESTRLEN];
  char key[FILESTRLEN];
  char value[FILESTRLEN];
  
  fp =fopen("/home/mhobbis/solacetcexit_linux/solacetcexit.conf","r");// opening of file
  if (!fp){// checking for error
    //Need to log the error 
    return 1; //error
  }
  
  while (fgets(con, FILESTRLEN, fp)!=NULL)// reading file content
  {
    long unsigned int lineLen = strlen(con);
      
    //if line starts with '#' treat as comment and ignore
    //and ignore lines thatstart with white space
    if( *(con) == '#' || *(con) == '\n' || *(con) == '\t' || *(con) == ' ') continue;
	
    //find the '=' char
    long unsigned int strPosn = 0;
    for (strPosn = 0; strPosn < lineLen; strPosn++)
    {
      if( *(con + strPosn) == '=') break;	
    }

    //int posn = 0;	
    cpContToVar(con, key, 0, strPosn);
	
    //parse the values into vars
    if(strcmp(key, "HOST") == 0)
    {
      cpContToVar(con, stv_p->host, strPosn + 1, lineLen);
    } else if (strcmp(key, "VPN") == 0)
    {
            cpContToVar(con, stv_p->vpn, strPosn + 1, lineLen);
    } else if (strcmp(key, "CLIENT_USERNAME") == 0)
    {
      cpContToVar(con, stv_p->client_username, strPosn + 1, lineLen);
    } else if (strcmp(key, "PASSWORD") == 0)
    {
      cpContToVar(con, stv_p->password, strPosn + 1, lineLen);
    } else if (strcmp(key, "TOPIC_PREFIX") == 0)
    {
      cpContToVar(con, stv_p->topicPrefix, strPosn + 1, lineLen);
    } else if (strcmp(key, "EVENT_QOS") == 0)
    {
      cpContToVar(con, value, strPosn + 1, lineLen);
      stv_p->eventQos = atoi(value);
    }else if (strcmp(key, "APP_LOG_LEVEL") == 0)
    {
      cpContToVar(con, value, strPosn + 1, lineLen);
      if (strcmp(key, "ERROR") == 0) {
        stv_p->appLogLevel = SOLCLIENT_LOG_ERROR;
      } else if (strcmp(key, "DEBUG") == 0) {
        stv_p->appLogLevel = SOLCLIENT_LOG_DEBUG;
      } else {
        stv_p->appLogLevel = SOLCLIENT_LOG_INFO;
      }    
    }else if (strcmp(key, "SDK_LOG_LEVEL") == 0)
    {
      cpContToVar(con, value, strPosn + 1, lineLen);
      if (strcmp(key, "ERROR") == 0) {
        stv_p->appLogLevel = SOLCLIENT_LOG_ERROR;
      } else if (strcmp(key, "DEBUG") == 0) {
        stv_p->appLogLevel = SOLCLIENT_LOG_DEBUG;
      } else {
        stv_p->appLogLevel = SOLCLIENT_LOG_INFO;
      } 
    } else if (strcmp(key, "PAYLOAD_BUF_SIZE") == 0)
    {
      cpContToVar(con, value, strPosn + 1, lineLen);
      stv_p->payloadBufSize = atoi(value);
    } else
    {
      //printf("Found unknown setting in file : unknown=%s\n", key);
    }
  }
  
  
 solClient_log(SOLCLIENT_LOG_INFO, "SOLACETCEXIT :: Config loaded - host = %s, client_username = %s, password = %s, vpn = %s, topic-prefix = %s, QOS = %d, payload buffer size = %d", stv_p->host, stv_p->client_username, stv_p->password, stv_p->vpn, stv_p->topicPrefix, stv_p->eventQos, stv_p->payloadBufSize);
  
  return(0);
  
}


//-----------------------------------------------------------------------------
// Connect to the Solace Broker
//-----------------------------------------------------------------------------
/*****************************************************************************
 * messageReceiveCallback
 *****************************************************************************/
solClient_rxMsgCallback_returnCode_t
messageReceiveCallback ( solClient_opaqueSession_pt opaqueSession_p, solClient_opaqueMsg_pt msg_p, void *user_p )
{
    // Unused
    return SOLCLIENT_CALLBACK_OK;
}


/*****************************************************************************
 * eventCallback
 *****************************************************************************/
void
eventCallback ( solClient_opaqueSession_pt opaqueSession_p,
                solClient_session_eventCallbackInfo_pt eventInfo_p, void *user_p )
{
  solClient_errorInfo_pt errorInfo_p;  
  messageCorrelationStruct_pt correlationInfo = ( messageCorrelationStruct_pt ) eventInfo_p->correlation_p; 
  solaceThreadVar_p stv_p = (solaceThreadVar_p)(user_p); 

  switch ( eventInfo_p->sessionEvent ) {

    case SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT:
      // Non-error events are logged at the INFO level.
      solClient_log ( SOLCLIENT_LOG_DEBUG,
                            "adPubAck_eventCallback() called MESSAGE ACK - %s, message number = %llu, message unique_idv2 = %s, message unique_idv2 = %s \n",
                            solClient_session_eventToString ( eventInfo_p->sessionEvent ), 
                            correlationInfo->msgId,
                            correlationInfo->uniqueIdV2,
                            correlationInfo->recoveryId );
      //need to free this entry once we know they come back in order
      if (correlationInfo->next_p == NULL){   //we are at the end of the list so need to set head and tail to NULL
        stv_p->msgMemoryListHead_p = NULL;
        stv_p->msgMemoryListTail_p = NULL;
        free(correlationInfo);
      
      } else {                                //there are other messages to be tracked just delete this one
        stv_p->msgMemoryListHead_p = correlationInfo->next_p;
        free(correlationInfo); 
      }
      break;
    
    case SOLCLIENT_SESSION_EVENT_UP_NOTICE:
    case SOLCLIENT_SESSION_EVENT_TE_UNSUBSCRIBE_OK:
    case SOLCLIENT_SESSION_EVENT_CAN_SEND:
    case SOLCLIENT_SESSION_EVENT_RECONNECTING_NOTICE:
    case SOLCLIENT_SESSION_EVENT_RECONNECTED_NOTICE:
    case SOLCLIENT_SESSION_EVENT_PROVISION_OK:
    case SOLCLIENT_SESSION_EVENT_SUBSCRIPTION_OK:
    case SOLCLIENT_SESSION_EVENT_VIRTUAL_ROUTER_NAME_CHANGED:
    case SOLCLIENT_SESSION_EVENT_MODIFYPROP_OK:
    case SOLCLIENT_SESSION_EVENT_REPUBLISH_UNACKED_MESSAGES:

      /* Non-error events are logged at the INFO level. */
      solClient_log ( SOLCLIENT_LOG_INFO,
                      "eventCallback() called - %s\n",
                      solClient_session_eventToString ( eventInfo_p->sessionEvent ));
      break;

    case SOLCLIENT_SESSION_EVENT_REJECTED_MSG_ERROR:
      // Non-error events are logged at the INFO level.
      solClient_log ( SOLCLIENT_LOG_ERROR,
                            "adPubAck_eventCallback() called MESSAGE REJECTED - %s, message number = %llu, message unique_idv2 = %s, message unique_idv2 = %s,  \n",
                            solClient_session_eventToString ( eventInfo_p->sessionEvent ), 
                            correlationInfo->msgId,
                            correlationInfo->uniqueIdV2,
                            correlationInfo->recoveryId );
      //need to free this entry once we know they come back in order
      if (correlationInfo->next_p == NULL){   //we are at the end of the list so need to set head and tail to NULL
        stv_p->msgMemoryListHead_p = NULL;
        stv_p->msgMemoryListTail_p = NULL;
        free(correlationInfo);
      
      } else {                                //there are other messages to be tracked just delete this one
        stv_p->msgMemoryListHead_p = correlationInfo->next_p;
        free(correlationInfo); 
      }
      break;
    
    case SOLCLIENT_SESSION_EVENT_DOWN_ERROR:
    case SOLCLIENT_SESSION_EVENT_CONNECT_FAILED_ERROR:
    case SOLCLIENT_SESSION_EVENT_SUBSCRIPTION_ERROR:
    case SOLCLIENT_SESSION_EVENT_RX_MSG_TOO_BIG_ERROR:
    case SOLCLIENT_SESSION_EVENT_ASSURED_DELIVERY_DOWN:
    case SOLCLIENT_SESSION_EVENT_TE_UNSUBSCRIBE_ERROR:
    case SOLCLIENT_SESSION_EVENT_PROVISION_ERROR:
    case SOLCLIENT_SESSION_EVENT_MODIFYPROP_FAIL:

      /* Extra error information is available on error events */
      errorInfo_p = solClient_getLastErrorInfo (  );
      solClient_log ( SOLCLIENT_LOG_ERROR, "common_eventCallback() called - %s; subCode %s, responseCode %d, reason %s\n",
               solClient_session_eventToString ( eventInfo_p->sessionEvent ),
               solClient_subCodeToString ( errorInfo_p->subCode ), errorInfo_p->responseCode, 
               errorInfo_p->errorStr );
      break;

    default:
      /* Unrecognized or deprecated events. */
      solClient_log ( SOLCLIENT_LOG_ERROR, "common_eventCallback() called - %s.  Unrecognized or deprecated event.\n",
                     solClient_session_eventToString ( eventInfo_p->sessionEvent ) );
      break;
  }
}


/*****************************************************************************
 * cleanup
 *****************************************************************************/
void cleanup(){
  solClient_returnCode_t rc = SOLCLIENT_OK;
  if ( ( rc = solClient_cleanup (  ) ) != SOLCLIENT_OK ) {
        handleError ( rc, "solClient_cleanup()" );
  }
}


/*****************************************************************************
 * Connect To Broker
 *****************************************************************************/
int connectToBroker(solaceThreadVar_p stv_p){

  solClient_returnCode_t rc = SOLCLIENT_OK;
  
  /*************************************************************************
   * Create a Context
   *************************************************************************/
  solClient_log(SOLCLIENT_LOG_INFO, "Creating context\n\n" );
  solClient_context_createFuncInfo_t contextFuncInfo = SOLCLIENT_CONTEXT_CREATEFUNC_INITIALIZER;
  solClient_session_createFuncInfo_t sessionFuncInfo = SOLCLIENT_SESSION_CREATEFUNC_INITIALIZER;

  if ( ( rc = solClient_context_create ( SOLCLIENT_CONTEXT_PROPS_DEFAULT_WITH_CREATE_THREAD,
                                         &(stv_p->context_p), &contextFuncInfo, 
                                         sizeof( contextFuncInfo ) ) ) != SOLCLIENT_OK ) {
    solClient_log ( SOLCLIENT_LOG_ERROR, "Context Create -  FAIL\n\n" );
    if ( ( rc = solClient_cleanup (  ) ) != SOLCLIENT_OK ) {
      solClient_log ( SOLCLIENT_LOG_ERROR, "SolClient_cleanup - FAIL\n\n" );
    }
    return(1);
  }


  /*************************************************************************
   * configure and connect the session
   *************************************************************************/

  sessionFuncInfo.rxMsgInfo.callback_p = messageReceiveCallback;
  sessionFuncInfo.rxMsgInfo.user_p = NULL;
  sessionFuncInfo.eventInfo.callback_p = eventCallback;
  sessionFuncInfo.eventInfo.user_p = (void *)stv_p;

  /* Session Properties */
  char           *sessionProps[50];
  int             propIndex = 0;

  sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_HOST;
  sessionProps[propIndex++] = stv_p->host;
  sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_VPN_NAME;
  sessionProps[propIndex++] = stv_p->vpn;
  sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_USERNAME;
  sessionProps[propIndex++] = stv_p->client_username;  
  sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_PASSWORD;
  sessionProps[propIndex++] = stv_p->password;
  sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_CONNECT_RETRIES;
  sessionProps[propIndex++] = "5";
  sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_RECONNECT_RETRIES;
  sessionProps[propIndex++] = "1000";
  sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_GENERATE_SEND_TIMESTAMPS;
  sessionProps[propIndex++] = SOLCLIENT_PROP_ENABLE_VAL;
  sessionProps[propIndex++] = SOLCLIENT_SESSION_PROP_APPLICATION_DESCRIPTION;
  sessionProps[propIndex++] = "Solace Exit Connector for tcVISION";
  sessionProps[propIndex++] = NULL;
  sessionProps[propIndex++] = NULL;
  

  /* Create the Session. */
  if ( ( rc = solClient_session_create ( sessionProps,
                                         stv_p->context_p,
                                         &(stv_p->session_p), &sessionFuncInfo, 
                                         sizeof ( sessionFuncInfo ) ) ) 
                                         != SOLCLIENT_OK ) {
    handleError ( rc, "solClient_session_create()" );
    cleanup();
    return(1);
  }

  /* Connect the Session. */
  if ( ( rc = solClient_session_connect ( stv_p->session_p ) ) != SOLCLIENT_OK ) {
    handleError ( rc, "solClient_session_connect()" );
    cleanup();
    return(1);
  }
  
  solClient_log ( SOLCLIENT_LOG_INFO, "SolaceTCExit connected to Solace Broker" );
  
  return(0);

}


//-----------------------------------------------------------------------------
// Publish to the Solace Broker
//-----------------------------------------------------------------------------
int publishMessage(solaceThreadVar_p stv_p, PHP_SX2_DESC sx2_desc, char * bytes, int length, char * destinationName){

  solClient_returnCode_t rc = SOLCLIENT_OK;

  /* Message */
  solClient_opaqueMsg_pt msg_p = NULL;
  solClient_destination_t destination;


  /* Allocate memory for the message that is to be sent. */
  if ( ( rc = solClient_msg_alloc ( &msg_p ) ) != SOLCLIENT_OK ) {
     handleError ( rc, "solClient_msg_alloc()" );
     return(1);
  }

  /* Set the message delivery mode. */
  if ( ( rc = solClient_msg_setDeliveryMode ( msg_p, SOLCLIENT_DELIVERY_MODE_PERSISTENT ) ) != SOLCLIENT_OK ) {
    handleError ( rc, "solClient_msg_setDeliveryMode()" );
    return(1);
  }

        /* Set the destination. */
  destination.destType = SOLCLIENT_TOPIC_DESTINATION;
  destination.dest = destinationName;
  if ( ( rc = solClient_msg_setDestination ( msg_p, &destination, sizeof ( destination ) ) ) != SOLCLIENT_OK ) {
     handleError ( rc, "solClient_msg_setDestination()" );
    return(1);
  }

        /* Add some content to the message. */
  if ( ( rc = solClient_msg_setBinaryAttachment ( msg_p, bytes, length ) ) != SOLCLIENT_OK ) {
    handleError ( rc, "solClient_msg_setBinaryAttachment()" );
    return(1);
  }
  
  // Need to put published messages on a local queue so that we can log failures to send
  // Failures will result in a log entry only as it is assumed that changes could be replayed
  // if required
  
  
  /*************************************************************************
  * MSG ACK correlation
  *************************************************************************/
  messageCorrelationStruct_pt msgMemoryItem_p = ( messageCorrelationStruct_pt ) malloc ( sizeof ( messageCorrelationStruct_t ) );

  // Store the message information in message memory array.
  msgMemoryItem_p->next_p = NULL;
  strcpy(msgMemoryItem_p->recoveryId, sx2_desc->RecoveryToken);
  strcpy(msgMemoryItem_p->uniqueIdV2, sx2_desc->UniqueId_V2);
  msgMemoryItem_p->msgId = stv_p->msgNumber++;
 

  if ( stv_p->msgMemoryListTail_p != NULL ) {
    stv_p->msgMemoryListTail_p->next_p = msgMemoryItem_p; // need to be in threadvar
  }

  if ( stv_p->msgMemoryListHead_p == NULL ) {
    stv_p->msgMemoryListHead_p = msgMemoryItem_p; // need to be in threadvar
  }

  stv_p->msgMemoryListTail_p = msgMemoryItem_p; // need to be in thread var
  
  if ( ( rc = solClient_msg_setCorrelationTagPtr ( msg_p, msgMemoryItem_p, sizeof ( *msgMemoryItem_p ) ) ) != SOLCLIENT_OK ){
    handleError ( rc, "solClient_msg_setCorrelationTagPtr()" );
    return(1);
  }
  
  /* Send the message. */
  if ( ( rc = solClient_session_sendMsg ( stv_p->session_p, msg_p ) ) != SOLCLIENT_OK ) {
    handleError ( rc, "solClient_session_sendMsg()" );
    return(1);
  }
  

  freeMessage:
  if ( ( rc = solClient_msg_free ( &msg_p ) ) != SOLCLIENT_OK ) {
    handleError ( rc, "solClient_msg_free()" );
    return(1);
  }

  return(0);
}






//-----------------------------------------------------------------------------
// library initialization
//-----------------------------------------------------------------------------
#ifdef _WINDOWS
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  return TRUE;
}
#else
int _init(void) {
  return (TRUE);
}
#endif

//-----------------------------------------------------------------------------
// tcVISION High Performance Exit Initialization
//-----------------------------------------------------------------------------
TCS_EXPORT(int) HP_Initialize(char *ErrorMsg, void **UserData, C_SCRIPT_FUNCTIONS *FunctionTable)
{
  solClient_returnCode_t rc = SOLCLIENT_OK;
  solaceThreadVar_p pSolThVar = (solaceThreadVar_p)malloc(sizeof(solaceThreadVar));

  if (pSolThVar == NULL)
  {
    snprintf(ErrorMsg, 50, "Memory allocation error.");
    return (8);
  };

  if (NULL == UserData)
  {
    snprintf(ErrorMsg, 50, "Call without UserData set.");
    return (8);
  };

  memset(pSolThVar, 0, sizeof(solaceThreadVar));

  *UserData = pSolThVar;
  
  /* set logfile before any other API calls. */
  if ( ( rc = solClient_log_setFile ("/home/mhobbis/tmp/solacetcexitlogfile.log" ) ) != SOLCLIENT_OK ) {
    handleError ( rc, "solClient_initialize()" );
    return(1);
  }
  
  /* solClient needs to be initialized before any other API calls. */
  if ( ( rc = solClient_initialize ( SOLCLIENT_LOG_DEFAULT_FILTER, NULL ) ) != SOLCLIENT_OK ) {
    handleError ( rc, "solClient_initialize()" );
    return(1);
  }
  
  
  solClient_version_info_pt version_p;

  if ( solClient_version_get ( &version_p ) != SOLCLIENT_OK ) {
    solClient_log ( SOLCLIENT_LOG_ERROR, "Unknown library version, solClient_version_get returns FAIL\n\n" );
  } else {
    solClient_log ( SOLCLIENT_LOG_INFO, "CCSMP Version %s (%s)\tVariant: %s\n\n", version_p->version_p, version_p->dateTime_p, version_p->variant_p );
  }
    
  // Read the Solace Config
  if (readSolaceExitConfig(pSolThVar)){
     solClient_log ( SOLCLIENT_LOG_ERROR, "Error reading config file - FAIL" );
     snprintf(ErrorMsg, 50, "Error Reading Config File");
     return(8);
  }
  
  
  solClient_log_setFilterLevel ( SOLCLIENT_LOG_CATEGORY_APP, pSolThVar->appLogLevel );
  solClient_log_setFilterLevel ( SOLCLIENT_LOG_CATEGORY_SDK, pSolThVar->sdkLogLevel );

  // Connect to Solace
  if (connectToBroker(pSolThVar)){
     solClient_log ( SOLCLIENT_LOG_ERROR, "Error connecting to the broker - FAIL" );
     snprintf(ErrorMsg, 50, "Error connecting to the broker.");
     return(8);
  }

  return (0);
}

//-----------------------------------------------------------------------------
// tcVISION High Performance Exit Termination
//-----------------------------------------------------------------------------
TCS_EXPORT(int) HP_Terminate(char *ErrorMsg, void **UserData, C_SCRIPT_FUNCTIONS *FunctionTable)
{
  if (NULL == UserData)
  {
    snprintf(ErrorMsg, 50, "Call without UserData set.");
    return (8);
  }
  else
  {
    solClient_returnCode_t rc = SOLCLIENT_OK;
  
    solaceThreadVar_p stv_p = (solaceThreadVar_p)*UserData;
  
     /* Disconnect the Session. */
    if ( ( rc = solClient_session_disconnect ( stv_p->session_p ) ) != SOLCLIENT_OK ) {
      handleError ( rc, "solClient_session_disconnect()" );
    }

    if (stv_p->fLog)
    {
      fclose(stv_p->fLog);
      //stv_p->fLog = NULL;
    };
    
    /* Cleanup solClient. */
    cleanup();
    
    /* free thread data */
    free(stv_p);
  };

  return (0);
}



//-----------------------------------------------------------------------------
// Process Field
//-----------------------------------------------------------------------------
char * getValueAsJson(PHP_SX2_FLD fld_p, int before_after){

  char * myReturnVal = NULL;
  
  if(fld_p->Field_AI_DataLength > fld_p->Field_BI_DataLength){
    myReturnVal = (char *)malloc(fld_p->Field_AI_DataLength + 100);
  } else {
    myReturnVal = (char *)malloc(fld_p->Field_BI_DataLength + 100);
  }
  
  /*
  tcVISION types.
  Even IDs = Types and follow on odd IDs are the same type but may be nullable.
  
  SQL_DATE_384: ISO-Date (length 10)
  SQL_TIME_388: ISO-Time (length 8)
  SQL_TIMESTP_392: ISO-Timestamp (length 26)
  SQL_BLOB_404: binary large object
  SQL_CLOB_408: character large object
  SQL_DBCLOB_412: database character large object
  SQL_VCHAR_448: varchar, 2 byte length in front
  SQL_CHAR_452: fixed character
  SQL_LVCHAR_456: long varchar, 4 byte length in front
  SQL_NCHAR_460: null terminated character field
  SQL_VGCHAR_464: variable graphic character, 2 byte length in front
  SQL_GCHAR_468: fixed graphic character
  SQL_LVGCHAR_472: long variable graphic character, 4 byte length in front
  SQL_FLOAT_480: float value (length 4)
  SQL_DECIMAL_484: packed decimal
  SQL_BIGINTEGER_492: big integer (length 8)
  SQL_INTEGER_496: integer (length 4)
  SQL_SMALLINT_500: smallint (length 2)
  SQL_VBINARY_908: variable binary, 2 byte length in front
  SQL_BINARY_912: fixed binary
  SQL_BLOB_LOC_960: BLOB locator
  SQL_CLOB_LOC_964: CLOB locator
  SQL_DBCLOB_LOC_968: DBCLOB locator
  SQL_XML_LOC_988: XML locator
  SQL_DECFLOAT_996: DECFLOAT (see https://www.ibm.com/docs/en/db2/11.1?topic=functions-decfloat)
  */

  switch(fld_p->FieldType)
  {
    case 384:
    case 385:
      //ISO-Date - encoded as wchar array - fixed length no null term
    case 388:
    case 389: 
      //ISO-Time - encoded as wchar array - fixed length no null term     
    case 392:
    case 393:
      //ISO-Timestamp encoded as wchar array - fixed length no null term
    case 452:
    case 453:
      //wchar array - fixed length no null term
    case 456:
    case 457:
      //\0 terminated wchar array
      {
        char * rtnPosn = myReturnVal;  
        uint16_t * wideChar;
        int length;

        strcpy(rtnPosn++, "\"");
        if(before_after) {
          wideChar = (uint16_t *)(fld_p->Field_AI_Data);
          length = fld_p->Field_AI_DataLength;
        } else {
          wideChar = (uint16_t *)(fld_p->Field_BI_Data);
          length = fld_p->Field_BI_DataLength;
        }   
        
        int loop;
        for(loop = 0; loop < length ; loop++){
          if(*(wideChar) == 0) break;  
          *(rtnPosn++) = *(wideChar++);
        }

        strcpy(rtnPosn++, "\"");   
        strcpy(rtnPosn++, "\0");   
      }    
      break;
    
    case 480:
    case 481:
      //S390 float value (length 8(?) although its a 4 byte float!!)
      {        
        //get the right value based on before_after. false = before, true = after 
        unsigned char tempStr[200];
        unsigned char * tempStrPtr = &tempStr[0]; 
        
        if(before_after) {
          print_S390_float ((unsigned char *)fld_p->Field_AI_Data, tempStrPtr, fld_p->FieldScale, fld_p->FieldLength); 
        } else {
          print_S390_float ((unsigned char *)fld_p->Field_BI_Data, tempStrPtr, fld_p->FieldScale, fld_p->FieldLength);
        } 
       //remove the leading '+'
       if(*tempStrPtr = '+') tempStrPtr++; 
       strcpy(myReturnVal, (const char*)tempStrPtr);
      }   
      break;

    case 496:
    case 497:
      // Value is byte integer in BE format  
      {
        //get the right value based on before_after. false = before, true = after 
        int value;
        if(before_after) {
          value = *(int*)(fld_p->Field_AI_Data);
        } else {
          value = *(int*)(fld_p->Field_BI_Data);       
        }
        fourByteSwap((uint32_t *)&value);
        sprintf(myReturnVal, "%d", value);
      } 
      break;
                      
    case 484:
    case 485:
      //packed decimal
      {              
        unsigned char tempStr[200];
        unsigned char * tempStrPtr = tempStr;
         
        //get the right value based on before_after. false = before, true = after 
        if(before_after) {
          print_packed_decimal((unsigned char *)fld_p->Field_AI_Data, fld_p->FieldLength, fld_p->FieldScale, tempStrPtr); 
        } else {
          print_packed_decimal((unsigned char *)fld_p->Field_BI_Data, fld_p->FieldLength, fld_p->FieldScale, tempStrPtr); 
        } 
        
       //remove the leading 0s
       while (*tempStrPtr == '0'){ 
         tempStrPtr++;
       } 
       strcpy(myReturnVal, (const char*)tempStrPtr);
        
        
      }   
      break;    
    
    case 988:
    case 989:
      
    case 996:
    case 997:
  
    case 404:
    case 405:
      //binary large object
      
    case 408:
    case 409:
      //character large object
      
    case 412:
    case 413:
      //database character large object
      
    case 448:
    case 449: 
         
    case 460:
    case 461:
    
    case 464:
    case 465:
    
    case 468: 
    case 469:
    
    case 472:
    case 473:

    case 492:
    case 493:  
          
    case 500:
    case 501:
    
    case 504:
    case 505:
    
    case 908:
    case 909:
    
    case 912:
    case 913:
    
    case 960:
    case 961:
    
    case 964:
    case 965:
    
    case 968:
    case 969:

    case 972:
    case 973:

    
    default:
   
      strcpy(myReturnVal, "\"UNKNOWN_VALUE_TYPE_PLACE_HOLDER\"");    
  
  };


  return(myReturnVal);
}


#define EXPECTED_S2_PARAMETER_VERSION 3

TCS_EXPORT(int) HP_ExitS2LogRecordRead(char *ErrorMsg, void **UserData, C_SCRIPT_FUNCTIONS *FunctionTable, PHP_SX2_DESC sx2_desc)
{

  solaceThreadVar_p stv_p;

  if (NULL == UserData)
  {
    snprintf(ErrorMsg, 50, "Call without UserData set.");
    return (8);
  };

  stv_p = (solaceThreadVar_p)*UserData;

  if (NULL == stv_p)
  {
    snprintf(ErrorMsg, 50, "Call without UserData initialized.");
    return (8);
  };
  
  if (sx2_desc->Version != EXPECTED_S2_PARAMETER_VERSION)
  {
    snprintf(ErrorMsg, 50, "Wrong version %d of parameter, %d expected.", sx2_desc->Version, EXPECTED_S2_PARAMETER_VERSION);
    return (8);
  };
    
  solClient_log(SOLCLIENT_LOG_DEBUG, "|Decoding Message|  Message Field Count = %d", sx2_desc->Fields_Count);

  // Very, Very rough JSON Parsing
  int bufSize = stv_p->payloadBufSize;
  char * messageBody = (char *) malloc(bufSize);
  
  char destName[256];
  char * bodyPosn = messageBody;
  char * destPosn = destName;
  //char * limit = bodyPosn + 100000;
  
  strcpy(destPosn, stv_p->topicPrefix);
  destPosn += strlen(stv_p->topicPrefix);
  strcpy(destPosn, "/");
  destPosn++;


  //Event Type (String)
  strcpy(bodyPosn, "{\"Event\":");
  bodyPosn += 9;

  if (sx2_desc->FunctionType == 7)
  {
    switch (sx2_desc->OperationType)
    {
    case 1:
      strcpy(bodyPosn, "\"INSERT\",");
      bodyPosn += 9;
      
      strcpy(destPosn, "INSERT");
      destPosn += 6;     
      
      break;
    case 2:
      strcpy(bodyPosn, "\"UPDATE\",");
      bodyPosn += 9;
            
      strcpy(destPosn, "UPDATE");
      destPosn += 6;     
      
      break;
    case 3:
      strcpy(bodyPosn, "\"DELETE\",");
      bodyPosn += 9;
            
      strcpy(destPosn, "DELETE");
      destPosn += 6;     
      
      break;
    case 4:
      strcpy(bodyPosn, "\"TRUNCATE\",");
      bodyPosn += 11;
            
      strcpy(destPosn, "TRUNCATE");
      destPosn += 8;     
      
      break;
    default:
      solClient_log(SOLCLIENT_LOG_DEBUG, "|Decoding Message|  Unkonwn Operation - bailing out");
      snprintf(ErrorMsg, 50, "unknown operation type (%d)", sx2_desc->OperationType);
      return (8);
    };
  }  
  
  if ((sx2_desc->FunctionType == 2) || (sx2_desc->FunctionType == 4)) {
      strcpy(bodyPosn, "\"COMMIT\",");
      bodyPosn += 9;
            
      strcpy(destPosn, "COMMIT");
      destPosn += 6;     
      
  }
  else if ((sx2_desc->FunctionType == 3) || (sx2_desc->FunctionType == 5)) {
      strcpy(bodyPosn, "\"ROLLBACK\",");
      bodyPosn += 11;
            
      strcpy(destPosn, "ROLLBACK");
      destPosn += 8;     
      
  }
  else if (sx2_desc->FunctionType == 1) {
      strcpy(bodyPosn, "\"STARTTX\",");
      bodyPosn += 10;
            
      strcpy(destPosn, "STARTTX");
      destPosn += 7;       
  }
  
  //Parse message to json
  
  
  //Table field does not appear in starttx or commit 
  //If there is a table name then add it - TableName (String)
  int nameLen = strlen(sx2_desc->TableName);
  if (nameLen !=0){   
    strcpy(bodyPosn, "\"Table\":");
    bodyPosn += 8;

    strcpy(bodyPosn, "\"");
    bodyPosn++;
    strncpy(bodyPosn, sx2_desc->TableName, nameLen);
    bodyPosn += nameLen;
    strcpy(bodyPosn, "\",");
    bodyPosn+=2;
  
    // while we are here complete the destination 
    strcpy(destPosn++, "/");
    strncpy(destPosn, sx2_desc->TableName, nameLen);
    destPosn += nameLen;
  }
  
  strcpy(destPosn++, "\0");
  
  //back to the message body
  
  //TimeStamp
  strcpy(bodyPosn, "\"TimeStamp\":\"");
  bodyPosn += 13;
  strncpy(bodyPosn, sx2_desc->TimeStamp, 26);
  bodyPosn += 26;
  strcpy(bodyPosn, "\",");
  bodyPosn += 2;
  
  //Unique ID & Recovery Token
  strcpy(bodyPosn, "\"UniqueId_V2\":\"");
  bodyPosn += 15;
  strcpy(bodyPosn, sx2_desc->UniqueId_V2);
  bodyPosn += strlen(sx2_desc->UniqueId_V2);
  strcpy(bodyPosn, "\",");
  bodyPosn += 2;
  strcpy(bodyPosn, "\"RecoveryToken\":\"");
  bodyPosn += 17;
  strcpy(bodyPosn, sx2_desc->RecoveryToken);
  bodyPosn += strlen(sx2_desc->RecoveryToken);
  strcpy(bodyPosn, "\"");
  bodyPosn ++;
  
  
  solClient_log(SOLCLIENT_LOG_DEBUG, "Parsed json message header used %d bytes out of %d", 
                bodyPosn - messageBody, bufSize);
  
  if (sx2_desc->FunctionType == 7)
  {
  
    // work through fields
    // start field array
    strcpy(bodyPosn, ",");
    bodyPosn ++;
    strcpy(bodyPosn, "\"Fields\":");
    bodyPosn += 9;
    strcpy(bodyPosn++, "[");
  
    char * valueJsonStr_p = NULL;
  
    int fieldLoop = 0;
    PHP_SX2_FLD fld_p = sx2_desc->Fields;
  
    for (fieldLoop = 0; fieldLoop < sx2_desc->Fields_Count; fieldLoop++){
  
      solClient_log(SOLCLIENT_LOG_DEBUG, 
                    "|Decoding Message Fields|  Message Field Count = %d, loop = %d, FieldType = %d, Field Length = %d, Field BI Data Length = %d, Field AI Data Length = %d",  
                    sx2_desc->Fields_Count, fieldLoop, fld_p->FieldType, fld_p->FieldLength, fld_p->Field_BI_DataLength, fld_p->Field_AI_DataLength);
                  
      // test enough buffer exists for the field + name otherwise allocate more buffer.
      // Add 50 padding for numbers in json format 
      int reqBuf = fld_p->FieldNameLength + fld_p->FieldLength + 50;
      //record offset bytesincase the new pointer is different from the old
      int currentOffset = bodyPosn - messageBody;
      if (((messageBody + bufSize) - bodyPosn) < reqBuf) {
        bufSize = bufSize + (fld_p->FieldLength); // do we need x2 heretoaccount for base64 enc??
        messageBody = (char *) realloc((void *)messageBody, bufSize);
        //set the new pointer
        bodyPosn = messageBody + currentOffset;
        solClient_log(SOLCLIENT_LOG_DEBUG, 
                      "Not enough payload buffer - need to realloc - new buffer size = %d",  
                      bufSize);
      }             
    
      strcpy(bodyPosn, "{\"FieldName\":\"");
      bodyPosn += 14;
      strncpy(bodyPosn, fld_p->FieldName, fld_p->FieldNameLength);
      bodyPosn += fld_p->FieldNameLength;
      strcpy(bodyPosn, "\",");
      bodyPosn+=2;
    
      if (fld_p->Field_BI_Null) {
        strcpy(bodyPosn, "\"BeforeValue\":");
        bodyPosn += 14;
        strcpy(bodyPosn, "null,");
        bodyPosn +=5;
      } else {
        strcpy(bodyPosn, "\"BeforeValue\":");
        bodyPosn += 14;   
        valueJsonStr_p = getValueAsJson(fld_p, 0);
        strcpy(bodyPosn, valueJsonStr_p);
        bodyPosn += strlen(valueJsonStr_p); 
        strcpy(bodyPosn++, ",");
        free(valueJsonStr_p);
      }
    
      if (fld_p->Field_AI_Null) {
        strcpy(bodyPosn, "\"AfterValue\":");
        bodyPosn += 13;
        strcpy(bodyPosn, "null");
        bodyPosn +=4;   
      } else {
        strcpy(bodyPosn, "\"AfterValue\":");
        bodyPosn += 13;   
        valueJsonStr_p = getValueAsJson(fld_p, 1);
        strcpy(bodyPosn, valueJsonStr_p);
        bodyPosn += strlen(valueJsonStr_p); 
        free(valueJsonStr_p);
      }

      strcpy(bodyPosn, "}");
      bodyPosn++;

      if (fieldLoop < (sx2_desc->Fields_Count - 1)){ 
        strcpy(bodyPosn, ",");
        bodyPosn++;
      } 
    
      fld_p = fld_p->next;
    }
  
    // end field array
    strcpy(bodyPosn++, "]");
  }
  // Terminate the message
  strcpy(bodyPosn++, "}");
  strcpy(bodyPosn, "\0");
  
  
  publishMessage(stv_p, sx2_desc, messageBody, (solClient_uint32_t)strlen(messageBody), destName);
  
  
  //if (stv_p->fLog)
  //{  
  solClient_log(SOLCLIENT_LOG_DEBUG, "Message processed - %s", messageBody);  
  //} 
  
  free(messageBody);
  
  //Set return flag to 1 to tell tcVison to stop further processing
  sx2_desc->ReturnFlags = 1;    
  // return 0 - no errors   
  return (0);
  
  
 /* 
  if (stv_p->fLog)
  {  
      solClient_log(SOLCLIENT_LOG_INFO, "Message processed - %s", messageBody);  
      solClient_log(SOLCLIENT_LOG_INFO, "EXIT READ :: Undo/Redo record at '% .26s' for '%s' with %d fields...\n",
      sx2_desc->TimeStamp, sx2_desc->TableId, sx2_desc->Fields_Count);
  } 
  
  if ((sx2_desc->OperationType >= 1) && (sx2_desc->OperationType <= 3))
  {
      // INSERT (1), UPDATE (2) or DELETE (3)

  int i;
  PHP_SX2_FLD pFld = sx2_desc->Fields;

  sx2_desc->Fields_Count_new = sx2_desc->Fields_Count;
  for (i = 0; i < sx2_desc->Fields_Count; i++)
  {
    short sFieldType;
    char  szFieldName[256];

    if (pFld == NULL)
    {
      snprintf(ErrorMsg, 50, "Non matching field count/fields.");
      return (8);
    };

    memcpy(szFieldName, pFld->FieldName, min(pFld->FieldNameLength, sizeof(szFieldName) - 1));
    szFieldName[min(pFld->FieldNameLength, sizeof(szFieldName) - 1)] = '\0';

#ifdef VERBOSE
        if (pUser->fLog)
        {
          write_message(pUser->fLog, "  FieldName:   %s\n", szFieldName);
          write_message(pUser->fLog, "  FieldType:   %d (%d)\n", pFld->FieldType, pFld->FieldType_CSD);
          write_message(pUser->fLog, "  FieldScale:  %d\n", pFld->FieldScale);
          write_message(pUser->fLog, "  FieldLength: %d\n", pFld->FieldLength);
          write_message(pUser->fLog, "  KeySeqNr:    %d\n", pFld->FieldKeySequenceNr);
          write_message(pUser->fLog, "  FieldCCSID:  %d\n", pFld->FieldCCSID);
          write_message(pUser->fLog, "  FieldFlags:  %d\n", pFld->FieldFlags);
          write_message(pUser->fLog, "  Field_BI_Data:\n");
          if (pFld->Field_BI_Data && pFld->Field_BI_DataLength)
            dump_hex_data(pUser->fLog, (unsigned char*)pFld->Field_BI_Data, pFld->Field_BI_DataLength, (char*) "    ");
          write_message(pUser->fLog, "  Field_AI_Data:\n");
          if (pFld->Field_AI_Data && pFld->Field_AI_DataLength)
            dump_hex_data(pUser->fLog, (unsigned char*)pFld->Field_AI_Data, pFld->Field_AI_DataLength, (char*) "    ");
          write_message(pUser->fLog, "\n");
        };
#endif
        if ( !(pFld->FieldFlags & FIELD_NOT_AVAILABLE) )
        {
          // let's do something with the field......
        }

        pFld = pFld->next;
      };
    };
  }
  else if ((sx2_desc->FunctionType == 2) || (sx2_desc->FunctionType == 4)) {
    if (pUser->fLog)
      write_message(pUser->fLog, "COMMIT record at '% .26s'...\n", sx2_desc->TimeStamp);
  }
  else if ((sx2_desc->FunctionType == 3) || (sx2_desc->FunctionType == 5)) {
    if (pUser->fLog)
      write_message(pUser->fLog, "ROLLBACK record at '% .26s'...\n", sx2_desc->TimeStamp);
  }
  else if (sx2_desc->FunctionType == 1) {
    if (pUser->fLog)
      write_message(pUser->fLog, "START TRANSACTION record at '% .26s'...\n", sx2_desc->TimeStamp);
  }


*/



/*
  PUSERDATASTRUCT pUser;
  int nRC = 0;

  // just initialize
  sx2_desc->ReturnFlags = 0;

  if (NULL == UserData)
  {
    snprintf(ErrorMsg, 50, "Call without UserData set.");
    return (8);
  };

  pUser = (PUSERDATASTRUCT)*UserData;

  if (NULL == pUser)
  {
    snprintf(ErrorMsg, 50, "Call without UserData initialized.");
    return (8);
  };

  if (sx2_desc->Version != EXPECTED_S2_PARAMETER_VERSION)
  {
    snprintf(ErrorMsg, 50, "Wrong version %d of parameter, %d expected.", sx2_desc->Version, EXPECTED_S2_PARAMETER_VERSION);
    return (8);
  };

  if (sx2_desc->FunctionType == 7)
  {
    // UNDO-REDO record
    char szStatement[10];

    switch (sx2_desc->OperationType)
    {
    case 1:
      strcpy(szStatement, "INSERT");
      pUser->nInserts++;
      break;
    case 2:
      strcpy(szStatement, "UPDATE");
      pUser->nUpdates++;
      break;
    case 3:
      strcpy(szStatement, "DELETE");
      pUser->nDeletes++;
      break;
    case 4:
      strcpy(szStatement, "TRUNCATE");
      pUser->nTruncates++;
      break;
    default:
      snprintf(ErrorMsg, 50, "unknown operation type (%d)", sx2_desc->OperationType);
      return (8);
    };

#ifdef VERBOSE
    if (pUser->fLog)
    {    
      write_message(pUser->fLog, "Undo/Redo record (%s) at '% .26s' for '%s' with %d fields...\n",
      szStatement, sx2_desc->TimeStamp, sx2_desc->TableId, sx2_desc->Fields_Count);

      write_message(pUser->fLog, "Mat's new text field to prove compile and restart");
    } 
#endif

    if ((sx2_desc->OperationType >= 1) && (sx2_desc->OperationType <= 3))
    {
      // INSERT (1), UPDATE (2) or DELETE (3)

      int i;
      PHP_SX2_FLD pFld = sx2_desc->Fields;

      sx2_desc->Fields_Count_new = sx2_desc->Fields_Count;
      for (i = 0; i < sx2_desc->Fields_Count; i++)
      {
        short sFieldType;
        char  szFieldName[256];

        if (pFld == NULL)
        {
          snprintf(ErrorMsg, 50, "Non matching field count/fields.");
          return (8);
        };

        memcpy(szFieldName, pFld->FieldName, min(pFld->FieldNameLength, sizeof(szFieldName) - 1));
        szFieldName[min(pFld->FieldNameLength, sizeof(szFieldName) - 1)] = '\0';

#ifdef VERBOSE
        if (pUser->fLog)
        {
          write_message(pUser->fLog, "  FieldName:   %s\n", szFieldName);
          write_message(pUser->fLog, "  FieldType:   %d (%d)\n", pFld->FieldType, pFld->FieldType_CSD);
          write_message(pUser->fLog, "  FieldScale:  %d\n", pFld->FieldScale);
          write_message(pUser->fLog, "  FieldLength: %d\n", pFld->FieldLength);
          write_message(pUser->fLog, "  KeySeqNr:    %d\n", pFld->FieldKeySequenceNr);
          write_message(pUser->fLog, "  FieldCCSID:  %d\n", pFld->FieldCCSID);
          write_message(pUser->fLog, "  FieldFlags:  %d\n", pFld->FieldFlags);
          write_message(pUser->fLog, "  Field_BI_Data:\n");
          if (pFld->Field_BI_Data && pFld->Field_BI_DataLength)
            dump_hex_data(pUser->fLog, (unsigned char*)pFld->Field_BI_Data, pFld->Field_BI_DataLength, (char*) "    ");
          write_message(pUser->fLog, "  Field_AI_Data:\n");
          if (pFld->Field_AI_Data && pFld->Field_AI_DataLength)
            dump_hex_data(pUser->fLog, (unsigned char*)pFld->Field_AI_Data, pFld->Field_AI_DataLength, (char*) "    ");
          write_message(pUser->fLog, "\n");
        };
#endif
        if ( !(pFld->FieldFlags & FIELD_NOT_AVAILABLE) )
        {
          // let's do something with the field......
        }

        pFld = pFld->next;
      };
    };
  }
  else if ((sx2_desc->FunctionType == 2) || (sx2_desc->FunctionType == 4)) {
    if (pUser->fLog)
      write_message(pUser->fLog, "COMMIT record at '% .26s'...\n", sx2_desc->TimeStamp);
  }
  else if ((sx2_desc->FunctionType == 3) || (sx2_desc->FunctionType == 5)) {
    if (pUser->fLog)
      write_message(pUser->fLog, "ROLLBACK record at '% .26s'...\n", sx2_desc->TimeStamp);
  }
  else if (sx2_desc->FunctionType == 1) {
    if (pUser->fLog)
      write_message(pUser->fLog, "START TRANSACTION record at '% .26s'...\n", sx2_desc->TimeStamp);
  }
*/
 
}
