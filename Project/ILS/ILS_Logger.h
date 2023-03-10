#pragma once

#include <cstdio>
#include <memory>
#include <string>
#include <stdarg.h>

//=============================================================================
/// Интерфейс для регистрации хода процессов.
/// @ingroup Kernel
/// ILogger - абстрактный класс (интерфейс) определяющий набор функций
/// для регистрации хода процессов. Большинство методов библиотеки имеют
/// два варианта запуска, с регистрацией хода выполнения процесса и без нее.
/// <i>Если пользователь хочет следить за ходом выполнения процесса, он должен
/// создать класс наследолванный от данного, реализовать в нем все функции
/// данного, и передать ссылку на это класс в функцию запуска процесса.</i>
/// Основные функции для регистрации соообщений:
/// - ILogger::log(const LogId&, const char*, ...)
/// - ILogger::wrn(const LogId&, const char*, ...)
/// - ILogger::err(const LogId&, const char*, ...)
///
/// Функция для регитсрации прогресса выполнения:
/// - ILogger::progress(double pop)
///
/// \warning В случае ошибки процессы не генерируют исключение, а <b>только</b>
/// вызывают функцию ILogger::err(const LogId&, const char*, ...) переданного
/// регистратора (Логгера).
struct ILogger {
public:
	//---------------------------------------------------------------------------
	/// Тип идентификатора сообщений и процессов.
	typedef std::string LogId;
	/// Тип содержания всех сообщений.
	typedef std::string Msg;
	///
	typedef void (ILogger::*TFuncPtr)(const Msg& msg, const LogId& id) const;
	/// Константа, максимальная длина сообщения.
	static const unsigned long max_msg_size = 1024;
	// Регистрация сообщения для анализатора логов.
	virtual void infOut(const Msg& msg, const LogId& id) const = 0;
	// Регистрация информационного сообщения, которую надо переобпределить.
	virtual void logOut(const Msg& msg, const LogId& id) const = 0;
	// Регистрация предупреждения (warning) и не фатальной ошибки, которую надо переобпределить.
	virtual void wrnOut(const Msg& msg, const LogId& id) const = 0;
	// Регистрация фатальной ошибки, которую надо переобпределить.
	virtual void errOut(const Msg& msg, const LogId& id) const = 0;
protected: // Функции, которые надо переопределить при определении реального логгера
	friend struct Logger;
	/// Перевод текста сообщения.
	/// Эту функция переводит (или как-то транслирует) текст сообщения для вывода
	/// пользователю, сохраняя при этом его printf-формат.
	/// \param id - идентификатор текстовый сообщения.
	/// \param msg - текст сообщения по умолчанию (на английском языке).
	/// \return - транслированное сообщение.
	/// \note По умолчанию данная функция возвращает тоже сообщение, что и получила на вход.
	virtual Msg msgTranslate(const LogId& id, const char* msg) const {
		// Для отображение параметра типа "время" используется специальный ключ %t, для логов просто переводим его в %f
		Msg str = msg;
		size_t start_pos = 0;
		while ((start_pos = str.find("%t")) != std::string::npos) {
			str[start_pos + 1] = 'f';
		}
		return str;
	}
	//---------------------------------------------------------------------------
	/// Подготовка потоков к выводу (вывод заголовка лога).
	/// \param l - требуется ли готовить log-поток.
	/// \param w - требуется ли готовить wrn-поток.
	/// \param e - требуется ли готовить err-поток.
	virtual void onLogStart(bool l, bool w, bool e) {}
	/// Подготовка потоков к закрытий (вывод окончания лога).
	/// \param l - требуется ли готовить log-поток.
	/// \param w - требуется ли готовить wrn-поток.
	/// \param e - требуется ли готовить err-поток.
	virtual void onLogFinish(bool l, bool w, bool e) {}
public:
	//---------------------------------------------------------------------------
	/// Регистрация сообщения.
	/// Тип сообщения и способ регистрации определяется указателем на передаваемую функцию:
	/// infOut - регистрация сообщения для анализатора логов.
	/// logOut - регистрация информационного сообщения.
	/// wrnOut - регистрация предупреждения (warning) и не фатальной ошибки.
	/// errOut - регистрация фатальной ошибки.
	/// \param id  - идентификатор сообщения.
	/// \param msg - тело сообщения в формате функции \c printf().
	/// \param ... - набор данных для вывода в сообщении по принципу \c printf().
	void reg(void (ILogger::*regType)(const Msg& msg, const LogId& id) const, const LogId& id, const char* msg, ...) const {
		char* str = new char[max_msg_size];
		try {
			va_list marker;
			va_start(marker, msg);
			vsnprintf(str, max_msg_size, msgTranslate(id, msg).c_str(), marker);
			va_end(marker);
			(this->*regType)(str, id);
		}
		catch (...) {}
		delete[] str;
	}
	//---------------------------------------------------------------------------
	/// Регистрация отладочных сообщений ошибки.
	/// Регистрация фатальной ошибки, после которой результаты процесса не определены.
	/// \param id  - идентификатор сообщения.
	/// \param msg - тело сообщения в формате функции \c printf().
	/// \param ... - набор данных для вывода в сообщении по принципу \c printf().
	inline void dbg(const char* msg, ...) const {
#ifdef _DEBUG
		char* str = new char[max_msg_size + 6];
		strcpy(str, "DEBUG:");
		try {
			va_list marker;
			va_start(marker, msg);
			vsnprintf(str + 6, max_msg_size, msg, marker);
			va_end(marker);
			logOut(str, "dbg");
		}
		catch (...) {}
		delete[] str;
#endif //#ifdef _DEBUG
	}
	/// Параметр логгирования
	virtual double logParam(int param) const { return 0.; };
}; //struct Logger

