#include <stdio.h>
#include <time.h>
#include <string.h>
#include "ILS_StdLog.h"

//=============================================================================
// BaseLogger - стандартая реализация основных механизмов Logger-а.
// Рекомендуется наследоваться именно от этого класса при определении
// собственного механизма регистрации хода процесса.
//-----------------------------------------------------------------------------
// Конструктор
BaseLogger::BaseLogger() {
	// По умолчанию ничего не выводим в заголовке
	show_info = 0;
	// Время начала
	bStarted = false;
	// флаг вывода лога в консоль
	bLogToConsole = false;
};
//-----------------------------------------------------------------------------
// Функции интерфейса
// Регистрация сообщений разных типов, каждая функция имеет праметры:
// id  - идентификатор сообщений
// msg - тело сообщения в формате функции printf
// ... - набор данных для вывода в сообщении по принципу printf
// Функции:
// Регистрация информационного сообщения
void BaseLogger::infOut(const Msg& msg, const LogId& id) const {
	lOut(iTitle() + msg);
}
//-----------------------------------------------------------------------------
// Функции интерфейса
// Регистрация сообщений разных типов, каждая функция имеет праметры:
// id  - идентификатор сообщений
// msg - тело сообщения в формате функции printf
// ... - набор данных для вывода в сообщении по принципу printf
// Функции:
// Регистрация информационного сообщения
void BaseLogger::logOut(const Msg& msg, const LogId& id) const {
	lOut(lTitle()+msg);
}
// Регистрация предупреждения (warning) и не фатальной ошибки
void BaseLogger::wrnOut(const Msg& msg, const LogId& id) const {
	++warnings;
	wOut(wTitle()+msg);
}
// Регистрация фатальной ошибки,
// после которой результаты процесса не определены
void BaseLogger::errOut(const Msg& msg, const LogId& id) const {
	++errors;
	eOut(eTitle()+msg);
}
//-----------------------------------------------------------------------------
// Вспомогательные функции
// Создается общая для всех типов сообщений строка с заголовком
// на основе значений настроек
std::string BaseLogger::title() const {
	std::string res = "";
	char s[128];
	time_t ltime;
	time( &ltime );
	struct tm *today;
	today = localtime( &ltime );
	if(show_info & 1) {
		strftime(s, 128, "%Y/%m/%d ", today );
		res += s;
	}
	if((show_info & 2) || ((show_info & 4) && (!bStarted))) {
		strftime(s, 128, "%H:%M:%S ", today );
		res += s;
	}
	if (!bStarted) {
		start_time = std::chrono::steady_clock::now();
		bStarted = true;
	}
	else if (show_info & 4) {
		long fDurationMilli = long(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count());
		sprintf(s, "% 8.2f ", double(fDurationMilli) / 1000.0);
		res += s;
	}
	return res;
}
// Для кажного типа сообзения задается отдельная функция, которая
// реализуется на основе общей
std::string BaseLogger::lTitle() const {
	return title()+"> ";
}
std::string BaseLogger::iTitle() const {
	return title() + "|INFO> ";
}
std::string BaseLogger::wTitle() const {
	return title()+"|WARNING> ";
}
std::string BaseLogger::eTitle() const {
	return title()+"|ERROR> ";
}

/// Конструктор.
/// Конструировние Логгера на потоки вывода библиотеки STL.
/// \param l_out - поток для вывода информационных сообщений.
/// \param w_out - поток для вывода предупреждений.
/// \param e_out - поток для вывода ошибок.
StdLogger::StdLogger(std::ostream& l_out,
					std::ostream& w_out,
					std::ostream& e_out) {
	log_out = &l_out; l_del=false;
	wrn_out = &w_out; w_del=false;
	err_out = &e_out; e_del=false;
	BaseLogger::onLogStart(true,wrn_out!=log_out,err_out!=wrn_out&&err_out!=log_out);
}

/// Конструктор по имени файла для инф. сообщений.
/// Конструировние Логгера на потоки вывода библиотеки STL.
/// \param l_out_file - Имя файла для вывода log информации.
/// \param w_out - поток для вывода предупреждений.
/// \param e_out - поток для вывода ошибок.
///
/// \note Если в качестве имени файла (первого параметра) передается "",
/// это значит, что информационные сообщения должны ингорироваться, а выводится
/// только предупреждения и ошибки.
StdLogger::StdLogger(std::string l_out_file,
					std::ostream& w_out,
					std::ostream& e_out,
					std::ios_base::openmode mode) {
	using namespace std;
	log_out = NULL;l_del=false;;
	if(l_out_file!="") {
		std::ofstream* log_stream = new std::ofstream(l_out_file.c_str(),mode|std::ios_base::out);
		if(*log_stream) {log_out = log_stream;l_del=true;}
		else delete log_stream;
	}
	wrn_out = &w_out; w_del=false;
	err_out = &e_out; e_del=false;
	BaseLogger::onLogStart(true,wrn_out!=log_out,err_out!=wrn_out&&err_out!=log_out);
}

