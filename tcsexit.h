//
// tcVISION Callable exits for Field Exit Handler & High Performance Record Exit Handler
//

#ifdef __cplusplus
    #define _EXTERN_C extern "C"
#else
    #define _EXTERN_C
#endif

#ifdef _WINDOWS
    #include <stdio.h>
    #include <stdlib.h>
    #include <windows.h>
    #include <winuser.h>

    #define TCS_EXPORT(returntype) _EXTERN_C __declspec(dllexport) returntype
#endif

#ifdef _UNIX
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <ctype.h>
  #include <time.h>
  #include <sys/timeb.h>
  #include <stdarg.h>

  #define TCS_EXPORT(returntype) _EXTERN_C returntype

  #ifndef TRUE
    #define TRUE 1
    #define FALSE 0
  #endif

  #ifndef min
    #define min(a,b) (((a) < (b)) ? (a) : (b))
  #endif

  #ifndef max
    #define max(a,b) (((a) > (b)) ? (a) : (b))
  #endif

#endif

//Current version 
#define FE_VERSION                    4
#define HP_VERSION                    1
#define RP_VERSION                    1
#define IDM_VERSION                    1

//Script functions available for C HP-Exits
typedef struct 
{
    int  (*Exit_GetVariableBuffer)  (char *name, void **Buffer, int *Buffer_Length);
    int  (*Exit_SetVariableBuffer)  (char *name, void *Buffer, int Buffer_Length);
    int  (*Exit_GetVariableInteger) (char *name, int *IntegerValue);
    int  (*Exit_SetVariableInteger) (char *name, int IntegerValue);
} C_SCRIPT_FUNCTIONS;

#define MAX_TABLE_DEPTH                5    //Max depth of subtables
#ifdef S390
    #define MAX_SUBTABLE_NUMBER        20
#else
    #define MAX_SUBTABLE_NUMBER        250
#endif

//SQL Data Type
#define SQL_DATE_384          384
#define SQL_TIME_388          388
#define SQL_TIMESTP_392       392
#define SQL_BLOB_404          404
#define SQL_CLOB_408          408
#define SQL_DBCLOB_412        412
#define SQL_VCHAR_448         448
#define SQL_CHAR_452          452
#define SQL_LVCHAR_456        456
#define SQL_NCHAR_460         460
#define SQL_VGCHAR_464        464
#define SQL_GCHAR_468         468
#define SQL_LVGCHAR_472       472
#define SQL_FLOAT_480         480
#define SQL_DECIMAL_484       484
#define SQL_BIGINTEGER_492    492
#define SQL_INTEGER_496       496
#define SQL_SMALLINT_500      500
#define SQL_DSLS_504          504
#define SQL_VBINARY_908       908
#define SQL_BINARY_912        912
#define SQL_BLOB_LOC_960      960
#define SQL_CLOB_LOC_964      964
#define SQL_DBCLOB_LOC_968    968
#define SQL_RSLOCATOR_972     972
#define SQL_XML_LOC_988       988
#define SQL_DECFLOAT_996      996

//Field Info for User Field Exit
//***************************************************************
// This structure must match 'USF  DSECT' in TCSEXIT.390!!
//***************************************************************
typedef struct  
{
    short          Version;                     //Input           Version of this structure 
    short          StructureLength;             //Input           Length of this structure
    short          InFieldType;                 //Input           Input Field Type
    short          InFieldCCSID;                //Input           Input CCSID
    short          InFieldScale;                //Input           Input Field Field Scale
    int            InFieldLength;               //Input           Length of field input data in bytes  
    int            InFieldOffset;               //Input           Input Field offset in input record
    int            OutFieldLength;              //Input/Output Output Field Length in Bytes or Digits
    short          OutFieldType;                //Input/Output Output SQL Field Type
    short          OutFieldCCSID;               //Input/Output Output CCSID
    short          OutFieldScale;               //Input/Output Output Field Scale
    short          TableLevel;                  //Input           Table level depth
    unsigned short TableIndex[MAX_TABLE_DEPTH]; //Input           Field indexes in table structures
    short          UserFieldExitNumber;         //Input           User Field exit Number
    short          FieldNameLength;             //Input           Length of Field Name
    char          *FieldName;                   //Input           Pointer to Fieldname
    void          *InFieldData;                 //Input           Pointer to Input Data
    int            InFieldDataLength;           //Input           Length of Input data
    void          *InRecord;                    //Input           Pointer to original input record
    int            InRecordLength;              //Input           Length of original Input record
    void          *OutFieldData;                //Input/Output Pointer to Output Data
    int            OutFieldDataLength;          //Input/Output Length of Output Data
    unsigned short         FieldFlag;                   //Input/Output Output processing flags
            #define FIELD_IS_NULL        0x0001      // Field Value is NULL (may be set on entry and changed on exit)
} FE_FLD_DESC, * PFE_FLD_DESC;