//=============================================================================
/// Реализация "родительского" регистратора событий.
/// @ingroup Kernel
/// Каждый класс, функции которого могут регистрировать события должен быть наследован
/// виртуальным публичным образом от этого класса. Для регистрации событий должны
/// вызываться функции данного, родительского класса.
/// Logger реализован таким образом, что перенаправялет свои сообщения другому
/// логгеру, который задается пользователем посредством функций этого класса.
/// При этом возможно несоклько ситуаций:
/// - Для данного объекта указан персональный логгер и лог ведется в него
/// - Персональный логгер не указан, но указан общий логгер и лог ведентся в него
/// - Никакой логгер не указан и лог событий вообще не ведется
///
/// по умолчанию никакой логгер не указан.
/// \see Logger
struct Logger : public ILogger {
private: // Указатели на регистраторы на которые транслируются сообщения
	mutable std::shared_ptr<ILogger> personal_logger;   // Персональный логгер данного объекта.
	mutable std::shared_ptr<ILogger> parent_logger;     // Родительский логгер, используется если не указан перссональный.
	/// Логгер данного объекта.
	/// Функция возварщает персональный логер данного объекта если он есть,
	/// или общий логгер если его нет. Если нет ни того не другого функция вернет NULL.
	std::shared_ptr<ILogger> logger() const {
		if (personal_logger != NULL) return personal_logger;
		else if (parent_logger != NULL) return parent_logger;
		else return NULL;
	}
public:
	/// Персональный логгер объекта.
	/// Функция возвращает персональный логгер объекта.
	/// Если он нет установлен, то возвратится NULL.
	virtual std::shared_ptr<ILogger> getPersonalLogger() const { return personal_logger; }
	/// Установить персональный логгер.
	/// Функция устанавливает персональный логгер данного объекта.
	/// По умолчанию она его обнуляет.
	virtual void setPersonalLogger(std::shared_ptr<ILogger> l = NULL) const { personal_logger = l; }
private:
	/// Родительский логгер объекта.
	/// Функция возвращает родительский логгер объекта.
	/// Если он нет установлен, то возвратится NULL.
	std::shared_ptr<ILogger> getParentLogger() const { return personal_logger; }
	/// Установить родительский логгер.
	/// Функция устанавливает родительский логгер данного объекта.
	/// По умолчанию она его обнуляет.
	void setParentLogger(std::shared_ptr<ILogger> l = NULL) const { personal_logger = l; }
public:  // Реализация функций Logger-а.
	virtual Msg msgTranslate(const LogId& id, const char* msg) const {
		if (logger()) return logger()->msgTranslate(id, msg);
		else return msg;
	}
	virtual void infOut(const Msg& msg, const LogId& id) const { if (logger()) logger()->infOut(msg, id); }
	virtual void logOut(const Msg& msg, const LogId& id) const { if (logger()) logger()->logOut(msg, id); }
	virtual void wrnOut(const Msg& msg, const LogId& id) const { if (logger()) logger()->wrnOut(msg, id); }
	virtual void errOut(const Msg& msg, const LogId& id) const { if (logger()) logger()->errOut(msg, id); }
public:
	/// Параметр логгирования
	virtual double logParam(int param) const {
		if (logger()) return logger()->logParam(param);
		else return 0.;
	};
}; //struct Logger