/// Конструктор по имени файла для инф. сообщений и предупрждений.
/// Конструировние Логгера на потоки вывода библиотеки STL.
/// \param l_out_file - Имя файла для вывода log информации.
/// \param w_out_file - Имя файла для вывода предупреждений.
/// \param e_out - поток для вывода ошибок.
///
/// \note Если в качестве имени файла передается "",
/// это значит, что сообщения такого типа должны ингорироваться, а выводится
/// будут только другие сообщения.
StdLogger::StdLogger(std::string l_out_file,
					std::string w_out_file,
					std::ostream& e_out,
					std::ios_base::openmode mode) {
	using namespace std;
	log_out = NULL;l_del=false;;
	if(l_out_file!="") {
		std::ofstream* log_stream = new std::ofstream(l_out_file.c_str(),mode|std::ios_base::out);
		if(*log_stream) {log_out = log_stream;l_del=true;}
		else delete log_stream;
	}
	wrn_out = NULL;w_del=false;;
	if(w_out_file!="") {
		std::ofstream* wrn_stream = new std::ofstream(w_out_file.c_str(),mode|std::ios_base::out);
		if(*wrn_stream) {wrn_out = wrn_stream;w_del=true;}
		else delete wrn_stream;
	}
	err_out = &e_out; e_del=false;
	BaseLogger::onLogStart(true,wrn_out!=log_out,err_out!=wrn_out&&err_out!=log_out);
}

/// Конструктор по имени файла для всех сообщений.
/// Конструировние Логгера на потоки вывода библиотеки STL.
/// \param l_out_file - Имя файла для вывода log информации.
/// \param w_out_file - Имя файла для вывода предупреждений.
/// \param e_out_file - Имя файла для вывода ошибок.
///
/// \note Если в качестве имени файла передается "",
/// это значит, что сообщения такого типа должны ингорироваться, а выводится
/// будут только другие сообщения.
StdLogger::StdLogger(std::string l_out_file,
					std::string w_out_file,
					std::string e_out_file,
					std::ios_base::openmode mode) {
	using namespace std;
	log_out = NULL;l_del=false;;
	if(l_out_file!="") {
		std::ofstream* log_stream = new std::ofstream(l_out_file.c_str(),mode|std::ios_base::out);
		if(*log_stream) {log_out = log_stream;l_del=true;}
		else delete log_stream;
	}
	wrn_out = NULL;w_del=false;;
	if(w_out_file==l_out_file && w_out_file!="") {
		wrn_out = log_out;
		w_del = false;
	} else if(w_out_file!="") {
		std::ofstream* wrn_stream = new std::ofstream(w_out_file.c_str(),mode|std::ios_base::out);
		if(*wrn_stream) {wrn_out = wrn_stream;w_del=true;}
		else delete wrn_stream;
	}
	err_out = NULL;e_del=false;;
	if(e_out_file==l_out_file && e_out_file!="") {
		err_out = log_out;
		e_del = false;
	} else if(e_out_file==w_out_file && e_out_file!="") {
		err_out = wrn_out;
		e_del = false;
	} else if(e_out_file!="") {
		std::ofstream* err_stream = new std::ofstream(e_out_file.c_str(),mode|std::ios_base::out);
		if(*err_stream) {err_out = err_stream;e_del=true;}
		else delete err_stream;
	}
	BaseLogger::onLogStart(true,wrn_out!=log_out,err_out!=wrn_out&&err_out!=log_out);
}

/// Конструктор копирования.
StdLogger::StdLogger(const StdLogger& src) {
	if(src.l_del) log_out=NULL; // не можем прокопировать, иначе возникнет ситуация когда объект родитель умрет и будем иметь указатель на невалидный объект
	else log_out = src.log_out;
	if(src.w_del) wrn_out=NULL; // не можем прокопировать, иначе возникнет ситуация когда объект родитель умрет и будем иметь указатель на невалидный объект
	else wrn_out = src.wrn_out;
	if(src.e_del) err_out=NULL; // не можем прокопировать, иначе возникнет ситуация когда объект родитель умрет и будем иметь указатель на невалидный объект
	else err_out = src.err_out;
	l_del=false;
	w_del=false;
	e_del=false;
	BaseLogger::onLogStart(true,wrn_out!=log_out,err_out!=wrn_out&&err_out!=log_out);
}

StdLogger::~StdLogger() {
	BaseLogger::onLogFinish(true,wrn_out!=log_out,err_out!=wrn_out&&err_out!=log_out);
	if(l_del && log_out) delete log_out;
	if(w_del && wrn_out) delete wrn_out;
	if(e_del && err_out) delete err_out;
}

void StdLogger::ConsoleOut(const std::string& msg) const {
	if (bLogToConsole)
		std::cout << msg << std::endl;
}

void StdLogger::lOut(const std::string& msg) const {
	if (log_out) (*log_out) << msg << std::endl;
	ConsoleOut(msg);
}

void StdLogger::wOut(const std::string& msg) const {
	if (wrn_out) (*wrn_out) << msg << std::endl;
	ConsoleOut(msg);
}

void StdLogger::eOut(const std::string& msg) const {
	if (err_out) (*err_out) << msg << std::endl;
	ConsoleOut(msg);
}