//Field Info for HP_ExitGetLogRecord
typedef struct  
{
    short          Version;                     //Input           Version of this structure 
    short          StructureLength;             //Input           Length of this structure
    short          InputFileNameLength;         //Input           Length of Input File Name
    short          MaxKeyLength;                //Output       Max Length of key field
    short          KeyLength;                   //Output       Actual Length of key field
    short          Filler1;
    int            MaxRecordLength;             //Output       Max Length of a Data Record
    int            RecordLength;                //Output       Actual Length of Data Record
    int            ReturnFlags;                 //Output       Return Flags
    char          *RecordAddress;                //Output       Pointer to Record Data
    char          *KeyAddress;                     //Output       Pointer to Key Data
    char          *InputFileName;                //Input           Pointer to Input File Name
    char           StructureFileName[65];        //Output       Structure File Name
} HP_GLR_DESC, * PHP_GLR_DESC;

//Field Info for RP_Process exit
typedef struct SRP_FLD
{
    struct SRP_FLD *pnext; 
    short          FieldType;
    short          FieldScale;
    int            FieldLength;
    short          FieldCCSID;
    short          FieldKeySequenceNr;
    short          FieldTable;
    short          FieldLevel;
    short          FieldIndex[5];
    short          Field_Null;
    int            Field_DataLength;
    void          *Field_Data;
    short          FieldNameLength;
	  char           FieldName[];
} RP_FLD, * PRP_FLD;

//Info for RP_Process exit
typedef struct  SRP_DESC
{
    short          Version;                     //Input           Version of this structure 
    short          StructureLength;             //Input           Length of this structure
    short          CallNumber;                  //Input           From replication process deinition
    short          DataSourceType;              //Input           
    short          DataType;                    //Input           
    short          FunctionType;                //Input           Function 
    short          OperationType;               //Input           Operation
    char           TimeStamp[26];               //Input           Timestamp
    char           UniqueId_V2[33];             //Input           LUW id
    char           RecoveryToken_V2[33];        //Input           Id of unit of recovery
    char           InputType[50];               //Input           Type of input object
    char           InputTable[128];             //Input           Name of input object
    char           OutputTarget[128];           //Input           Name of output target
    char           OutputTable[128];            //Input           Name of output table
    char           LUW_ID[25];                  //Input           Name of unit of recovery
    unsigned char  ImageType;                   //Input           Type of image data (B=before/A=after)
    void          *pImageData;                  //Input           Pointer to input image data
    int            ImageLength;                 //Input           Length of input image data
    void          *pNewData;                    //Input           Pointer to output data
    short          Fields_Count;                //Input           Field count 
    PRP_FLD        ActField;                    //Input        Pointer to current field
    PRP_FLD        Fields;                      //Input        Pointer to field list 

} RP_DESC, * PRP_DESC;

//Info for IDManagement exit
typedef struct  SIDM_DESC
{
    short          Version;                     //Input           Version of this structure 
    short          StructureLength;             //Input           Length of this structure
    short          CallNumber;                  //Input           From replication process deinition
    short          DataSourceType;              //Input           
    short          DataType;                    //Input           
    short          FunctionType;                //Input           Function 
    short          OperationType;               //Input           Operation
    char           TimeStamp[26];               //Input           Timestamp
    char           UniqueId_V2[33];             //Input           LUW id
    char           RecoveryToken[25];           //Input           Id of unit of recovery
    char           InputType[50];               //Input           Type of input object
    char           InputTable[128];             //Input           Name of input object
    char           OutputTarget[128];           //Input           Name of output target
    char           OutputTable[128];            //Input           Name of output table
    char           LUW_ID[25];                  //Input           Name of unit of recovery
    void          *pKeyData;                    //Input           Pointer to input key data
    int            KeyLength;                   //Input           Length of input key data
    void          *pIdData;                     //Output       Pointer to new ID data
    int            IDLength;                    //Output       Length of  new ID data
    short          IDType;                      //Output       Type of  new ID data

} IDM_DESC, * PIDM_DESC;

