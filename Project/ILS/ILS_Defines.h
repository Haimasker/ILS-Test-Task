#ifndef ILS_DefinesH
#define ILS_DefinesH

#include "ILS_LoggerStream.h"

/// Макрос записи сообщения в лог.
/// Макрос надо обязательно вызывать в двух парах скобок!
/// \note Пример работы:
/// \code
/// ILS_LOG(( "SOME_FUNC", "f(%f,%d) ", fSmth, iSmth  ));
/// \endcode
#define ILS_LOG(LOG_ARG) {TLoggerStream(this,&ILogger::logOut)LOG_ARG;}
#define ILS_LOG_(PTR, LOG_ARG) {TLoggerStream(PTR,&ILogger::logOut)LOG_ARG;}

/// Макрос записи предупреждения в лог.
/// @ingroup Common
#define ILS_WRN(LOG_ARG)  {TLoggerStream(this,&ILogger::wrnOut)LOG_ARG;}
#define ILS_WRN_(PTR, LOG_ARG)  {TLoggerStream(PTR,&ILogger::wrnOut)LOG_ARG;}

/// Макрос начала секции.
/// Макрос создает try-блок скобку его начала, чтобы в макросе закрытия секции сообщить о наличии исключений в ней.
/// @ingroup Common
#define ILS_SECTB(SECTID, LOG_ARG) {\
	TLoggerStream oSection##SECTID(this,&ILogger::infOut,#SECTID); \
	oSection##SECTID.SectBegin LOG_ARG; \
	oSection##SECTID.Flush();\
	try

/// Макрос начала нумерованной секции.
/// Макрос создает try-блок скобку его начала, чтобы в макросе закрытия секции сообщить о наличии исключений в ней.
/// @ingroup Common
#define ILS_SECTBI(SECTID, INDEX, LOG_ARG) {\
	TLoggerStream oSection##SECTID(this,&ILogger::infOut,#SECTID,INDEX); \
	oSection##SECTID.SectBegin LOG_ARG; \
	oSection##SECTID.Flush();\
	try

/// Макрос окончания секции.
/// @ingroup Common
#define ILS_SECTE(SECTID, LOG_ARG) \
	catch(const std::exception& e)  {wrn("SectException", "Секция %s не завершена из-за: %s", oSection##SECTID.SectId(), e.what());throw;}\
	catch(...) {wrn("SectException", "Секция %s не завершена из-за: %s", oSection##SECTID.SectId(), "unknown");throw;}\
	oSection##SECTID.SectEnd LOG_ARG;\
	}

/// Макрос окончания нумерованной секции.
/// @ingroup Common
#define ILS_SECTEI(SECTID, INDEX, LOG_ARG) \
	catch(const std::exception& e)  {wrn("SectException", "Секция %s не завершена из-за: %s", oSection##SECTID.SectId(), e.what());throw;}\
	catch(...) {wrn("SectException", "Секция %s не завершена из-за: %s", oSection##SECTID.SectId(), "unknown");throw;}\
	oSection##SECTID.SectCheck(#SECTID, INDEX);\
	oSection##SECTID.SectEnd LOG_ARG;\
	}

#endif  // ILS_DefinesH