//Field Info for HP_ExitS1LogRecordRead
typedef struct  
{
    short          Version;                     //Input           Version of this structure 
    short          StructureLength;             //Input           Length of this structure
    short          DataSourceType;              //Input           Length of Input File Name
    short          DataType;                    //Input           Length of Input File Name
    int            ReturnFlags;                 //Output       Return Flags
    char           TimeStamp[26];               //Input           Timestamp
    char           UniqueId_V2[33];             //Input           LUW id
    char           RecoveryToken[25];           //Input           Id of unit of recovery
    char           TableId[64];                 //Input           Table Id of unit of recovery
    int            FunctionType;                //Input           Function 
    int            OperationType;               //Input           Operation
    int            BI_Length;                   //Input/Output Length of BI Data
    void          *BI_Data;                     //Input/Output Pointer to BI data
    int            AI_Length;                   //Input/Output Length of AI Data
    void          *AI_Data;                     //Input/Output Pointer to AI data
    union
    {
        struct
        {
            short  CKL;                     //Input		  
            char   DB[8];                   //Input		  
            char   Segment[8];              //Input	
        } DLI;
        struct
        {
            unsigned short dbnr;                    //Input		  
            unsigned short finr;                    //Input		  
        } ADA;
    } u;
    int            Out_Count;                   // Exit output conet
    char           TimeStampRaw[8];             //Input           Timestamp in STCK format
    char           Userid[8];                   //Input           
    char           SystemId[8];                 //Input           
    char           NewTableId[128];             //Output
} HP_SX1_DESC, * PHP_SX1_DESC;

//Field Info for HP_ExitS2LogRecordRead
typedef struct SX2_FLD
{
    struct SX2_FLD *next; 
    short          FieldType;
    short          FieldScale;
    int            FieldLength;
    short          FieldType_CSD;
    short          FieldFlags;
        #define FIELD_NOT_AVAILABLE   0x0001
    short          FieldCCSID;
    short          FieldKeySequenceNr;
    short          Field_BI_Null;
    short          Field_AI_Null;
    int            Field_BI_DataLength;
    void          *Field_BI_Data;
    int            Field_AI_DataLength;
    void          *Field_AI_Data;
    short          FieldNameLength;
	char           FieldName[];
} HP_SX2_FLD, * PHP_SX2_FLD;

//Info for HP_ExitS2LogRecordRead
typedef struct  
{
    short          Version;                     //Input           Version of this structure 
    short          StructureLength;             //Input           Length of this structure
    short          DataSourceType;              //Input           Length of Input File Name
    short          DataType;                    //Input           Length of Input File Name
    int            ReturnFlags;                 //Output       Return Flags
    char           TimeStamp[26];               //Input           Timestamp
    char           UniqueId_V2[33];             //Input           LUW id
    char           RecoveryToken[25];           //Input           Id of unit of recovery
    char           TableId[64];                 //Input           Table Id input side
    char           TableName[128];              //Input           Table Id output side
    int            FunctionType;                //Input           Function 
    int            OperationType;               //Input           Operation
    short          Fields_Count;                //Input           Field count 
    short          Fields_Count_new;            //Output       New Field count after user Field/Data modification
    PHP_SX2_FLD    Fields;                      //Input/Output Field list 

} HP_SX2_DESC, * PHP_SX2_DESC;

//Parameter Info for HP_ExitS3LogRecordRead
typedef struct SX3_PRM
{
    struct SX3_PRM *next; 
    char           FieldName[128];
    short          FieldType;
    short          FieldScale;
    short          FieldCCSID;
    short          FieldParamSequenceNr;
    short          FieldWhereParamSequenceNr;
    short          Field_Null;
    int            FieldLength;
    int            Field_DataLength;
    void          *Field_Data;
} HP_SX3_PRM, * PHP_SX3_PRM;

//Info for HP_ExitS3LogRecordRead
typedef struct  
{
    short          Version;                     //Input           Version of this structure 
    short          StructureLength;             //Input           Length of this structure
    short          DataSourceType;              //Input           Length of Input File Name
    short          DataType;                    //Input           Length of Input File Name
    int            CallFlag;                    //Input           Call flag
    int            ReturnFlags;                 //Output       Return Flags
    char           TimeStamp[26];               //Input           Timestamp
    char           UniqueId_V2[33];             //Input           LUW id
    char           RecoveryToken[25];           //Input           Id of unit of recovery
    char           TargetName[128];             //Input           Output target name
    char           TableName[128];              //Input           Output table name
    int            FunctionType;                //Input           Function 
    int            OperationType;               //Input           Operation
    char          *pComment;                    //Input/Output Comment
    int            CommentLength;               //Input/Output Comment Length
    int            MaxCommentLength;            //Input        Maximum comment Length
    char          *pSQLDML;                     //Input/Output SQL DML
    int            SQLDMLLength;                //Input/Output SQL DML Length
    int            MaxSQLDMLLength;             //Input        Maximum SQL DML Length
    short          SQLDMLCCSID;                 //Input        SQL DML codepage id
    short          reserved;
    short          Parms_Count;                 //Input        Parameter count 
    short          Parms_Count_new;             //Output       New parameter count 
    PHP_SX3_PRM    Parms;                      //Input/Output Parameter list 

} HP_SX3_DESC, * PHP_SX3_DESC;

//Info for HP_ExitSegmentationTest
typedef struct  
{
    short          Version;                     //Input           Version of this structure 
    short          StructureLength;             //Input           Length of this structure
    short          DataSourceType;              //Input           Type of data source
    short          DataType;                    //Input           Type of data 
    int            ReturnFlags;                 //Output       Return Flags
    char           TimeStamp[26];               //Input           Timestamp
    char           TableId[64];                 //Input           Table Id input side
    char           TableName[128];              //Input           Table Id output side
    int            FunctionType;                //Input           Function 
    int            OperationType;               //Input           Operation
    int            SegmentRecordsWritten;       //Input        Number of records written to the current segment
    long long      SegmentBytesWritten;         //Input        Number of bytes written to the current segment 

} HP_SEGTEST_DESC, * PHP_SEGTEST_DESC;

//Field Info for HP_ExitBulkRead
typedef struct  
{
    short          Version;                     //Input           Version of this structure 
    short          StructureLength;             //Input           Length of this structure
    short          DataSourceType;              //Input          
    short          DataType;                    //Input          
    int            ReturnFlags;                 //Output      
    int            I_Length;                    //Input/Output Length  of Data
    void          *I_Data;                      //Input        Pointer to Data
    short          DLI_CKL;                     //Input          
    char           DLI_DB[8];                   //Input          
    char           DLI_Segment[8];              //Input    
    char           DataSourceName[65];          //Input for Bulk and Batch compare
} HP_BULK_DESC, * PHP_BULK_DESC;

//Field Info for HP_ExitPoolRead
typedef struct  
{
    short          Version;                     //Input           Version of this structure 
    short          StructureLength;             //Input           Length of this structure
    int            ReturnFlags;                 //Output      
    int            I_Length;                    //Input/Output Length  of Data
    void          *I_Data;                      //Input        Pointer to Data
    void          *RecordType;                  //Input        Type of record i.e. 'VSME' etc
} HP_POOL_DESC, * PHP_POOL_DESC;

//call Info for HP_ExitLUWManager
typedef struct {
    short Version;                      //Input       Version of this structure
    short StructureLength;              //Input       Length of this structure
    short DataSourceType;               //Input       Length of Input File Name
    short DataType;                     //Input       Length of Input File Name
    int   ReturnFlags;                  //Output      Return Flags
    int   Back_Update;                  //Output      back update Flag
    char  RecoveryToken[25];            //Input       Id of unit of recovery
    int   ActualProcessing;             //Input       call time of this exit   
        #define  LMNG_DESC_LUW_CREATE             1
        #define  LMNG_DESC_LUW_EXECUTE_COMMIT     2
        #define  LMNG_DESC_LUW_EXECUTE_ROLLBACK   3
        #define  LMNG_DESC_LUW_EXECUTE_UNKOWN     4
        #define  LMNG_DESC_LUW_DELETE             5
        #define  LMNG_DESC_LUW_ADD_IMPLICITE      6
        #define  LMNG_DESC_LUW_PARALLEL_APPLY     7
    unsigned char StartTimeStamp[8];    // Input        start of the LUW
    unsigned char EndTimeStamp[8];      // Input        end of the LUW
    int      LUW_LogRecs;               // Input   
    int      LUW_LogBytes;              // Input   
    int      LUW_UndoRedoLogRecs;       // Input   
    char     UserFilenameToken[64];     // Input/Output
    union                               // Input   
    {
        struct {
            char            DB2_CorrelationId[12];
            char            DB2_AuthorizationId[8];
            char            DB2_ResourceName[8];  
            char            DB2_ConnectionType[8];
            char            DB2_ConnectionId[8];
        } DB2M_DATA;
        struct {
            char            DB2_AuthorizationId[8];
        } DB2L_DATA;
        struct {
            char            DB2_AuthorizationId[8];
        } DB2V_DATA;
        struct {
            char            IDMS_id[8];
            char            IDMS_area[18];
            char            IDMS_record[18];
            char            IDMS_program[8];
            char            IDMS_task1[4];
            int             IDMS_task2;
            int             IDMS_RUID;
            unsigned short  IDMS_CVNO;
        } IDMS_DATA;
        struct {
            char            ADABAS_Jobname[8];
            char            ADABAS_Userid[8];
        } ADABAS_DATA;
        struct {
            char            DLI_system[8];
            char            DLI_command[4];
            char            DLI_AuthorizationId[8];
            char            DLI_userid[8];
            char            DLI_db[8];
            char            DLI_psb[8];
        } DLI_DATA;
        struct {
            char            CICS_id[8];
            char            CICS_userid[8];
            char            CICS_transaction[4];
            int             CICS_task;
        } CICS_VSAM_DATA;
        struct {
            unsigned int    DATACOM_Run_Unit;
            unsigned int    DATACOM_Tsn;
            char            DATACOM_Job_Name[8];
            char            DATACOM_User_Id[8];
            char            DATACOM_Monitor_Id[12];
        } DATACOM_DATA;
        struct {
            char            ORA_RBA[25];
            char            ORA_SCNc[12];
            int             ORA_UserId;
            int             ORA_AuditSessionId;
            char            ORA_Username[33];
            short           ORA_Thread;
        } ORACLE_DATA;
        struct {
            char            MSSQL_DatabaseName[128]; 
            short           MSSQL_uid;
            short           MSSQL_spid;
        } MSSQL_DATA;
    } u;

} HP_LMNG_DESC, *PHP_LMNG_DESC;

// user data and functions
#define LOGBUFFERSIZE  1000

typedef struct __userdatastruct {
  FILE*              fLog;
  // counters
  unsigned long long ullRunTime;
  unsigned int       nLookups;
  unsigned int       nInserts;
  unsigned int       nUpdates;
  unsigned int       nDeletes;
  unsigned int       nTruncates;
  unsigned int       nErrors;
  unsigned int       nStarttransactions;
  unsigned int       nCommits;
  unsigned int       nRollbacks;
  
  char*              pszBuffer;
} USERDATASTRUCT, *PUSERDATASTRUCT;

// helper functions
void        write_message(FILE *pOutputFile, const char * format, ...);
void        dump_hex_data(FILE* f, unsigned char* data, int len, char* prefix);
int         print_packed_decimal(unsigned char* buf, int nPrecision, int nScale, unsigned char* szOutput);
int         DisplayTo_S390_Float (unsigned char *pbyteChar, unsigned char *pbyteDouble);
int         DisplayTo_S390_Packed (char *pbyteChar, unsigned short usPrecision, unsigned short usScale, unsigned char *pbytePacked, char DecimalCharacter);
int         print_S390_float (unsigned char* pbyteData, unsigned char*  pszOutput, int bDoublePrecision, int OutScale);

